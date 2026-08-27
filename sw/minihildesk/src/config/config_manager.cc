////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// config_manager.cc
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
#include <config/config_manager.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace {
constexpr std::string_view cEnvHome{"HOME"};
constexpr std::string_view cDefaultConfigFilename{"config"};
constexpr std::string_view cConfigDirName{".minihil"};
constexpr std::string_view cJsonIpKey{"ip"};
constexpr std::string_view cJsonPortKey{"port"};
constexpr std::string_view cJsonSslKey{"ssl"};
constexpr std::string_view cJsonMtlsKey{"mtls"};
constexpr std::string_view cDefaultIp{"127.0.0.1"};
constexpr int cDefaultPort{9000};
constexpr int cJsonIndent{4};
constexpr std::string_view cLoadErrorMsg{"[ConfigManager] Failed to load config: "};
constexpr std::string_view cSaveErrorMsg{"[ConfigManager] Failed to save config: "};
} // namespace

namespace minihildesk::Config {

ConfigManager::ConfigManager()
    : m_ip(cDefaultIp.data()),
      m_port(cDefaultPort),
      m_useSsl(false),
      m_useMtls(false) {
  std::string path = getConfigPath();
  if (!std::filesystem::exists(path)) {
    defaultConfigStore();
  } else {
    load();
  }
}

std::string ConfigManager::getConfigPath() const {
  const char *home = std::getenv(cEnvHome.data());
  if (!home) {
    return cDefaultConfigFilename.data();
  }
  std::filesystem::path p(home);
  p /= cConfigDirName;
  p /= cDefaultConfigFilename;
  return p.string();
}

bool ConfigManager::load() {
  std::string path = getConfigPath();
  if (!std::filesystem::exists(path)) {
    return false;
  }
  try {
    std::ifstream f(path);
    if (!f.is_open())
      return false;
    nlohmann::json j;
    f >> j;
    if (j.contains(cJsonIpKey)) {
      m_ip = j[cJsonIpKey.data()].get<std::string>();
    }
    if (j.contains(cJsonPortKey)) {
      m_port = j[cJsonPortKey.data()].get<int>();
    }
    if (j.contains(cJsonSslKey)) {
      m_useSsl = j[cJsonSslKey.data()].get<bool>();
    }
    if (j.contains(cJsonMtlsKey)) {
      m_useMtls = j[cJsonMtlsKey.data()].get<bool>();
    }
    return true;
  } catch (const std::exception &e) {
    std::cerr << cLoadErrorMsg << e.what()
              << std::endl;
    return false;
  }
}

bool ConfigManager::save() const {
  std::string path = getConfigPath();
  try {
    std::filesystem::path p(path);
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f(path);
    if (!f.is_open())
      return false;
    nlohmann::json j;
    j[cJsonIpKey.data()] = m_ip;
    j[cJsonPortKey.data()] = m_port;
    j[cJsonSslKey.data()] = m_useSsl;
    j[cJsonMtlsKey.data()] = m_useMtls;
    f << j.dump(cJsonIndent);
    return true;
  } catch (const std::exception &e) {
    std::cerr << cSaveErrorMsg << e.what()
              << std::endl;
    return false;
  }
}

void ConfigManager::defaultConfigStore() {
  populateDefaults();
  save();
}

void ConfigManager::populateDefaults() {
  m_ip = cDefaultIp.data();
  m_port = cDefaultPort;
  m_useSsl = false;
  m_useMtls = false;
}

} // namespace minihildesk::Config
