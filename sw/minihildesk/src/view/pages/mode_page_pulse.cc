////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_pulse.cc
/// Copyright (C) 2025 - 2026 Vladimir Roncevic <elektron.ronca@gmail.com>
///
/// minihildesk is free software: you can redistribute it and/or modify it
/// under the terms of the GNU General Public License as published by the
/// Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// minihildesk is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along
/// with this program. If not, see <http://www.gnu.org/licenses/>.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <view/pages/mode_page_pulse.h>
#include <iostream>

namespace {
constexpr std::string_view cPulseDesc{"Generate a momentary active pulse."};
constexpr std::string_view cPulseLabel{"Width (ms):"};
constexpr std::string_view cBtnTrigger{"Trigger"};
constexpr int cBoxSpacing{8};
constexpr int cInputSpacing{5};
constexpr double cMinMs{10.0};
constexpr double cPulseTimeMax{100000.0};
constexpr double cStepMs{50.0};
constexpr double cPageMs{250.0};
} // namespace

namespace minihildesk::View {

PulseModePage::PulseModePage(int relayId)
    : Gtk::Box(Gtk::Orientation::VERTICAL, cBoxSpacing), m_relayId(relayId),
      m_inputBox(Gtk::Orientation::HORIZONTAL, cInputSpacing) {

  m_descLabel.set_text(cPulseDesc.data());
  m_descLabel.set_wrap(true);
  m_descLabel.set_justify(Gtk::Justification::CENTER);
  m_descLabel.set_vexpand(true);
  m_descLabel.set_valign(Gtk::Align::END);
  append(m_descLabel);

  m_pulseLabel.set_text(cPulseLabel.data());
  m_pulseSpin.set_range(cMinMs, cPulseTimeMax);
  m_pulseSpin.set_increments(cStepMs, cPageMs);
  m_pulseSpin.set_width_chars(6);
  m_pulseSpin.set_value(500.0); // default 500 ms

  m_inputBox.set_halign(Gtk::Align::CENTER);
  m_inputBox.append(m_pulseLabel);
  m_inputBox.append(m_pulseSpin);
  append(m_inputBox);

  m_triggerButton.set_label(cBtnTrigger.data());
  m_triggerButton.set_halign(Gtk::Align::CENTER);
  m_triggerButton.set_margin_bottom(10);
  m_triggerButton.signal_clicked().connect(
      sigc::mem_fun(*this, &PulseModePage::onTriggerClicked));
  append(m_triggerButton);
}

void PulseModePage::onTriggerClicked() {
  uint32_t ms = static_cast<uint32_t>(m_pulseSpin.get_value());
  std::cout << "[PulseModePage] Trigger clicked, ms: " << ms << std::endl;
  m_signalTriggered.emit(ms);
}

} // namespace minihildesk::View
