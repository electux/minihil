////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// iapp_controller.h
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

#include <sigc++/sigc++.h>
#include <string>

namespace minihildesk {
namespace Config {
class IConfigManager;
}
namespace Network {
class ITcpClient;
}

class IAppController {
public:
  virtual ~IAppController() = default;

  virtual Network::ITcpClient &getClient() = 0;

  virtual void start() = 0;
  virtual void stop() = 0;

  virtual void requestConnect(const std::string &ip, int port, bool useSsl,
                              bool useMtls) = 0;
  virtual void requestDisconnect() = 0;
  virtual bool isConnected() const = 0;

  virtual void toggleRelay(int relayId, bool state) = 0;
  virtual void queryAllRelays() = 0;

  virtual sigc::signal<void(const std::string &)> &signal_log() = 0;
  virtual sigc::signal<void(int, bool)> &signal_relay_state() = 0;
  virtual sigc::signal<void(bool)> &signal_connection_state() = 0;

  virtual Config::IConfigManager &getConfig() = 0;
};
} // namespace minihildesk
