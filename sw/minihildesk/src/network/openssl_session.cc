////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// openssl_session.cc
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
#include <iostream>
#include <network/imtls_configurator.h>
#include <network/openssl_session.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace {
constexpr std::string_view cErrSslContext{
    "[OpenSslSession] Failed to create SSL context."};
constexpr std::string_view cErrSslObject{
    "[OpenSslSession] Failed to create SSL object."};
constexpr std::string_view cErrSslHandshake{
    "[OpenSslSession] SSL connection handshake failed."};
constexpr std::string_view cSslSuccessMsg{
    "[OpenSslSession] SSL connection established successfully."};
} // namespace

namespace minihildesk::Network {

OpenSslSession::OpenSslSession(
    std::unique_ptr<IMtlsConfigurator> mtlsConfigurator)
    : m_mtlsConfigurator(std::move(mtlsConfigurator)) {}

OpenSslSession::~OpenSslSession() { disconnect(); }

bool OpenSslSession::initAndConnect(int socketFd, bool useMtls) {
  disconnect();

  const SSL_METHOD *method = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) {
    std::cerr << cErrSslContext << std::endl;
    return false;
  }

  m_sslCtx = ctx;

  if (useMtls) {
    if (m_mtlsConfigurator) {
      if (!m_mtlsConfigurator->configureContext(ctx)) {
        disconnect();
        return false;
      }
    }
  } else {
    // Bypass CA verification because we connect using local IPs with
    // self-signed certs
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
  }

  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    std::cerr << cErrSslObject << std::endl;
    disconnect();
    return false;
  }

  m_ssl = ssl;

  SSL_set_fd(ssl, socketFd);
  if (SSL_connect(ssl) <= 0) {
    std::cerr << cErrSslHandshake << std::endl;
    ERR_print_errors_fp(stderr);
    disconnect();
    return false;
  }

  std::cout << cSslSuccessMsg << std::endl;
  return true;
}

void OpenSslSession::disconnect() {
  if (m_ssl) {
    SSL_free(static_cast<SSL *>(m_ssl));
    m_ssl = nullptr;
  }
  if (m_sslCtx) {
    SSL_CTX_free(static_cast<SSL_CTX *>(m_sslCtx));
    m_sslCtx = nullptr;
  }
}

int OpenSslSession::send(const char *buf, int size) {
  if (!m_ssl) {
    return -1;
  }
  return SSL_write(static_cast<SSL *>(m_ssl), buf, size);
}

int OpenSslSession::receive(char *buf, int size) {
  if (!m_ssl) {
    return -1;
  }
  return SSL_read(static_cast<SSL *>(m_ssl), buf, size);
}

} // namespace minihildesk::Network
