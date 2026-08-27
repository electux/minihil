////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// tcp_client.cc
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
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <network/issl_session.h>
#include <network/tcp_client.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
constexpr std::string_view cErrSocketCreate{
    "[TcpClient] Failed to create socket."};
constexpr std::string_view cErrInvalidAddress{
    "[TcpClient] Invalid address / Address not supported."};
constexpr std::string_view cErrConnectFailed{"[TcpClient] Connection failed."};
} // namespace

namespace minihildesk::Network {

TcpClient::TcpClient(std::unique_ptr<ISslSession> sslSession)
    : m_sslSession(std::move(sslSession)) {}

TcpClient::~TcpClient() { disconnect(); }

bool TcpClient::connect(const std::string &ip, int port, bool useSsl,
                        bool useMtls) {
  disconnect();

  std::lock_guard<std::mutex> lock(m_mutex);
  m_useSsl = useSsl;

  m_socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (m_socketFd < 0) {
    std::cerr << cErrSocketCreate << std::endl;
    return false;
  }

  struct sockaddr_in serv_addr{};
  std::memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(static_cast<uint16_t>(port));

  if (::inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
    std::cerr << cErrInvalidAddress << std::endl;
    ::close(m_socketFd);
    m_socketFd = -1;
    return false;
  }

  if (::connect(m_socketFd, reinterpret_cast<struct sockaddr *>(&serv_addr),
                sizeof(serv_addr)) < 0) {
    std::cerr << cErrConnectFailed << std::endl;
    ::close(m_socketFd);
    m_socketFd = -1;
    return false;
  }

  // Initialize SSL if requested
  if (m_useSsl) {
    if (m_sslSession) {
      if (!m_sslSession->initAndConnect(m_socketFd, useMtls)) {
        ::close(m_socketFd);
        m_socketFd = -1;
        return false;
      }
    }
  }

  return true;
}

void TcpClient::disconnect() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_sslSession) {
    m_sslSession->disconnect();
  }
  if (m_socketFd >= 0) {
    ::shutdown(m_socketFd, SHUT_RDWR);
    ::close(m_socketFd);
    m_socketFd = -1;
  }
  m_readBuffer.clear();
}

void TcpClient::shutdownSocket() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_socketFd >= 0) {
    ::shutdown(m_socketFd, SHUT_RDWR);
  }
}

bool TcpClient::isOpen() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_socketFd >= 0;
}

bool TcpClient::send(const std::string &message) {
  int fd = -1;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    fd = m_socketFd;
  }
  if (fd < 0)
    return false;

  size_t totalSent = 0;
  while (totalSent < message.size()) {
    ssize_t sent = 0;
    if (m_useSsl && m_sslSession) {
      sent = m_sslSession->send(message.c_str() + totalSent,
                                static_cast<int>(message.size() - totalSent));
    } else {
      sent = ::send(fd, message.c_str() + totalSent, message.size() - totalSent,
                    MSG_NOSIGNAL);
    }

    if (sent <= 0) {
      return false; // Do not disconnect here, let the read thread handle it
                    // safely!
    }
    totalSent += static_cast<size_t>(sent);
  }
  return true;
}

std::string TcpClient::receiveLine() {
  char buf[1024];
  while (true) {
    size_t pos = m_readBuffer.find('\n');
    if (pos != std::string::npos) {
      std::string line = m_readBuffer.substr(0, pos);
      m_readBuffer.erase(0, pos + 1);
      return line;
    }

    int fd = -1;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      fd = m_socketFd;
    }
    if (fd < 0) {
      return "";
    }

    ssize_t n = 0;
    if (m_useSsl && m_sslSession) {
      n = m_sslSession->receive(buf, sizeof(buf) - 1);
    } else {
      n = ::recv(fd, buf, sizeof(buf) - 1, 0);
    }

    if (n <= 0) {
      return "";
    }
    buf[n] = '\0';
    m_readBuffer.append(buf, static_cast<size_t>(n));
  }
}

} // namespace minihildesk::Network
