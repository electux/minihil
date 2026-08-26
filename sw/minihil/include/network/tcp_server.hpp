#pragma once

#include "core/iserver.hpp"
#include "core/irpc_handler.hpp"
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

namespace minihil {

class TcpServer : public IServer {
public:
    TcpServer(int port, std::shared_ptr<IRpcHandler> rpcHandler);
    ~TcpServer() override;

    // Starts the listener thread (non-blocking call)
    bool start() override;

    // Gracefully stops the listener and waits for threads to terminate
    void stop() override;

private:
    int m_port;
    std::shared_ptr<IRpcHandler> m_rpcHandler;
    std::atomic<bool> m_running;
    int m_serverSocket;
    std::thread m_listenerThread;
    
    std::vector<int> m_clientSockets;
    std::vector<std::thread> m_clientThreads;
    std::mutex m_clientsMutex;

    void listenLoop();
    void handleClient(int clientSocket);
    void cleanupClosedClients();
};

} // namespace minihil
