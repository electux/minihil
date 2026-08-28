#pragma once

#include "core/idevice_controller.h"
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>

namespace minihil {

class BaseRelayController : public IDeviceController {
public:
    BaseRelayController();
    ~BaseRelayController() override;

    bool init() override;
    bool setRelay(int relayId, bool state) override;
    bool getRelay(int relayId) const override;
    std::map<int, bool> getAllStates() const override;

    // Replicating microhil_base modes
    bool startTimer(int relayId, uint32_t seconds) override;
    bool startPulse(int relayId, uint32_t durationMs) override;
    bool startBlink(int relayId, uint32_t onMs, uint32_t offMs, uint32_t count) override;
    std::string getRelayStatus(int relayId) const override;

protected:
    // Pure virtual methods to be implemented by Gpiod/Sil controllers for actual IO
    virtual bool initHardware() = 0;
    virtual bool setRelayPhysical(int relayId, bool state) = 0;

private:
    enum class RelayMode {
        TOGGLE,
        TIMER,
        PULSE,
        BLINK
    };

    struct RelayState {
        RelayMode mode = RelayMode::TOGGLE;
        bool active = false;
        bool state = false;
        std::chrono::steady_clock::time_point startTime;
        uint64_t durationMs = 0;
        uint32_t blinkOnMs = 0;
        uint32_t blinkOffMs = 0;
        uint32_t blinkCount = 0;
        bool blinkPhase = false;
    };

    void tickLoop();
    void tick();
    void setRelayInternal(int relayId, bool state);

    mutable std::mutex m_mutex;
    std::map<int, RelayState> m_relayStates;
    std::thread m_tickThread;
    std::atomic<bool> m_running{false};
};

} // namespace minihil
