#pragma once

#include "core/idevice_controller.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace minihil {

class GpiodRelayController : public IDeviceController {
public:
    GpiodRelayController();
    ~GpiodRelayController() override;

    bool init() override;
    bool setRelay(int relayId, bool state) override;
    bool getRelay(int relayId) const override;
    std::map<int, bool> getAllStates() const override;

private:
    std::string m_chipPath;
    // Map from Relay ID (1-8) to BCM pin offset
    std::map<int, unsigned int> m_relayGpioMap;
    // Cache for relay states
    mutable std::map<int, bool> m_states;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace minihil
