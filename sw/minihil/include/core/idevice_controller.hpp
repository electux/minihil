#pragma once
#include <map>

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
};

} // namespace minihil
