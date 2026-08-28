////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_timer.cc
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
#include <view/pages/mode_page_timer.h>
#include <iostream>

namespace {
constexpr std::string_view cTimerDesc{"Keep relay active for a duration."};
constexpr std::string_view cDurationLabel{"Duration (s):"};
constexpr std::string_view cBtnStart{"Start"};
constexpr int cBoxSpacing{8};
constexpr int cInputSpacing{5};
constexpr double cMinSeconds{1.0};
constexpr double cMaxSeconds{3600.0};
constexpr double cStepSeconds{1.0};
constexpr double cPageSeconds{10.0};
} // namespace

namespace minihildesk::View {

TimerModePage::TimerModePage(int relayId)
    : Gtk::Box(Gtk::Orientation::VERTICAL, cBoxSpacing), m_relayId(relayId),
      m_inputBox(Gtk::Orientation::HORIZONTAL, cInputSpacing) {

  m_descLabel.set_text(cTimerDesc.data());
  m_descLabel.set_wrap(true);
  m_descLabel.set_justify(Gtk::Justification::CENTER);
  m_descLabel.set_vexpand(true);
  m_descLabel.set_valign(Gtk::Align::END);
  append(m_descLabel);

  m_durationLabel.set_text(cDurationLabel.data());
  m_durationSpin.set_range(cMinSeconds, cMaxSeconds);
  m_durationSpin.set_increments(cStepSeconds, cPageSeconds);
  m_durationSpin.set_value(10.0); // default 10 seconds

  m_inputBox.set_halign(Gtk::Align::CENTER);
  m_inputBox.append(m_durationLabel);
  m_inputBox.append(m_durationSpin);
  append(m_inputBox);

  m_startButton.set_label(cBtnStart.data());
  m_startButton.set_halign(Gtk::Align::CENTER);
  m_startButton.set_margin_bottom(10);
  m_startButton.signal_clicked().connect(
      sigc::mem_fun(*this, &TimerModePage::onStartClicked));
  append(m_startButton);
}

void TimerModePage::onStartClicked() {
  uint32_t seconds = static_cast<uint32_t>(m_durationSpin.get_value());
  std::cout << "[TimerModePage] Start clicked, seconds: " << seconds << std::endl;
  m_signalStarted.emit(seconds);
}

} // namespace minihildesk::View
