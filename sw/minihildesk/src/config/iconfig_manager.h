////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// iconfig_manager.h
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

#include <string>

namespace minihildesk::Config {
class IConfigManager {
public:
  virtual ~IConfigManager() = default;

  virtual bool load() = 0;
  virtual bool save() const = 0;
  virtual void defaultConfigStore() = 0;
  virtual void populateDefaults() = 0;

  virtual std::string getIp() const = 0;
  virtual void setIp(const std::string &ip) = 0;

  virtual int getPort() const = 0;
  virtual void setPort(int port) = 0;

  virtual bool getUseSsl() const = 0;
  virtual void setUseSsl(bool useSsl) = 0;

  virtual bool getUseMtls() const = 0;
  virtual void setUseMtls(bool useMtls) = 0;
};
} // namespace minihildesk::Config
