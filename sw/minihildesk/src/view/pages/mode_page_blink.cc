////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_blink.cc
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
#include <view/pages/mode_page_blink.h>
#include <iostream>

namespace {
constexpr std::string_view cBlinkDesc{"Cycle relay state ON and OFF."};
constexpr std::string_view cOnLabel{"ON (ms):"};
constexpr std::string_view cOffLabel{"OFF (ms):"};
constexpr std::string_view cCountLabel{"Count:"};
constexpr std::string_view cBtnStart{"Start Blink"};
constexpr int cBoxSpacing{6};
constexpr int cGridSpacing{4};
constexpr double cMinTime{10.0};
constexpr double cMaxTime{100000.0};
constexpr double cStepTime{50.0};
constexpr double cPageTime{250.0};
constexpr double cMinCount{0.0}; // 0 = infinite
constexpr double cMaxCount{1000.0};
constexpr double cStepCount{1.0};
constexpr double cPageCount{10.0};
} // namespace

namespace minihildesk::View {

BlinkModePage::BlinkModePage(int relayId)
    : Gtk::Box(Gtk::Orientation::VERTICAL, cBoxSpacing), m_relayId(relayId) {

  m_descLabel.set_text(cBlinkDesc.data());
  m_descLabel.set_wrap(true);
  m_descLabel.set_justify(Gtk::Justification::CENTER);
  m_descLabel.set_vexpand(true);
  m_descLabel.set_valign(Gtk::Align::END);
  append(m_descLabel);

  m_onLabel.set_text(cOnLabel.data());
  m_onSpin.set_range(cMinTime, cMaxTime);
  m_onSpin.set_increments(cStepTime, cPageTime);
  m_onSpin.set_value(500.0);
  m_onSpin.set_width_chars(6);

  m_offLabel.set_text(cOffLabel.data());
  m_offSpin.set_range(cMinTime, cMaxTime);
  m_offSpin.set_increments(cStepTime, cPageTime);
  m_offSpin.set_value(500.0);
  m_offSpin.set_width_chars(6);

  m_countLabel.set_text(cCountLabel.data());
  m_countSpin.set_range(cMinCount, cMaxCount);
  m_countSpin.set_increments(cStepCount, cPageCount);
  m_countSpin.set_value(5.0); // default 5 blinks

  m_grid.set_row_spacing(cGridSpacing);
  m_grid.set_column_spacing(cGridSpacing);
  m_grid.set_halign(Gtk::Align::CENTER);

  m_grid.attach(m_onLabel, 0, 0, 1, 1);
  m_grid.attach(m_onSpin, 1, 0, 1, 1);
  m_grid.attach(m_offLabel, 0, 1, 1, 1);
  m_grid.attach(m_offSpin, 1, 1, 1, 1);
  m_grid.attach(m_countLabel, 0, 2, 1, 1);
  m_grid.attach(m_countSpin, 1, 2, 1, 1);
  append(m_grid);

  m_startButton.set_label(cBtnStart.data());
  m_startButton.set_halign(Gtk::Align::CENTER);
  m_startButton.set_margin_bottom(10);
  m_startButton.signal_clicked().connect(
      sigc::mem_fun(*this, &BlinkModePage::onStartClicked));
  append(m_startButton);
}

void BlinkModePage::onStartClicked() {
  uint32_t onMs = static_cast<uint32_t>(m_onSpin.get_value());
  uint32_t offMs = static_cast<uint32_t>(m_offSpin.get_value());
  uint32_t count = static_cast<uint32_t>(m_countSpin.get_value());
  std::cout << "[BlinkModePage] Start Blink clicked, onMs: " << onMs
            << ", offMs: " << offMs << ", count: " << count << std::endl;
  m_signalStarted.emit(onMs, offMs, count);
}

} // namespace minihildesk::View
