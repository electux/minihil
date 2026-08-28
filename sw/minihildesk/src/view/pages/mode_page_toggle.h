////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// mode_page_toggle.h
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
#include <gtkmm/switch.h>
#include <sigc++/sigc++.h>

namespace minihildesk::View {

class ToggleModePage : public Gtk::Box {
public:
  explicit ToggleModePage(int relayId);
  ~ToggleModePage() override = default;

  void updateState(bool active);
  bool getState() const;

  sigc::signal<void(bool)> &signal_toggled() { return m_signalToggled; }

private:
  bool onStateSet(bool state);

  int m_relayId;
  Gtk::Label m_descLabel;
  Gtk::Switch m_switch;
  bool m_updating{false};

  sigc::signal<void(bool)> m_signalToggled;
};

} // namespace minihildesk::View
