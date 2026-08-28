#pragma once

#include "core/base_relay_controller.h"

namespace minihil {

class SilRelayController : public BaseRelayController {
public:
    SilRelayController();
    ~SilRelayController() override;

protected:
    bool initHardware() override;
    bool setRelayPhysical(int relayId, bool state) override;
};

} // namespace minihil
