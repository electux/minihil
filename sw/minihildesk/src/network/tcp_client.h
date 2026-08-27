////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// tcp_client.h
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

#include <memory>
#include <mutex>
#include <network/itcp_client.h>
#include <string>

namespace minihildesk::Network {

class ISslSession;

class TcpClient : public ITcpClient {
public:
  explicit TcpClient(std::unique_ptr<ISslSession> sslSession);
  ~TcpClient() override;

  TcpClient(const TcpClient &) = delete;
  TcpClient &operator=(const TcpClient &) = delete;

  bool connect(const std::string &ip, int port, bool useSsl = false,
               bool useMtls = false) override;
  void disconnect() override;
  void shutdownSocket() override;
  bool isOpen() const override;

  bool send(const std::string &message) override;
  std::string receiveLine() override; // reads until '\n'

private:
  int m_socketFd{-1};
  mutable std::mutex m_mutex;
  std::string m_readBuffer;

  bool m_useSsl{false};
  std::unique_ptr<ISslSession> m_sslSession;
};

} // namespace minihildesk::Network
