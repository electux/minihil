////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_blink.h
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
#pragma once

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <sigc++/sigc++.h>

namespace minihildesk::View {

class BlinkModePage : public Gtk::Box {
public:
  explicit BlinkModePage(int relayId);
  ~BlinkModePage() override = default;

  sigc::signal<void(uint32_t, uint32_t, uint32_t)> &signal_started() {
    return m_signalStarted;
  }

private:
  void onStartClicked();

  int m_relayId;
  Gtk::Label m_descLabel;
  Gtk::Grid m_grid;

  Gtk::Label m_onLabel;
  Gtk::SpinButton m_onSpin;

  Gtk::Label m_offLabel;
  Gtk::SpinButton m_offSpin;

  Gtk::Label m_countLabel;
  Gtk::SpinButton m_countSpin;

  Gtk::Button m_startButton;

  sigc::signal<void(uint32_t, uint32_t, uint32_t)> m_signalStarted;
};

} // namespace minihildesk::View
