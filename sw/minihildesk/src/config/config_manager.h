////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// config_manager.h
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

#include <config/iconfig_manager.h>
#include <string>

namespace minihildesk::Config {

class ConfigManager : public IConfigManager {
public:
  ConfigManager();
  ~ConfigManager() override = default;

  bool load() override;
  bool save() const override;
  void defaultConfigStore() override;
  void populateDefaults() override;

  std::string getIp() const override { return m_ip; }
  void setIp(const std::string &ip) override { m_ip = ip; }

  int getPort() const override { return m_port; }
  void setPort(int port) override { m_port = port; }

  bool getUseSsl() const override { return m_useSsl; }
  void setUseSsl(bool useSsl) override { m_useSsl = useSsl; }

  bool getUseMtls() const override { return m_useMtls; }
  void setUseMtls(bool useMtls) override { m_useMtls = useMtls; }

private:
  std::string m_ip;
  int m_port;
  bool m_useSsl;
  bool m_useMtls;
  std::string getConfigPath() const;
};

} // namespace minihildesk::Config
