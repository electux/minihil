////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_timer.h
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
#include <sigc++/sigc++.h>

namespace minihildesk::View {

class TimerModePage : public Gtk::Box {
public:
  explicit TimerModePage(int relayId);
  ~TimerModePage() override = default;

  sigc::signal<void(uint32_t)> &signal_started() { return m_signalStarted; }

private:
  void onStartClicked();

  int m_relayId;
  Gtk::Label m_descLabel;
  Gtk::Box m_inputBox;
  Gtk::Label m_durationLabel;
  Gtk::SpinButton m_durationSpin;
  Gtk::Button m_startButton;

  sigc::signal<void(uint32_t)> m_signalStarted;
};

} // namespace minihildesk::View
