#include "network/tcp_server.h"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace minihil {

TcpServer::TcpServer(int port, std::shared_ptr<IRpcHandler> rpcHandler,
                     bool useSsl, bool useMtls, const std::string& caPath, const std::string& certPath, const std::string& keyPath)
    : m_port(port),
      m_rpcHandler(rpcHandler),
      m_running(false),
      m_serverSocket(-1),
      m_useSsl(useSsl),
      m_useMtls(useMtls),
      m_caPath(caPath),
      m_certPath(certPath),
      m_keyPath(keyPath),
      m_sslCtx(nullptr) {}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start() {
    if (m_running) return true;

    if (m_useSsl) {
        // Initialize OpenSSL context
        const SSL_METHOD* method = TLS_server_method();
        SSL_CTX* ctx = SSL_CTX_new(method);
        if (!ctx) {
            std::cerr << "[TcpServer] Failed to create SSL context." << std::endl;
            ERR_print_errors_fp(stderr);
            return false;
        }

        // Load cert and key
        if (SSL_CTX_use_certificate_file(ctx, m_certPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
            std::cerr << "[TcpServer] Failed to use certificate file: " << m_certPath << std::endl;
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            return false;
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, m_keyPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
            std::cerr << "[TcpServer] Failed to use private key file: " << m_keyPath << std::endl;
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            return false;
        }

        // Load CA if mTLS is enabled
        if (m_useMtls && !m_caPath.empty()) {
            if (SSL_CTX_load_verify_locations(ctx, m_caPath.c_str(), nullptr) <= 0) {
                std::cerr << "[TcpServer] Failed to load CA verify locations from: " << m_caPath << std::endl;
                ERR_print_errors_fp(stderr);
                SSL_CTX_free(ctx);
                return false;
            }
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
            std::cout << "[TcpServer] mTLS enabled: requiring client certificate verified by CA: " << m_caPath << std::endl;
        }

        m_sslCtx = ctx;
        std::cout << "[TcpServer] SSL Context initialized successfully with cert: " << m_certPath << " (mTLS: " << (m_useMtls ? "ENABLED" : "DISABLED") << ")" << std::endl;
    }

    // Create IPv4 TCP socket
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocket < 0) {
        std::cerr << "[TcpServer] Failed to create socket." << std::endl;
        if (m_sslCtx) {
            SSL_CTX_free(static_cast<SSL_CTX*>(m_sslCtx));
            m_sslCtx = nullptr;
        }
        return false;
    }

    // Reuse address to prevent "address already in use" errors on restart
    int opt = 1;
    if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[TcpServer] Warning: Failed to set SO_REUSEADDR." << std::endl;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(m_port);

    if (bind(m_serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[TcpServer] Failed to bind to port " << m_port << "." << std::endl;
        close(m_serverSocket);
        m_serverSocket = -1;
        if (m_sslCtx) {
            SSL_CTX_free(static_cast<SSL_CTX*>(m_sslCtx));
            m_sslCtx = nullptr;
        }
        return false;
    }

    if (listen(m_serverSocket, 5) < 0) {
        std::cerr << "[TcpServer] Failed to listen." << std::endl;
        close(m_serverSocket);
        m_serverSocket = -1;
        if (m_sslCtx) {
            SSL_CTX_free(static_cast<SSL_CTX*>(m_sslCtx));
            m_sslCtx = nullptr;
        }
        return false;
    }

    m_running = true;
    m_listenerThread = std::thread(&TcpServer::listenLoop, this);
    
    std::cout << "[TcpServer] Server listening on port " << m_port << " (SSL: " << (m_useSsl ? "ENABLED" : "DISABLED") << ")..." << std::endl;
    return true;
}

void TcpServer::stop() {
    if (!m_running) return;

    m_running = false;
    
    // Close server socket to unblock accept() in listenLoop
    if (m_serverSocket >= 0) {
        shutdown(m_serverSocket, SHUT_RDWR);
        close(m_serverSocket);
        m_serverSocket = -1;
    }

    if (m_listenerThread.joinable()) {
        m_listenerThread.join();
    }

    // Shut down active client connections to trigger read failures in client threads
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (const auto& session : m_clientSessions) {
            if (session.socketFd >= 0) {
                shutdown(session.socketFd, SHUT_RDWR);
            }
        }
    }

    // Wait for all client threads to clean up and exit
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            if (m_clientSessions.empty()) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (m_sslCtx) {
        SSL_CTX_free(static_cast<SSL_CTX*>(m_sslCtx));
        m_sslCtx = nullptr;
    }
    
    std::cout << "[TcpServer] Server stopped." << std::endl;
}

void TcpServer::listenLoop() {
    while (m_running) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        int clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddress, &clientLen);

        if (clientSocket < 0) {
            if (!m_running) break; // Clean shutdown
            std::cerr << "[TcpServer] Accept failed." << std::endl;
            continue;
        }

        SSL* ssl = nullptr;
        if (m_useSsl && m_sslCtx) {
            ssl = SSL_new(static_cast<SSL_CTX*>(m_sslCtx));
            SSL_set_fd(ssl, clientSocket);
        }

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clientSessions.push_back({clientSocket, ssl});
        }

        // Handle each client in a separate detached thread
        std::thread t(&TcpServer::handleClient, this, clientSocket);
        t.detach();
    }
}

void TcpServer::handleClient(int clientSocket) {
    std::cout << "[TcpServer] Client connected." << std::endl;
    char buffer[2048];
    std::string requestBuffer;

    SSL* ssl = nullptr;
    if (m_useSsl) {
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            for (const auto& session : m_clientSessions) {
                if (session.socketFd == clientSocket) {
                    ssl = static_cast<SSL*>(session.ssl);
                    break;
                }
            }
        }

        if (ssl) {
            if (SSL_accept(ssl) <= 0) {
                std::cerr << "[TcpServer] SSL handshake failed." << std::endl;
                ERR_print_errors_fp(stderr);

                if (ssl) SSL_free(ssl);
                close(clientSocket);

                std::lock_guard<std::mutex> lock(m_clientsMutex);
                auto it = std::find_if(m_clientSessions.begin(), m_clientSessions.end(),
                    [clientSocket](const ClientSession& s) { return s.socketFd == clientSocket; });
                if (it != m_clientSessions.end()) {
                    m_clientSessions.erase(it);
                }
                return;
            }
            std::cout << "[TcpServer] SSL handshake completed successfully." << std::endl;
        }
    }

    while (m_running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = 0;

        if (m_useSsl && ssl) {
            bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        } else {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        }

        if (bytesRead <= 0) {
            break; // Client disconnected or socket closed during shutdown
        }

        requestBuffer.append(buffer, bytesRead);

        // JSON-RPC text frames are terminated by newlines
        size_t newlinePos;
        while ((newlinePos = requestBuffer.find('\n')) != std::string::npos) {
            std::string rawRequest = requestBuffer.substr(0, newlinePos);
            requestBuffer.erase(0, newlinePos + 1);

            if (!rawRequest.empty() && rawRequest != "\r") {
                std::string rawResponse = m_rpcHandler->processRequest(rawRequest);
                if (!rawResponse.empty() && m_running) {
                    if (m_useSsl && ssl) {
                        SSL_write(ssl, rawResponse.c_str(), rawResponse.size());
                    } else {
                        send(clientSocket, rawResponse.c_str(), rawResponse.size(), 0);
                    }
                }
            }
        }
    }

    if (m_useSsl && ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(clientSocket);
    
    // Remove from active client tracking
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        auto it = std::find_if(m_clientSessions.begin(), m_clientSessions.end(),
            [clientSocket](const ClientSession& s) { return s.socketFd == clientSocket; });
        if (it != m_clientSessions.end()) {
            m_clientSessions.erase(it);
        }
    }
    std::cout << "[TcpServer] Client disconnected." << std::endl;
}

void TcpServer::cleanupClosedClients() {}

} // namespace minihil
