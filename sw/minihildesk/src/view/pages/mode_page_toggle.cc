////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_toggle.cc
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
#include <view/pages/mode_page_toggle.h>

namespace {
constexpr std::string_view cToggleDesc{"Manually control relay state."};
constexpr int cBoxSpacing{8};
constexpr int cSwitchMarginBottom{10};
constexpr int cSwitchMarginTop{5};
} // namespace

namespace minihildesk::View {

ToggleModePage::ToggleModePage(int relayId)
    : Gtk::Box(Gtk::Orientation::VERTICAL, cBoxSpacing), m_relayId(relayId) {
  
  m_descLabel.set_text(cToggleDesc.data());
  m_descLabel.set_wrap(true);
  m_descLabel.set_justify(Gtk::Justification::CENTER);
  m_descLabel.set_vexpand(true);
  m_descLabel.set_valign(Gtk::Align::END);
  append(m_descLabel);

  m_switch.set_halign(Gtk::Align::CENTER);
  m_switch.set_valign(Gtk::Align::CENTER);
  m_switch.set_margin_bottom(cSwitchMarginBottom);
  m_switch.set_margin_top(cSwitchMarginTop);

  m_switch.signal_state_set().connect(
      sigc::mem_fun(*this, &ToggleModePage::onStateSet), false);

  append(m_switch);
}

void ToggleModePage::updateState(bool active) {
  m_updating = true;
  m_switch.set_active(active);
  m_switch.set_state(active);
  m_updating = false;
}

bool ToggleModePage::getState() const {
  return m_switch.get_active();
}

bool ToggleModePage::onStateSet(bool state) {
  if (m_updating) {
    return false;
  }
  m_signalToggled.emit(state);
  return false;
}

} // namespace minihildesk::View
