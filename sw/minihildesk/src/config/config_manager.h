#pragma once
#include <string>

namespace minihildesk {

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;

    bool load();
    bool save() const;

    std::string getIp() const { return m_ip; }
    void setIp(const std::string& ip) { m_ip = ip; }

    int getPort() const { return m_port; }
    void setPort(int port) { m_port = port; }

    bool getUseSsl() const { return m_useSsl; }
    void setUseSsl(bool useSsl) { m_useSsl = useSsl; }

    bool getUseMtls() const { return m_useMtls; }
    void setUseMtls(bool useMtls) { m_useMtls = useMtls; }

private:
    std::string m_ip{"127.0.0.1"};
    int m_port{9000};
    bool m_useSsl{false};
    bool m_useMtls{false};
    std::string getConfigPath() const;
};

} // namespace minihildesk
