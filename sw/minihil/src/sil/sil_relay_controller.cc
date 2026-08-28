#include "sil/sil_relay_controller.h"
#include <iostream>

namespace minihil {

SilRelayController::SilRelayController() = default;
SilRelayController::~SilRelayController() = default;

bool SilRelayController::initHardware() {
    std::cout << "[SilRelayController] Software-in-the-Loop simulation initialized." << std::endl;
    return true;
}

bool SilRelayController::setRelayPhysical(int relayId, bool state) {
    std::cout << "[SilRelayController] [SIL-SIM] Relay " << relayId << " set to " << (state ? "ON" : "OFF") << std::endl;
    return true;
}

} // namespace minihil
