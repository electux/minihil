////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// openssl_mtls_configurator.cc
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
#include <cstdlib>
#include <iostream>
#include <network/openssl_mtls_configurator.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

namespace {
constexpr std::string_view cEnvHome{"HOME"};
constexpr std::string_view cMinihilConfigDir{"/.minihil/"};
constexpr std::string_view cDefaultDir{"./"};
constexpr std::string_view cCaCertFile{"ca.crt"};
constexpr std::string_view cClientCertFile{"client.crt"};
constexpr std::string_view cClientKeyFile{"client.key"};

// Error and status messages
constexpr std::string_view cErrMtlsCa{"[OpenSslMtlsConfigurator] mTLS error: "
                                      "Failed to load CA certificate from: "};
constexpr std::string_view cErrMtlsClientCert{
    "[OpenSslMtlsConfigurator] mTLS error: Failed to use client certificate "
    "file: "};
constexpr std::string_view cErrMtlsClientKey{
    "[OpenSslMtlsConfigurator] mTLS error: Failed to use client private key "
    "file: "};
constexpr std::string_view cMtlsEnabledMsg{
    "[OpenSslMtlsConfigurator] mTLS enabled. Loaded certificates from: "};
} // namespace

namespace minihildesk::Network {

bool OpenSslMtlsConfigurator::configureContext(void *ctx) {
  if (!ctx) {
    return false;
  }

  SSL_CTX *sslCtx = static_cast<SSL_CTX *>(ctx);

  const char *homedir = getenv(cEnvHome.data());
  std::string configDir = "";
  if (homedir) {
    configDir = std::string(homedir) + cMinihilConfigDir.data();
  } else {
    configDir = cDefaultDir.data();
  }

  std::string caPath = configDir + cCaCertFile.data();
  std::string clientCertPath = configDir + cClientCertFile.data();
  std::string clientKeyPath = configDir + cClientKeyFile.data();

  if (SSL_CTX_load_verify_locations(sslCtx, caPath.c_str(), nullptr) <= 0) {
    std::cerr << cErrMtlsCa << caPath << std::endl;
    ERR_print_errors_fp(stderr);
    return false;
  }

  if (SSL_CTX_use_certificate_file(sslCtx, clientCertPath.c_str(),
                                   SSL_FILETYPE_PEM) <= 0) {
    std::cerr << cErrMtlsClientCert << clientCertPath << std::endl;
    ERR_print_errors_fp(stderr);
    return false;
  }

  if (SSL_CTX_use_PrivateKey_file(sslCtx, clientKeyPath.c_str(),
                                  SSL_FILETYPE_PEM) <= 0) {
    std::cerr << cErrMtlsClientKey << clientKeyPath << std::endl;
    ERR_print_errors_fp(stderr);
    return false;
  }

  // Enforce peer verification (validate server certificate)
  SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, nullptr);
  std::cout << cMtlsEnabledMsg << configDir << std::endl;

  return true;
}

} // namespace minihildesk::Network
