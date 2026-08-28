#include <iostream>
#include <memory>
#include <csignal>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <filesystem>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "core/idevice_controller.h"
#include "protocol/jsonrpc_router.h"
#include "network/tcp_server.h"

#ifdef MOCK_GPIO
#include "sil/sil_relay_controller.h"
using ConcreteController = minihil::SilRelayController;
#else
#include "hardware/gpiod_relay_controller.h"
using ConcreteController = minihil::GpiodRelayController;
#endif

namespace fs = std::filesystem;

constexpr int PORT = 9000;

// Synchronization for graceful daemon shutdown
std::mutex g_shutdownMutex;
std::condition_variable g_shutdownCV;
bool g_shutdownRequested = false;

void signalHandler(int signum) {
    std::cout << "\n[Main] Shutdown signal (" << signum << ") received. Notifying threads..." << std::endl;
    {
        std::lock_guard<std::mutex> lock(g_shutdownMutex);
        g_shutdownRequested = true;
    }
    g_shutdownCV.notify_one();
}

bool add_ext(X509* cert, int nid, const char* value) {
    X509_EXTENSION* ex = nullptr;
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, cert, cert, nullptr, nullptr, 0);
    ex = X509V3_EXT_conf_nid(nullptr, &ctx, nid, const_cast<char*>(value));
    if (!ex) return false;
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    return true;
}

bool add_ext_signed(X509* cert, X509* issuer, int nid, const char* value) {
    X509_EXTENSION* ex = nullptr;
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, issuer, cert, nullptr, nullptr, 0);
    ex = X509V3_EXT_conf_nid(nullptr, &ctx, nid, const_cast<char*>(value));
    if (!ex) return false;
    X509_add_ext(cert, ex, -1);
    X509_EXTENSION_free(ex);
    return true;
}

EVP_PKEY* generateKeyPair() {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) return nullptr;
    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0 ||
        EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        if (pkey) EVP_PKEY_free(pkey);
        return nullptr;
    }
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

bool writePrivateKey(const std::string& path, EVP_PKEY* pkey) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = (PEM_write_PrivateKey(f, pkey, nullptr, nullptr, 0, nullptr, nullptr) > 0);
    fclose(f);
    return ok;
}

bool writeCert(const std::string& path, X509* x509) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = (PEM_write_X509(f, x509) > 0);
    fclose(f);
    return ok;
}

bool generatePki(const std::string& caCertPath, const std::string& caKeyPath,
                 const std::string& serverCertPath, const std::string& serverKeyPath,
                 const std::string& clientCertPath, const std::string& clientKeyPath) {
    std::cout << "[Main] Generating Root CA and certificates (Server & Client)..." << std::endl;

    // 1. Generate Root CA
    EVP_PKEY* caKey = generateKeyPair();
    if (!caKey) return false;
    X509* caCert = X509_new();
    if (!caCert) {
        EVP_PKEY_free(caKey);
        return false;
    }
    X509_set_version(caCert, 2); // V3
    ASN1_INTEGER_set(X509_get_serialNumber(caCert), 1);
    X509_gmtime_adj(X509_get_notBefore(caCert), 0);
    X509_gmtime_adj(X509_get_notAfter(caCert), 315360000L); // 10 years
    X509_set_pubkey(caCert, caKey);

    X509_NAME* caName = X509_get_subject_name(caCert);
    X509_NAME_add_entry_by_txt(caName, "C", MBSTRING_ASC, (const unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(caName, "O", MBSTRING_ASC, (const unsigned char*)"Electux", -1, -1, 0);
    X509_NAME_add_entry_by_txt(caName, "CN", MBSTRING_ASC, (const unsigned char*)"MiniHIL Root CA", -1, -1, 0);
    X509_set_issuer_name(caCert, caName);

    add_ext(caCert, NID_basic_constraints, "critical,CA:TRUE");
    add_ext(caCert, NID_key_usage, "critical,keyCertSign,cRLSign");

    if (!X509_sign(caCert, caKey, EVP_sha256())) {
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }

    // 2. Generate Server Certificate
    EVP_PKEY* serverKey = generateKeyPair();
    if (!serverKey) {
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }
    X509* serverCert = X509_new();
    if (!serverCert) {
        EVP_PKEY_free(serverKey);
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }
    X509_set_version(serverCert, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(serverCert), 2);
    X509_gmtime_adj(X509_get_notBefore(serverCert), 0);
    X509_gmtime_adj(X509_get_notAfter(serverCert), 31536000L); // 1 year
    X509_set_pubkey(serverCert, serverKey);

    X509_NAME* serverName = X509_get_subject_name(serverCert);
    X509_NAME_add_entry_by_txt(serverName, "C", MBSTRING_ASC, (const unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(serverName, "O", MBSTRING_ASC, (const unsigned char*)"Electux", -1, -1, 0);
    X509_NAME_add_entry_by_txt(serverName, "CN", MBSTRING_ASC, (const unsigned char*)"localhost", -1, -1, 0);
    X509_set_issuer_name(serverCert, X509_get_subject_name(caCert));

    add_ext_signed(serverCert, caCert, NID_basic_constraints, "CA:FALSE");
    add_ext_signed(serverCert, caCert, NID_key_usage, "critical,digitalSignature,keyEncipherment");
    add_ext_signed(serverCert, caCert, NID_ext_key_usage, "serverAuth");

    if (!X509_sign(serverCert, caKey, EVP_sha256())) {
        X509_free(serverCert);
        EVP_PKEY_free(serverKey);
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }

    // 3. Generate Client Certificate
    EVP_PKEY* clientKey = generateKeyPair();
    if (!clientKey) {
        X509_free(serverCert);
        EVP_PKEY_free(serverKey);
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }
    X509* clientCert = X509_new();
    if (!clientCert) {
        EVP_PKEY_free(clientKey);
        X509_free(serverCert);
        EVP_PKEY_free(serverKey);
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }
    X509_set_version(clientCert, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(clientCert), 3);
    X509_gmtime_adj(X509_get_notBefore(clientCert), 0);
    X509_gmtime_adj(X509_get_notAfter(clientCert), 31536000L); // 1 year
    X509_set_pubkey(clientCert, clientKey);

    X509_NAME* clientName = X509_get_subject_name(clientCert);
    X509_NAME_add_entry_by_txt(clientName, "C", MBSTRING_ASC, (const unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(clientName, "O", MBSTRING_ASC, (const unsigned char*)"Electux", -1, -1, 0);
    X509_NAME_add_entry_by_txt(clientName, "CN", MBSTRING_ASC, (const unsigned char*)"minihildesk", -1, -1, 0);
    X509_set_issuer_name(clientCert, X509_get_subject_name(caCert));

    add_ext_signed(clientCert, caCert, NID_basic_constraints, "CA:FALSE");
    add_ext_signed(clientCert, caCert, NID_key_usage, "critical,digitalSignature");
    add_ext_signed(clientCert, caCert, NID_ext_key_usage, "clientAuth");

    if (!X509_sign(clientCert, caKey, EVP_sha256())) {
        X509_free(clientCert);
        EVP_PKEY_free(clientKey);
        X509_free(serverCert);
        EVP_PKEY_free(serverKey);
        X509_free(caCert);
        EVP_PKEY_free(caKey);
        return false;
    }

    // Write keys & certs to disk
    bool success = true;
    success &= writePrivateKey(caKeyPath, caKey);
    success &= writeCert(caCertPath, caCert);
    success &= writePrivateKey(serverKeyPath, serverKey);
    success &= writeCert(serverCertPath, serverCert);
    success &= writePrivateKey(clientKeyPath, clientKey);
    success &= writeCert(clientCertPath, clientCert);

    X509_free(clientCert);
    EVP_PKEY_free(clientKey);
    X509_free(serverCert);
    EVP_PKEY_free(serverKey);
    X509_free(caCert);
    EVP_PKEY_free(caKey);

    if (success) {
        std::cout << "[Main] Successfully generated full PKI certs:\n"
                  << "  - CA: " << caCertPath << ", " << caKeyPath << "\n"
                  << "  - Server: " << serverCertPath << ", " << serverKeyPath << "\n"
                  << "  - Client: " << clientCertPath << ", " << clientKeyPath << std::endl;
    } else {
        std::cerr << "[Main] Error writing certificates/keys to disk." << std::endl;
    }
    return success;
}

int main(int argc, char* argv[]) {
    bool useSsl = false;
    bool useMtls = false;
    std::string caCertPath = "ca.crt";
    std::string caKeyPath = "ca.key";
    std::string certPath = "server.crt";
    std::string keyPath = "server.key";
    std::string clientCertPath = "client.crt";
    std::string clientKeyPath = "client.key";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ssl") {
            useSsl = true;
        } else if (arg == "--mtls") {
            useSsl = true;
            useMtls = true;
        } else if (arg == "--ca" && i + 1 < argc) {
            caCertPath = argv[++i];
        } else if (arg == "--cert" && i + 1 < argc) {
            certPath = argv[++i];
        } else if (arg == "--key" && i + 1 < argc) {
            keyPath = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: minihild [options]\n"
                      << "Options:\n"
                      << "  --ssl           Enable SSL/TLS secure connection\n"
                      << "  --mtls          Enable SSL/TLS and enforce Mutual TLS (mTLS) client verification\n"
                      << "  --ca <path>     Path to CA certificate file (default: ca.crt)\n"
                      << "  --cert <path>   Path to Server SSL certificate file (default: server.crt)\n"
                      << "  --key <path>    Path to Server SSL private key file (default: server.key)\n"
                      << "  -h, --help      Show this help message\n";
            return 0;
        }
    }

    std::cout << "Starting minihild (MiniHIL POSIX C++ Daemon)..." << std::endl;

    // Register POSIX signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGPIPE, SIG_IGN);

    // Generate certificates if SSL is requested and files do not exist
    if (useSsl) {
        if (!fs::exists(caCertPath) || !fs::exists(caKeyPath) ||
            !fs::exists(certPath) || !fs::exists(keyPath) ||
            !fs::exists(clientCertPath) || !fs::exists(clientKeyPath)) {
            if (!generatePki(caCertPath, caKeyPath, certPath, keyPath, clientCertPath, clientKeyPath)) {
                std::cerr << "[Main] Fatal: Failed to generate PKI certificates." << std::endl;
                return 1;
            }
        }
    }

    // 1. Instantiate HAL / SIL Device Controller (DIP)
    std::shared_ptr<minihil::IDeviceController> controller = std::make_shared<ConcreteController>();
    if (!controller->init()) {
        std::cerr << "[Main] Fatal: Failed to initialize device controller." << std::endl;
        return 1;
    }

    // 2. Instantiate Protocol Router
    auto router = std::make_shared<minihil::JsonRpcRouter>();

    // 3. Register JSON-RPC Methods
    router->registerMethod("set_relay", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        if (!params.is_object() || !params.contains("relay_id") || !params.contains("state")) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' (integer) and 'state' (boolean) are required."}};
        }

        int relayId = params["relay_id"].get<int>();
        bool state = params["state"].get<bool>();

        if (relayId < 1 || relayId > 8) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' must be between 1 and 8."}};
        }

        bool ok = controller->setRelay(relayId, state);
        if (!ok) {
            return {{"code", -32603}, {"error", "Internal error: Failed to update GPIO pin state."}};
        }

        return {{"success", true}, {"relay_id", relayId}, {"state", state}};
    });

    router->registerMethod("get_relays", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        auto states = controller->getAllStates();
        nlohmann::json result = nlohmann::json::object();
        for (const auto& [relayId, state] : states) {
            result[std::to_string(relayId)] = state;
        }
        return result;
    });

    router->registerMethod("start_timer", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        if (!params.is_object() || !params.contains("relay_id") || !params.contains("seconds")) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' (integer) and 'seconds' (integer) are required."}};
        }

        int relayId = params["relay_id"].get<int>();
        uint32_t seconds = params["seconds"].get<uint32_t>();

        if (relayId < 1 || relayId > 8) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' must be between 1 and 8."}};
        }

        bool ok = controller->startTimer(relayId, seconds);
        if (!ok) {
            return {{"code", -32603}, {"error", "Internal error: Failed to start timer."}};
        }

        return {{"success", true}, {"relay_id", relayId}, {"seconds", seconds}};
    });

    router->registerMethod("start_pulse", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        if (!params.is_object() || !params.contains("relay_id") || !params.contains("duration_ms")) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' (integer) and 'duration_ms' (integer) are required."}};
        }

        int relayId = params["relay_id"].get<int>();
        uint32_t durationMs = params["duration_ms"].get<uint32_t>();

        if (relayId < 1 || relayId > 8) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' must be between 1 and 8."}};
        }

        bool ok = controller->startPulse(relayId, durationMs);
        if (!ok) {
            return {{"code", -32603}, {"error", "Internal error: Failed to start pulse."}};
        }

        return {{"success", true}, {"relay_id", relayId}, {"duration_ms", durationMs}};
    });

    router->registerMethod("start_blink", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        if (!params.is_object() || !params.contains("relay_id") || !params.contains("on_ms") || !params.contains("off_ms") || !params.contains("count")) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' (integer), 'on_ms' (integer), 'off_ms' (integer), and 'count' (integer) are required."}};
        }

        int relayId = params["relay_id"].get<int>();
        uint32_t onMs = params["on_ms"].get<uint32_t>();
        uint32_t offMs = params["off_ms"].get<uint32_t>();
        uint32_t count = params["count"].get<uint32_t>();

        if (relayId < 1 || relayId > 8) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' must be between 1 and 8."}};
        }

        bool ok = controller->startBlink(relayId, onMs, offMs, count);
        if (!ok) {
            return {{"code", -32603}, {"error", "Internal error: Failed to start blink."}};
        }

        return {{"success", true}, {"relay_id", relayId}, {"on_ms", onMs}, {"off_ms", offMs}, {"count", count}};
    });

    router->registerMethod("get_relay_status", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        if (!params.is_object() || !params.contains("relay_id")) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' (integer) is required."}};
        }

        int relayId = params["relay_id"].get<int>();

        if (relayId < 1 || relayId > 8) {
            return {{"code", -32602}, {"error", "Invalid params: 'relay_id' must be between 1 and 8."}};
        }

        std::string status = controller->getRelayStatus(relayId);
        return {{"relay_id", relayId}, {"status", status}};
    });

    // 4. Instantiate Server and Inject Router Dependency
    auto server = std::make_shared<minihil::TcpServer>(PORT, router, useSsl, useMtls, caCertPath, certPath, keyPath);

    // 5. Start Server
    if (!server->start()) {
        std::cerr << "[Main] Fatal: Failed to start TCP server." << std::endl;
        return 1;
    }

    // 6. Block Main Thread until signal is caught
    {
        std::unique_lock<std::mutex> lock(g_shutdownMutex);
        g_shutdownCV.wait(lock, [] { return g_shutdownRequested; });
    }

    std::cout << "[Main] Stopping server daemon..." << std::endl;
    server->stop();
    std::cout << "[Main] MiniHIL daemon stopped." << std::endl;

    return 0;
}
