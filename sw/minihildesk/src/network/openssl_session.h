////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// openssl_session.h
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
#include <network/issl_session.h>

namespace minihildesk::Network {

class IMtlsConfigurator;

class OpenSslSession : public ISslSession {
public:
  explicit OpenSslSession(std::unique_ptr<IMtlsConfigurator> mtlsConfigurator);
  ~OpenSslSession() override;

  bool initAndConnect(int socketFd, bool useMtls) override;
  void disconnect() override;
  int send(const char *buf, int size) override;
  int receive(char *buf, int size) override;

private:
  std::unique_ptr<IMtlsConfigurator> m_mtlsConfigurator;
  void *m_sslCtx{nullptr}; // SSL_CTX*
  void *m_ssl{nullptr};    // SSL*
};

} // namespace minihildesk::Network
