#include "network/tcp_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace minihildesk {

TcpClient::~TcpClient() {
    disconnect();
}

bool TcpClient::connect(const std::string& ip, int port, bool useSsl, bool useMtls) {
    disconnect();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_useSsl = useSsl;

    m_socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketFd < 0) {
        std::cerr << "[TcpClient] Failed to create socket." << std::endl;
        return false;
    }

    struct sockaddr_in serv_addr{};
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "[TcpClient] Invalid address / Address not supported." << std::endl;
        ::close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    if (::connect(m_socketFd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        std::cerr << "[TcpClient] Connection failed." << std::endl;
        ::close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    // Initialize SSL if requested
    if (m_useSsl) {
        const SSL_METHOD* method = TLS_client_method();
        SSL_CTX* ctx = SSL_CTX_new(method);
        if (!ctx) {
            std::cerr << "[TcpClient] Failed to create SSL context." << std::endl;
            ::close(m_socketFd);
            m_socketFd = -1;
            return false;
        }

        m_sslCtx = ctx;

        if (useMtls) {
            // Resolve path to ~/.config/minihildesk/
            const char* homedir = getenv("HOME");
            std::string configDir = "";
            if (homedir) {
                configDir = std::string(homedir) + "/.config/minihildesk/";
            } else {
                configDir = "./";
            }
            std::string caPath = configDir + "ca.crt";
            std::string clientCertPath = configDir + "client.crt";
            std::string clientKeyPath = configDir + "client.key";

            if (SSL_CTX_load_verify_locations(ctx, caPath.c_str(), nullptr) <= 0) {
                std::cerr << "[TcpClient] mTLS error: Failed to load CA certificate from: " << caPath << std::endl;
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                m_sslCtx = nullptr;
                ::close(m_socketFd);
                m_socketFd = -1;
                return false;
            }

            if (SSL_CTX_use_certificate_file(ctx, clientCertPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
                std::cerr << "[TcpClient] mTLS error: Failed to use client certificate file: " << clientCertPath << std::endl;
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                m_sslCtx = nullptr;
                ::close(m_socketFd);
                m_socketFd = -1;
                return false;
            }

            if (SSL_CTX_use_PrivateKey_file(ctx, clientKeyPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
                std::cerr << "[TcpClient] mTLS error: Failed to use client private key file: " << clientKeyPath << std::endl;
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                m_sslCtx = nullptr;
                ::close(m_socketFd);
                m_socketFd = -1;
                return false;
            }

            // Enforce peer verification (validate server certificate)
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
            std::cout << "[TcpClient] mTLS enabled. Loaded certificates from: " << configDir << std::endl;
        } else {
            // Bypass CA verification because we connect using local IPs with self-signed certs
            SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
        }

        SSL* ssl = SSL_new(ctx);
        if (!ssl) {
            std::cerr << "[TcpClient] Failed to create SSL object." << std::endl;
            SSL_CTX_free(ctx);
            m_sslCtx = nullptr;
            ::close(m_socketFd);
            m_socketFd = -1;
            return false;
        }

        SSL_set_fd(ssl, m_socketFd);
        if (SSL_connect(ssl) <= 0) {
            std::cerr << "[TcpClient] SSL connection handshake failed." << std::endl;
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            m_sslCtx = nullptr;
            m_ssl = nullptr;
            ::close(m_socketFd);
            m_socketFd = -1;
            return false;
        }

        m_ssl = ssl;
        std::cout << "[TcpClient] SSL connection established successfully." << std::endl;
    }

    return true;
}

void TcpClient::disconnect() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_ssl) {
        SSL_free(static_cast<SSL*>(m_ssl));
        m_ssl = nullptr;
    }
    if (m_sslCtx) {
        SSL_CTX_free(static_cast<SSL_CTX*>(m_sslCtx));
        m_sslCtx = nullptr;
    }
    if (m_socketFd >= 0) {
        ::shutdown(m_socketFd, SHUT_RDWR);
        ::close(m_socketFd);
        m_socketFd = -1;
    }
    m_readBuffer.clear();
}

void TcpClient::shutdownSocket() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socketFd >= 0) {
        ::shutdown(m_socketFd, SHUT_RDWR);
    }
}

bool TcpClient::isOpen() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socketFd >= 0;
}

bool TcpClient::send(const std::string& message) {
    int fd = -1;
    SSL* ssl = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        fd = m_socketFd;
        ssl = static_cast<SSL*>(m_ssl);
    }
    if (fd < 0) return false;

    size_t totalSent = 0;
    while (totalSent < message.size()) {
        ssize_t sent = 0;
        if (m_useSsl && ssl) {
            sent = SSL_write(ssl, message.c_str() + totalSent, static_cast<int>(message.size() - totalSent));
        } else {
            sent = ::send(fd, message.c_str() + totalSent, message.size() - totalSent, 0);
        }

        if (sent <= 0) {
            return false; // Do not disconnect here, let the read thread handle it safely!
        }
        totalSent += static_cast<size_t>(sent);
    }
    return true;
}

std::string TcpClient::receiveLine() {
    char buf[1024];
    while (true) {
        size_t pos = m_readBuffer.find('\n');
        if (pos != std::string::npos) {
            std::string line = m_readBuffer.substr(0, pos);
            m_readBuffer.erase(0, pos + 1);
            return line;
        }

        int fd = -1;
        SSL* ssl = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            fd = m_socketFd;
            ssl = static_cast<SSL*>(m_ssl);
        }
        if (fd < 0) {
            return "";
        }

        ssize_t n = 0;
        if (m_useSsl && ssl) {
            n = SSL_read(ssl, buf, sizeof(buf) - 1);
        } else {
            n = ::recv(fd, buf, sizeof(buf) - 1, 0);
        }

        if (n <= 0) {
            return "";
        }
        buf[n] = '\0';
        m_readBuffer.append(buf, static_cast<size_t>(n));
    }
}

} // namespace minihildesk
