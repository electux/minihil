////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// app_controller.cc
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
#include <app_controller.h>
#include <chrono>
#include <config/iconfig_manager.h>
#include <iostream>
#include <network/itcp_client.h>

namespace {
constexpr std::string_view cSysConnecting{"[System] Connecting to "};
constexpr std::string_view cColon{":"};
constexpr std::string_view cSslPrefix{" (SSL: "};
constexpr std::string_view cOnStr{"ON"};
constexpr std::string_view cOffStr{"OFF"};
constexpr std::string_view cMtlsPrefix{", mTLS: "};
constexpr std::string_view cSuffix{")..."};
constexpr std::string_view cSysConnected{"[System] Connected successfully."};
constexpr std::string_view cSysConnectFailed{"[System] Connection failed."};
constexpr std::string_view cSysDisconnected{"[System] Disconnected."};
constexpr std::string_view cSysConnectionLost{"[System] Connection lost."};
constexpr std::string_view cSysErrorNotConnected{
    "[System] Error: Not connected."};
constexpr std::string_view cSysErrorSending{"[System] Error sending command."};
constexpr std::string_view cSysErrorParsing{
    "[System] Error parsing response: "};

// JSON keys and values
constexpr std::string_view cJsonRpcKey{"jsonrpc"};
constexpr std::string_view cJsonRpcVer{"2.0"};
constexpr std::string_view cMethodKey{"method"};
constexpr std::string_view cParamsKey{"params"};
constexpr std::string_view cIdKey{"id"};
constexpr std::string_view cRelayIdKey{"relay_id"};
constexpr std::string_view cStateKey{"state"};
constexpr std::string_view cErrorKey{"error"};
constexpr std::string_view cResultKey{"result"};

// RPC Methods
constexpr std::string_view cMethodSetRelay{"set_relay"};
constexpr std::string_view cMethodGetRelays{"get_relays"};

// Prefixes
constexpr std::string_view cTxPrefix{"[TX] "};
constexpr std::string_view cRxPrefix{"[RX] "};
constexpr std::string_view cErrorPrefix{"[Error] "};
constexpr std::string_view cNewLine{"\n"};

constexpr int cReadLoopSleepMs{50};
constexpr int cInitialRequestId{1};
} // namespace

namespace minihildesk {

AppController::AppController(std::unique_ptr<Config::IConfigManager> config,
                             std::unique_ptr<Network::ITcpClient> client)
    : m_config(std::move(config)), m_client(std::move(client)),
      m_requestId(cInitialRequestId) {}

AppController::~AppController() { stop(); }

Network::ITcpClient &AppController::getClient() { return *m_client; }

Config::IConfigManager &AppController::getConfig() { return *m_config; }

void AppController::start() {
  // Controller start (can pre-load settings or trigger autoconnect)
}

void AppController::stop() { requestDisconnect(); }

void AppController::requestConnect(const std::string &ip, int port, bool useSsl,
                                   bool useMtls) {
  requestDisconnect();

  m_signalLog.emit(std::string(cSysConnecting) + ip + cColon.data() +
                   std::to_string(port) + cSslPrefix.data() +
                   (useSsl ? cOnStr.data() : cOffStr.data()) +
                   cMtlsPrefix.data() +
                   (useMtls ? cOnStr.data() : cOffStr.data()) + cSuffix.data());

  if (m_client->connect(ip, port, useSsl, useMtls)) {
    m_config->setIp(ip);
    m_config->setPort(port);
    m_config->setUseSsl(useSsl);
    m_config->setUseMtls(useMtls);
    m_config->save();

    m_running = true;
    m_readThread = std::thread(&AppController::readLoop, this);

    m_signalConnectionState.emit(true);
    m_signalLog.emit(cSysConnected.data());

    // Query initial state of all relays
    queryAllRelays();
  } else {
    m_signalConnectionState.emit(false);
    m_signalLog.emit(cSysConnectFailed.data());
  }
}

void AppController::requestDisconnect() {
  bool wasRunning = m_running.exchange(false);

  if (m_client) {
    m_client->shutdownSocket(); // Unblock SSL_read/recv first
  }

  if (m_readThread.joinable()) {
    m_readThread.join();
  }

  if (m_client) {
    m_client
        ->disconnect(); // Safe to free memory now that read thread is finished
  }

  if (wasRunning) {
    m_signalConnectionState.emit(false);
    m_signalLog.emit(cSysDisconnected.data());
  }
}

bool AppController::isConnected() const {
  return m_client && m_client->isOpen();
}

void AppController::toggleRelay(int relayId, bool state) {
  nlohmann::json params;
  params[cRelayIdKey.data()] = relayId;
  params[cStateKey.data()] = state;
  sendJsonRpc(cMethodSetRelay.data(), params);
}

void AppController::queryAllRelays() { sendJsonRpc(cMethodGetRelays.data()); }

void AppController::sendJsonRpc(const std::string &method,
                                const nlohmann::json &params) {
  if (!m_client || !m_client->isOpen()) {
    m_signalLog.emit(cSysErrorNotConnected.data());
    return;
  }

  nlohmann::json req;
  req[cJsonRpcKey.data()] = cJsonRpcVer;
  req[cMethodKey.data()] = method;
  if (!params.is_null()) {
    req[cParamsKey.data()] = params;
  }
  req[cIdKey.data()] = m_requestId++;

  std::string raw = req.dump() + cNewLine.data();

  if (m_client->send(raw)) {
    m_signalLog.emit(cTxPrefix.data() + req.dump());
  } else {
    m_signalLog.emit(cSysErrorSending.data());
  }
}

void AppController::readLoop() {
  while (m_running) {
    if (m_client && m_client->isOpen()) {
      std::string line = m_client->receiveLine();

      if (line.empty()) {
        if (m_running) {
          m_running = false;
          m_signalConnectionState.emit(false);
          m_signalLog.emit(cSysConnectionLost.data());
        }
        break;
      }

      processResponse(line);

    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(cReadLoopSleepMs));
    }
  }
}

void AppController::processResponse(const std::string &rawResponse) {
  try {
    nlohmann::json res = nlohmann::json::parse(rawResponse);
    m_signalLog.emit(cRxPrefix.data() + rawResponse);

    if (res.contains(cErrorKey.data()) && !res[cErrorKey.data()].is_null()) {
      m_signalLog.emit(cErrorPrefix.data() + res[cErrorKey.data()].dump());
      return;
    }

    if (res.contains(cResultKey.data())) {
      auto result = res[cResultKey.data()];

      if (result.is_object()) {
        if (result.contains(cRelayIdKey.data()) &&
            result.contains(cStateKey.data())) {
          int relayId = result[cRelayIdKey.data()].get<int>();
          bool state = result[cStateKey.data()].get<bool>();
          m_signalRelayState.emit(relayId, state);
        } else {
          for (auto &[key, val] : result.items()) {
            try {
              int relayId = std::stoi(key);
              if (val.is_boolean()) {
                m_signalRelayState.emit(relayId, val.get<bool>());
              }
            } catch (...) {
            }
          }
        }
      }
    }

  } catch (const std::exception &e) {
    m_signalLog.emit(cSysErrorParsing.data() + std::string(e.what()));
  }
}

} // namespace minihildesk
