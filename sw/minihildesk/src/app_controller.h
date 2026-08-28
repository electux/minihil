////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// app_controller.h
/// Copyright (C) 2025 - 2026 Vladimir Roncevic <elektron.ronca@gmail.com>
///
/// minihildesk is free software: you can redistribute it and/or modify it
/// under the terms of the GNU General Public License as published by the
/// Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// minihildesk is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along
/// with this program. If not, see <http://www.gnu.org/licenses/>.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <atomic>
#include <iapp_controller.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <sigc++/sigc++.h>
#include <thread>

namespace minihildesk {
namespace Config {
class IConfigManager;
}
namespace Network {
class ITcpClient;
}

class AppController : public IAppController {
public:
  AppController(std::unique_ptr<Config::IConfigManager> config,
                std::unique_ptr<Network::ITcpClient> client);
  ~AppController() override;

  Network::ITcpClient &getClient() override;

  void start() override;
  void stop() override;

  void requestConnect(const std::string &ip, int port, bool useSsl,
                      bool useMtls) override;
  void requestDisconnect() override;
  bool isConnected() const override;

  void toggleRelay(int relayId, bool state) override;
  void startTimer(int relayId, uint32_t seconds) override;
  void startPulse(int relayId, uint32_t durationMs) override;
  void startBlink(int relayId, uint32_t onMs, uint32_t offMs,
                  uint32_t count) override;
  void queryAllRelays() override;

  sigc::signal<void(const std::string &)> &signal_log() override {
    return m_signalLog;
  }
  sigc::signal<void(int, bool)> &signal_relay_state() override {
    return m_signalRelayState;
  }
  sigc::signal<void(bool)> &signal_connection_state() override {
    return m_signalConnectionState;
  }

  Config::IConfigManager &getConfig() override;

private:
  void readLoop();
  void processResponse(const std::string &rawResponse);
  void sendJsonRpc(const std::string &method,
                   const nlohmann::json &params = nlohmann::json());

  std::unique_ptr<Config::IConfigManager> m_config;
  std::unique_ptr<Network::ITcpClient> m_client;
  std::thread m_readThread;
  std::atomic<bool> m_running{false};

  int m_requestId;

  sigc::signal<void(const std::string &)> m_signalLog;
  sigc::signal<void(int, bool)> m_signalRelayState;
  sigc::signal<void(bool)> m_signalConnectionState;
};

} // namespace minihildesk
