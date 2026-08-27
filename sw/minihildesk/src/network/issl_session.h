////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// issl_session.h
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

namespace minihildesk::Network {

class ISslSession {
public:
  virtual ~ISslSession() = default;

  virtual bool initAndConnect(int socketFd, bool useMtls) = 0;
  virtual void disconnect() = 0;
  virtual int send(const char *buf, int size) = 0;
  virtual int receive(char *buf, int size) = 0;
};

} // namespace minihildesk::Network
