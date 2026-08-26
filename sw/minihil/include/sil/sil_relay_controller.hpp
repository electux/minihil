#pragma once

#include "core/idevice_controller.hpp"
#include <map>

namespace minihil {

class SilRelayController : public IDeviceController {
public:
    SilRelayController();
    ~SilRelayController() override;

    bool init() override;
    bool setRelay(int relayId, bool state) override;
    bool getRelay(int relayId) const override;
    std::map<int, bool> getAllStates() const override;

private:
    std::map<int, bool> m_states;
};

} // namespace minihil
