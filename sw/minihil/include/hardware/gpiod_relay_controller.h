#pragma once

#include "core/base_relay_controller.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace minihil {

class GpiodRelayController : public BaseRelayController {
public:
    GpiodRelayController();
    ~GpiodRelayController() override;

protected:
    bool initHardware() override;
    bool setRelayPhysical(int relayId, bool state) override;

private:
    std::string m_chipPath;
    // Map from Relay ID (1-8) to BCM pin offset
    std::map<int, unsigned int> m_relayGpioMap;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace minihil
