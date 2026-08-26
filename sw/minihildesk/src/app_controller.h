#pragma once
#include "config/config_manager.h"
#include "network/tcp_client.h"
#include <thread>
#include <atomic>
#include <sigc++/sigc++.h>
#include <nlohmann/json.hpp>

namespace minihildesk {

class AppController {
public:
    AppController(ConfigManager& config);
    ~AppController();

    TcpClient& getClient() { return m_client; }

    void start();
    void stop();

    void requestConnect(const std::string& ip, int port, bool useSsl, bool useMtls);
    void requestDisconnect();
    bool isConnected() const;

    void toggleRelay(int relayId, bool state);
    void queryAllRelays();

    sigc::signal<void(const std::string&)>& signal_log() { return m_signalLog; }
    sigc::signal<void(int, bool)>& signal_relay_state() { return m_signalRelayState; }
    sigc::signal<void(bool)>& signal_connection_state() { return m_signalConnectionState; }

    ConfigManager& getConfig() { return m_config; }

private:
    void readLoop();
    void processResponse(const std::string& rawResponse);
    void sendJsonRpc(const std::string& method, const nlohmann::json& params = nlohmann::json());

    ConfigManager& m_config;
    TcpClient m_client;
    std::thread m_readThread;
    std::atomic<bool> m_running{false};

    int m_requestId{1};

    sigc::signal<void(const std::string&)> m_signalLog;
    sigc::signal<void(int, bool)> m_signalRelayState;
    sigc::signal<void(bool)> m_signalConnectionState;
};

} // namespace minihildesk
