#include "sil/sil_relay_controller.hpp"
#include <iostream>

namespace minihil {

SilRelayController::SilRelayController() {
    for (int i = 1; i <= 8; ++i) {
        m_states[i] = false;
    }
}

SilRelayController::~SilRelayController() = default;

bool SilRelayController::init() {
    std::cout << "[SilRelayController] Software-in-the-Loop simulation initialized." << std::endl;
    return true;
}

bool SilRelayController::setRelay(int relayId, bool state) {
    if (relayId < 1 || relayId > 8) return false;
    m_states[relayId] = state;
    std::cout << "[SilRelayController] [SIL-SIM] Relay " << relayId << " set to " << (state ? "ON" : "OFF") << std::endl;
    return true;
}

bool SilRelayController::getRelay(int relayId) const {
    if (relayId < 1 || relayId > 8) return false;
    return m_states.at(relayId);
}

std::map<int, bool> SilRelayController::getAllStates() const {
    return m_states;
}

} // namespace minihil
