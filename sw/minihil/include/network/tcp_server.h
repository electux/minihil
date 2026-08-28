#pragma once

#include "core/iserver.h"
#include "core/irpc_handler.h"
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

namespace minihil {

class TcpServer : public IServer {
public:
    TcpServer(int port, std::shared_ptr<IRpcHandler> rpcHandler,
              bool useSsl = false,
              bool useMtls = false,
              const std::string& caPath = "",
              const std::string& certPath = "",
              const std::string& keyPath = "");
    ~TcpServer() override;

    // Starts the listener thread (non-blocking call)
    bool start() override;

    // Gracefully stops the listener and waits for threads to terminate
    void stop() override;

private:
    struct ClientSession {
        int socketFd;
        void* ssl; // SSL* wrapper
    };

    int m_port;
    std::shared_ptr<IRpcHandler> m_rpcHandler;
    std::atomic<bool> m_running;
    int m_serverSocket;
    std::thread m_listenerThread;
    
    bool m_useSsl;
    bool m_useMtls;
    std::string m_caPath;
    std::string m_certPath;
    std::string m_keyPath;
    void* m_sslCtx; // SSL_CTX* pointer

    std::vector<ClientSession> m_clientSessions;
    std::vector<std::thread> m_clientThreads;
    std::mutex m_clientsMutex;

    void listenLoop();
    void handleClient(int clientSocket);
    void cleanupClosedClients();
};

} // namespace minihil
