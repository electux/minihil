////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// relay_widget.cc
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
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <view/relay_widget.h>
#include <iostream>

namespace {
constexpr std::string_view cRelayCardClass{"relay-card"};
constexpr std::string_view cRelayTitlePrefix{"<b>Relay "};
constexpr std::string_view cRelayTitleSuffix{"</b>"};
constexpr std::string_view cIndicatorChar{"●"};
constexpr std::string_view cIndicatorClass{"led-indicator"};
constexpr std::string_view cLedOffClass{"led-off"};
constexpr std::string_view cLedOnClass{"led-on"};

constexpr int cWidgetMargin{5};
constexpr int cBoxMargin{12};
constexpr int cBoxSpacing{8};
constexpr int cHeaderSpacing{5};
} // namespace

namespace minihildesk::View {

RelayWidget::RelayWidget(int relayId)
    : m_relayId(relayId),
      m_box(Gtk::Orientation::VERTICAL, cBoxSpacing),
      m_headerBox(Gtk::Orientation::HORIZONTAL, cHeaderSpacing),
      m_pageToggle(relayId),
      m_pageTimer(relayId),
      m_pagePulse(relayId),
      m_pageBlink(relayId) {
  set_margin(cWidgetMargin);

  // Set card styling class
  get_style_context()->add_class(cRelayCardClass.data());

  m_titleLabel.set_markup(std::string(cRelayTitlePrefix) + std::to_string(relayId) + std::string(cRelayTitleSuffix));
  m_titleLabel.set_halign(Gtk::Align::START);

  // Indicator label displays filled circle unicode
  m_indicatorLabel.set_text(cIndicatorChar.data());
  m_indicatorLabel.set_halign(Gtk::Align::END);
  m_indicatorLabel.set_hexpand(true);
  m_indicatorLabel.get_style_context()->add_class(cIndicatorClass.data());
  m_indicatorLabel.get_style_context()->add_class(cLedOffClass.data());

  m_headerBox.append(m_titleLabel);
  m_headerBox.append(m_indicatorLabel);

  // Populate mode selection dropdown
  m_modeCombo.append("Toggle");
  m_modeCombo.append("Timer");
  m_modeCombo.append("Pulse");
  m_modeCombo.append("Blink");
  m_modeCombo.set_active(0);
  m_modeCombo.signal_changed().connect(
      sigc::mem_fun(*this, &RelayWidget::onModeChanged));

  // Stack settings
  m_stack.set_transition_type(Gtk::StackTransitionType::CROSSFADE);
  m_stack.set_transition_duration(150);
  m_stack.set_hhomogeneous(true);
  m_stack.set_vhomogeneous(true);
  m_stack.set_vexpand(true);

  m_stack.add(m_pageToggle, "toggle");
  m_stack.add(m_pageTimer, "timer");
  m_stack.add(m_pagePulse, "pulse");
  m_stack.add(m_pageBlink, "blink");
  m_stack.set_visible_child("toggle");

  // Connect child page signals
  m_pageToggle.signal_toggled().connect(
      sigc::mem_fun(*this, &RelayWidget::onPageToggled));
  m_pageTimer.signal_started().connect(
      sigc::mem_fun(*this, &RelayWidget::onPageTimerStarted));
  m_pagePulse.signal_triggered().connect(
      sigc::mem_fun(*this, &RelayWidget::onPagePulseTriggered));
  m_pageBlink.signal_started().connect(
      sigc::mem_fun(*this, &RelayWidget::onPageBlinkStarted));

  m_box.append(m_headerBox);
  m_box.append(m_modeCombo);
  m_box.append(m_stack);
  m_box.set_margin(cBoxMargin);

  set_child(m_box);
}

void RelayWidget::updateState(bool active) {
  m_active = active;

  if (active) {
    m_indicatorLabel.get_style_context()->remove_class(cLedOffClass.data());
    m_indicatorLabel.get_style_context()->add_class(cLedOnClass.data());
    get_style_context()->add_class("active-relay");
  } else {
    m_indicatorLabel.get_style_context()->remove_class(cLedOnClass.data());
    m_indicatorLabel.get_style_context()->add_class(cLedOffClass.data());
    get_style_context()->remove_class("active-relay");
  }

  m_pageToggle.updateState(active);
}

bool RelayWidget::getState() const { return m_active; }

void RelayWidget::onModeChanged() {
  int activeIndex = m_modeCombo.get_active_row_number();
  switch (activeIndex) {
    case 0: m_stack.set_visible_child("toggle"); break;
    case 1: m_stack.set_visible_child("timer"); break;
    case 2: m_stack.set_visible_child("pulse"); break;
    case 3: m_stack.set_visible_child("blink"); break;
    default: m_stack.set_visible_child("toggle"); break;
  }

  // Automatically turn off the relay when mode changes for safety
  if (m_active) {
    m_signalToggled.emit(m_relayId, false);
  }
}

void RelayWidget::onPageToggled(bool state) {
  std::cout << "[RelayWidget] onPageToggled for relay " << m_relayId << ", state: " << state << std::endl;
  m_signalToggled.emit(m_relayId, state);
}

void RelayWidget::onPageTimerStarted(uint32_t seconds) {
  std::cout << "[RelayWidget] onPageTimerStarted for relay " << m_relayId << ", seconds: " << seconds << std::endl;
  m_signalTimerStarted.emit(m_relayId, seconds);
}

void RelayWidget::onPagePulseTriggered(uint32_t durationMs) {
  std::cout << "[RelayWidget] onPagePulseTriggered for relay " << m_relayId << ", durationMs: " << durationMs << std::endl;
  m_signalPulseTriggered.emit(m_relayId, durationMs);
}

void RelayWidget::onPageBlinkStarted(uint32_t onMs, uint32_t offMs, uint32_t count) {
  std::cout << "[RelayWidget] onPageBlinkStarted for relay " << m_relayId << ", onMs: " << onMs << ", offMs: " << offMs << ", count: " << count << std::endl;
  m_signalBlinkStarted.emit(m_relayId, onMs, offMs, count);
}

} // namespace minihildesk::View
