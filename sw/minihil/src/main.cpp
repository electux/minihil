#include <iostream>
#include <memory>
#include <csignal>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>

#include "core/idevice_controller.hpp"
#include "protocol/jsonrpc_router.hpp"
#include "network/tcp_server.hpp"

#ifdef MOCK_GPIO
#include "sil/sil_relay_controller.hpp"
using ConcreteController = minihil::SilRelayController;
#else
#include "hardware/gpiod_relay_controller.hpp"
using ConcreteController = minihil::GpiodRelayController;
#endif

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

int main() {
    std::cout << "Starting minihild (MiniHIL POSIX C++ Daemon)..." << std::endl;

    // Register POSIX signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // 1. Instantiate HAL / SIL Device Controller (DIP)
    std::shared_ptr<minihil::IDeviceController> controller = std::make_shared<ConcreteController>();
    if (!controller->init()) {
        std::cerr << "[Main] Fatal: Failed to initialize device controller." << std::endl;
        return 1;
    }

    // 2. Instantiate Protocol Router
    auto router = std::make_shared<minihil::JsonRpcRouter>();

    // 3. Register JSON-RPC Methods (Open/Closed Principle)
    
    // set_relay: { "relay_id": int [1-8], "state": bool }
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

    // get_relays: returns states of all 8 relays
    router->registerMethod("get_relays", [controller](const nlohmann::json& params, const nlohmann::json& id) -> nlohmann::json {
        auto states = controller->getAllStates();
        nlohmann::json result = nlohmann::json::object();
        for (const auto& [relayId, state] : states) {
            result[std::to_string(relayId)] = state;
        }
        return result;
    });

    // 4. Instantiate Server and Inject Router Dependency (Dependency Inversion)
    auto server = std::make_shared<minihil::TcpServer>(PORT, router);

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
