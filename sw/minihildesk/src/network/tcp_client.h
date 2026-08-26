#pragma once
#include <string>
#include <mutex>

namespace minihildesk {

class TcpClient {
public:
    TcpClient() = default;
    ~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    bool connect(const std::string& ip, int port, bool useSsl = false, bool useMtls = false);
    void disconnect();
    void shutdownSocket();
    bool isOpen() const;

    bool send(const std::string& message);
    std::string receiveLine(); // reads until '\n'

private:
    int m_socketFd{-1};
    mutable std::mutex m_mutex;
    std::string m_readBuffer;

    bool m_useSsl{false};
    void* m_sslCtx{nullptr}; // SSL_CTX*
    void* m_ssl{nullptr};    // SSL*
};

} // namespace minihildesk
