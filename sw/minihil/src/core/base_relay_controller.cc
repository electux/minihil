#include "core/base_relay_controller.h"
#include <iostream>

namespace minihil {

BaseRelayController::BaseRelayController() {
    for (int i = 1; i <= 8; ++i) {
        m_relayStates[i] = RelayState();
    }
}

BaseRelayController::~BaseRelayController() {
    m_running = false;
    if (m_tickThread.joinable()) {
        m_tickThread.join();
    }
}

bool BaseRelayController::init() {
    if (!initHardware()) {
        return false;
    }
    m_running = true;
    m_tickThread = std::thread(&BaseRelayController::tickLoop, this);
    return true;
}

bool BaseRelayController::setRelay(int relayId, bool state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (relayId < 1 || relayId > 8) return false;

    auto& rs = m_relayStates[relayId];
    rs.active = false;
    rs.mode = RelayMode::TOGGLE;
    setRelayInternal(relayId, state);
    return true;
}

bool BaseRelayController::getRelay(int relayId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (relayId < 1 || relayId > 8) return false;
    return m_relayStates.at(relayId).state;
}

std::map<int, bool> BaseRelayController::getAllStates() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::map<int, bool> states;
    for (const auto& [relayId, rs] : m_relayStates) {
        states[relayId] = rs.state;
    }
    return states;
}

bool BaseRelayController::startTimer(int relayId, uint32_t seconds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (relayId < 1 || relayId > 8) return false;

    auto& rs = m_relayStates[relayId];
    rs.mode = RelayMode::TIMER;
    rs.active = true;
    rs.startTime = std::chrono::steady_clock::now();
    rs.durationMs = static_cast<uint64_t>(seconds) * 1000;
    setRelayInternal(relayId, true);
    return true;
}

bool BaseRelayController::startPulse(int relayId, uint32_t durationMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (relayId < 1 || relayId > 8) return false;

    auto& rs = m_relayStates[relayId];
    rs.mode = RelayMode::PULSE;
    rs.active = true;
    rs.startTime = std::chrono::steady_clock::now();
    rs.durationMs = durationMs;
    setRelayInternal(relayId, true);
    return true;
}

bool BaseRelayController::startBlink(int relayId, uint32_t onMs, uint32_t offMs, uint32_t count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (relayId < 1 || relayId > 8) return false;

    auto& rs = m_relayStates[relayId];
    rs.mode = RelayMode::BLINK;
    rs.active = true;
    rs.blinkOnMs = onMs;
    rs.blinkOffMs = offMs;
    rs.blinkCount = count;
    rs.blinkPhase = true;
    rs.startTime = std::chrono::steady_clock::now();
    setRelayInternal(relayId, true);
    return true;
}

std::string BaseRelayController::getRelayStatus(int relayId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (relayId < 1 || relayId > 8) return "invalid channel";

    const auto& rs = m_relayStates.at(relayId);
    const char* phys_state = rs.state ? "ON" : "OFF";
    char buf[256];

    if (!rs.active) {
        snprintf(buf, sizeof(buf), "Channel %d: %s (Toggle)", relayId, phys_state);
    } else {
        auto now = std::chrono::steady_clock::now();
        switch (rs.mode) {
            case RelayMode::TIMER: {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - rs.startTime).count();
                uint64_t remaining = (rs.durationMs > static_cast<uint64_t>(elapsed)) ? 
                                     (rs.durationMs - static_cast<uint64_t>(elapsed)) : 0;
                snprintf(buf, sizeof(buf), "Channel %d: %s (Timer, rem: %llus)",
                         relayId, phys_state, (unsigned long long)(remaining / 1000));
                break;
            }
            case RelayMode::PULSE: {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - rs.startTime).count();
                uint64_t remaining = (rs.durationMs > static_cast<uint64_t>(elapsed)) ? 
                                     (rs.durationMs - static_cast<uint64_t>(elapsed)) : 0;
                snprintf(buf, sizeof(buf), "Channel %d: %s (Pulse, rem: %llums)",
                         relayId, phys_state, (unsigned long long)remaining);
                break;
            }
            case RelayMode::BLINK: {
                snprintf(buf, sizeof(buf), "Channel %d: %s (Blink, count: %u, phase: %s)",
                         relayId, phys_state, rs.blinkCount,
                         rs.blinkPhase ? "ON" : "OFF");
                break;
            }
            default:
                snprintf(buf, sizeof(buf), "Channel %d: %s", relayId, phys_state);
                break;
        }
    }
    return std::string(buf);
}

void BaseRelayController::tickLoop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        tick();
    }
}

void BaseRelayController::tick() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    for (auto& [relayId, rs] : m_relayStates) {
        if (!rs.active) continue;

        if (rs.mode == RelayMode::TIMER || rs.mode == RelayMode::PULSE) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - rs.startTime).count();
            if (static_cast<uint64_t>(elapsed) >= rs.durationMs) {
                setRelayInternal(relayId, false);
                rs.active = false;
                rs.mode = RelayMode::TOGGLE;
                std::cout << "<mh#sys#channel " << relayId << " off#end>" << std::endl;
            }
        } else if (rs.mode == RelayMode::BLINK) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - rs.startTime).count();
            if (rs.blinkPhase) {
                if (static_cast<uint32_t>(elapsed) >= rs.blinkOnMs) {
                    setRelayInternal(relayId, false);
                    rs.blinkPhase = false;
                    rs.startTime = now;
                }
            } else {
                if (static_cast<uint32_t>(elapsed) >= rs.blinkOffMs) {
                    if (rs.blinkCount > 0) {
                        rs.blinkCount--;
                        if (rs.blinkCount == 0) {
                            rs.active = false;
                            rs.mode = RelayMode::TOGGLE;
                            std::cout << "<mh#sys#channel " << relayId << " off#end>" << std::endl;
                            continue;
                        }
                    }
                    setRelayInternal(relayId, true);
                    rs.blinkPhase = true;
                    rs.startTime = now;
                }
            }
        }
    }
}

void BaseRelayController::setRelayInternal(int relayId, bool state) {
    auto& rs = m_relayStates[relayId];
    if (rs.state != state) {
        rs.state = state;
        setRelayPhysical(relayId, state);
    }
}

} // namespace minihil
