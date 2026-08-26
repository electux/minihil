#include "network/tcp_server.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>

namespace minihil {

TcpServer::TcpServer(int port, std::shared_ptr<IRpcHandler> rpcHandler)
    : m_port(port),
      m_rpcHandler(rpcHandler),
      m_running(false),
      m_serverSocket(-1) {}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start() {
    if (m_running) return true;

    // Create IPv4 TCP socket
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocket < 0) {
        std::cerr << "[TcpServer] Failed to create socket." << std::endl;
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
        return false;
    }

    if (listen(m_serverSocket, 5) < 0) {
        std::cerr << "[TcpServer] Failed to listen." << std::endl;
        close(m_serverSocket);
        m_serverSocket = -1;
        return false;
    }

    m_running = true;
    m_listenerThread = std::thread(&TcpServer::listenLoop, this);
    
    std::cout << "[TcpServer] Server listening on port " << m_port << "..." << std::endl;
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

    // Close all active client connections to unblock recv() in handleClient
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (int fd : m_clientSockets) {
        if (fd >= 0) {
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
    }
    m_clientSockets.clear();
    
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

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clientSockets.push_back(clientSocket);
        }

        // Handle each client in a separate detached thread (automatic memory reclamation on exit)
        std::thread t(&TcpServer::handleClient, this, clientSocket);
        t.detach();
    }
}

void TcpServer::handleClient(int clientSocket) {
    std::cout << "[TcpServer] Client connected." << std::endl;
    char buffer[2048];
    std::string requestBuffer;

    while (m_running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
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
                    send(clientSocket, rawResponse.c_str(), rawResponse.size(), 0);
                }
            }
        }
    }

    close(clientSocket);
    
    // Remove from active client tracking
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    auto it = std::find(m_clientSockets.begin(), m_clientSockets.end(), clientSocket);
    if (it != m_clientSockets.end()) {
        m_clientSockets.erase(it);
    }
    std::cout << "[TcpServer] Client disconnected." << std::endl;
}

} // namespace minihil
