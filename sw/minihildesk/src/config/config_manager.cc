#include "config/config_manager.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <iostream>

namespace minihildesk {

ConfigManager::ConfigManager() {
    // Try loading on construction
    load();
}

std::string ConfigManager::getConfigPath() const {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "config.json";
    }
    std::filesystem::path p(home);
    p /= ".config";
    p /= "minihildesk";
    p /= "config.json";
    return p.string();
}

bool ConfigManager::load() {
    std::string path = getConfigPath();
    if (!std::filesystem::exists(path)) {
        return false;
    }
    try {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        nlohmann::json j;
        f >> j;
        if (j.contains("ip")) {
            m_ip = j["ip"].get<std::string>();
        }
        if (j.contains("port")) {
            m_port = j["port"].get<int>();
        }
        if (j.contains("ssl")) {
            m_useSsl = j["ssl"].get<bool>();
        }
        if (j.contains("mtls")) {
            m_useMtls = j["mtls"].get<bool>();
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager] Failed to load config: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::save() const {
    std::string path = getConfigPath();
    try {
        std::filesystem::path p(path);
        std::filesystem::create_directories(p.parent_path());
        std::ofstream f(path);
        if (!f.is_open()) return false;
        nlohmann::json j;
        j["ip"] = m_ip;
        j["port"] = m_port;
        j["ssl"] = m_useSsl;
        j["mtls"] = m_useMtls;
        f << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager] Failed to save config: " << e.what() << std::endl;
        return false;
    }
}

} // namespace minihildesk
