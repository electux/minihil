#include "app_controller.h"
#include <iostream>
#include <chrono>

namespace minihildesk {

AppController::AppController(ConfigManager& config) : m_config(config) {}

AppController::~AppController() {
    stop();
}

void AppController::start() {
    // Controller start (can pre-load settings or trigger autoconnect)
}

void AppController::stop() {
    requestDisconnect();
}

void AppController::requestConnect(const std::string& ip, int port, bool useSsl, bool useMtls) {
    requestDisconnect();

    m_signalLog.emit("[System] Connecting to " + ip + ":" + std::to_string(port) + " (SSL: " + (useSsl ? "ON" : "OFF") + ", mTLS: " + (useMtls ? "ON" : "OFF") + ")...");
    if (m_client.connect(ip, port, useSsl, useMtls)) {
        m_config.setIp(ip);
        m_config.setPort(port);
        m_config.setUseSsl(useSsl);
        m_config.setUseMtls(useMtls);
        m_config.save();

        m_running = true;
        m_readThread = std::thread(&AppController::readLoop, this);

        m_signalConnectionState.emit(true);
        m_signalLog.emit("[System] Connected successfully.");

        // Query initial state of all relays
        queryAllRelays();
    } else {
        m_signalConnectionState.emit(false);
        m_signalLog.emit("[System] Connection failed.");
    }
}

void AppController::requestDisconnect() {
    bool wasRunning = m_running.exchange(false);
    m_client.shutdownSocket(); // Unblock SSL_read/recv first
    if (m_readThread.joinable()) {
        m_readThread.join();
    }
    m_client.disconnect(); // Safe to free memory now that read thread is finished
    if (wasRunning) {
        m_signalConnectionState.emit(false);
        m_signalLog.emit("[System] Disconnected.");
    }
}

bool AppController::isConnected() const {
    return m_client.isOpen();
}

void AppController::toggleRelay(int relayId, bool state) {
    nlohmann::json params;
    params["relay_id"] = relayId;
    params["state"] = state;
    sendJsonRpc("set_relay", params);
}

void AppController::queryAllRelays() {
    sendJsonRpc("get_relays");
}

void AppController::sendJsonRpc(const std::string& method, const nlohmann::json& params) {
    if (!m_client.isOpen()) {
        m_signalLog.emit("[System] Error: Not connected.");
        return;
    }

    nlohmann::json req;
    req["jsonrpc"] = "2.0";
    req["method"] = method;
    if (!params.is_null()) {
        req["params"] = params;
    }
    req["id"] = m_requestId++;

    std::string raw = req.dump() + "\n";
    if (m_client.send(raw)) {
        m_signalLog.emit("[TX] " + req.dump());
    } else {
        m_signalLog.emit("[System] Error sending command.");
    }
}

void AppController::readLoop() {
    while (m_running) {
        if (m_client.isOpen()) {
            std::string line = m_client.receiveLine();
            if (line.empty()) {
                if (m_running) {
                    m_running = false;
                    m_signalConnectionState.emit(false);
                    m_signalLog.emit("[System] Connection lost.");
                }
                break;
            }
            processResponse(line);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void AppController::processResponse(const std::string& rawResponse) {
    try {
        nlohmann::json res = nlohmann::json::parse(rawResponse);
        m_signalLog.emit("[RX] " + rawResponse);

        if (res.contains("error") && !res["error"].is_null()) {
            m_signalLog.emit("[Error] " + res["error"].dump());
            return;
        }

        if (res.contains("result")) {
            auto result = res["result"];
            if (result.is_object()) {
                if (result.contains("relay_id") && result.contains("state")) {
                    int relayId = result["relay_id"].get<int>();
                    bool state = result["state"].get<bool>();
                    m_signalRelayState.emit(relayId, state);
                } else {
                    for (auto& [key, val] : result.items()) {
                        try {
                            int relayId = std::stoi(key);
                            if (val.is_boolean()) {
                                m_signalRelayState.emit(relayId, val.get<bool>());
                            }
                        } catch (...) {}
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        m_signalLog.emit("[System] Error parsing response: " + std::string(e.what()));
    }
}

} // namespace minihildesk
