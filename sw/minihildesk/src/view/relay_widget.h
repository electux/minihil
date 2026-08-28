////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// relay_widget.h
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
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/stack.h>
#include <sigc++/sigc++.h>
#include <view/pages/mode_page_toggle.h>
#include <view/pages/mode_page_timer.h>
#include <view/pages/mode_page_pulse.h>
#include <view/pages/mode_page_blink.h>

namespace minihildesk::View {

class RelayWidget : public Gtk::Frame {
public:
  explicit RelayWidget(int relayId);
  ~RelayWidget() override = default;

  void updateState(bool active);
  bool getState() const;

  sigc::signal<void(int, bool)> &signal_toggled() { return m_signalToggled; }
  sigc::signal<void(int, uint32_t)> &signal_timer_started() {
    return m_signalTimerStarted;
  }
  sigc::signal<void(int, uint32_t)> &signal_pulse_triggered() {
    return m_signalPulseTriggered;
  }
  sigc::signal<void(int, uint32_t, uint32_t, uint32_t)> &
  signal_blink_started() {
    return m_signalBlinkStarted;
  }

private:
  void onModeChanged();

  void onPageToggled(bool state);
  void onPageTimerStarted(uint32_t seconds);
  void onPagePulseTriggered(uint32_t durationMs);
  void onPageBlinkStarted(uint32_t onMs, uint32_t offMs, uint32_t count);

  int m_relayId;
  bool m_active{false};

  Gtk::Box m_box;
  Gtk::Box m_headerBox;
  Gtk::Label m_titleLabel;
  Gtk::Label m_indicatorLabel;

  Gtk::ComboBoxText m_modeCombo;
  Gtk::Stack m_stack;

  ToggleModePage m_pageToggle;
  TimerModePage m_pageTimer;
  PulseModePage m_pagePulse;
  BlinkModePage m_pageBlink;

  sigc::signal<void(int, bool)> m_signalToggled;
  sigc::signal<void(int, uint32_t)> m_signalTimerStarted;
  sigc::signal<void(int, uint32_t)> m_signalPulseTriggered;
  sigc::signal<void(int, uint32_t, uint32_t, uint32_t)> m_signalBlinkStarted;
};

} // namespace minihildesk::View
