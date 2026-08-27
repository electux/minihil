#ifdef GPIO_HARDWARE_SUPPORT

#include "hardware/gpiod_relay_controller.hpp"
#include <gpiod.hpp>
#include <iostream>

namespace minihil {

struct GpiodRelayController::Impl {
    std::unique_ptr<::gpiod::chip> chip;
    std::unique_ptr<::gpiod::line_request> request;
};

GpiodRelayController::GpiodRelayController()
    : m_chipPath("/dev/gpiochip4"), // Try default for RPi 4/5
      m_impl(std::make_unique<Impl>())
{
    // Waveshare Relay Board (B) mapping: channels 1-8 to BCM pin offsets
    m_relayGpioMap = {
        {1, 5}, {2, 6}, {3, 13}, {4, 16},
        {5, 19}, {6, 20}, {7, 21}, {8, 26}
    };
    
    for (int i = 1; i <= 8; ++i) {
        m_states[i] = false;
    }
}

GpiodRelayController::~GpiodRelayController() = default;

bool GpiodRelayController::init() {
    try {
        std::string selectedChip = m_chipPath;
        bool chipOpened = false;
        
        try {
            m_impl->chip = std::make_unique<::gpiod::chip>(selectedChip);
            chipOpened = true;
        } catch (...) {
            // Fallback for Raspberry Pi 3 where GPIOs are on gpiochip0
            try {
                selectedChip = "/dev/gpiochip0";
                m_impl->chip = std::make_unique<::gpiod::chip>(selectedChip);
                chipOpened = true;
                m_chipPath = selectedChip;
            } catch (...) {
                // Fallback failed
            }
        }

        if (!chipOpened) {
            std::cerr << "[GpiodRelayController] Failed to open GPIO chip (neither /dev/gpiochip4 nor /dev/gpiochip0 was accessible)." << std::endl;
            return false;
        }

        std::cout << "[GpiodRelayController] Opened GPIO chip: " << m_chipPath << std::endl;

        ::gpiod::line_config line_cfg;
        for (const auto& [id, offset] : m_relayGpioMap) {
            line_cfg.add_line_settings(offset,
                ::gpiod::line_settings()
                    .set_direction(::gpiod::line::direction::OUTPUT)
                    .set_output_value(::gpiod::line::value::ACTIVE)
            );
        }

        ::gpiod::request_config req_cfg;
        req_cfg.set_consumer("minihild");

        m_impl->request = std::make_unique<::gpiod::line_request>(
            m_impl->chip->prepare_request()
                .set_request_config(req_cfg)
                .set_line_config(line_cfg)
                .do_request()
        );
        std::cout << "[GpiodRelayController] GPIO lines requested and configured as OUTPUT." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[GpiodRelayController] Exception in init: " << e.what() << std::endl;
        return false;
    }
}

bool GpiodRelayController::setRelay(int relayId, bool state) {
    if (relayId < 1 || relayId > 8) return false;
    
    m_states[relayId] = state;
    try {
        if (!m_impl->request) return false;
        unsigned int offset = m_relayGpioMap.at(relayId);
        m_impl->request->set_value(offset, state ? ::gpiod::line::value::INACTIVE : ::gpiod::line::value::ACTIVE);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[GpiodRelayController] Error setting relay " << relayId << ": " << e.what() << std::endl;
        return false;
    }
}

bool GpiodRelayController::getRelay(int relayId) const {
    if (relayId < 1 || relayId > 8) return false;
    return m_states[relayId];
}

std::map<int, bool> GpiodRelayController::getAllStates() const {
    return m_states;
}

} // namespace minihil

#else
// Empty translation unit fallback for host builds without libgpiod v2
static inline void dummy_gpiod_fallback() {}
#endif
