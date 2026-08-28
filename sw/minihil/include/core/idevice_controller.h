#pragma once
#include <map>
#include <string>
#include <stdint.h>

namespace minihil {

class IDeviceController {
public:
    virtual ~IDeviceController() = default;

    // Initializes the hardware/simulation controller
    virtual bool init() = 0;

    // Sets the state of a specific relay (1-8)
    virtual bool setRelay(int relayId, bool state) = 0;

    // Gets the state of a specific relay (1-8)
    virtual bool getRelay(int relayId) const = 0;

    // Returns a map of all relay channels and their states
    virtual std::map<int, bool> getAllStates() const = 0;

    // Replicating microhil_base modes
    virtual bool startTimer(int relayId, uint32_t seconds) = 0;
    virtual bool startPulse(int relayId, uint32_t durationMs) = 0;
    virtual bool startBlink(int relayId, uint32_t onMs, uint32_t offMs, uint32_t count) = 0;
    virtual std::string getRelayStatus(int relayId) const = 0;
};

} // namespace minihil
