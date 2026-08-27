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

namespace {
constexpr std::string_view cRelayCardClass{"relay-card"};
constexpr std::string_view cRelayTitlePrefix{"<b>Relay "};
constexpr std::string_view cRelayTitleSuffix{"</b>"};
constexpr std::string_view cIndicatorChar{"●"};
constexpr std::string_view cIndicatorClass{"led-indicator"};
constexpr std::string_view cLedOffClass{"led-off"};
constexpr std::string_view cLedOnClass{"led-on"};

constexpr int cWidgetMargin{5};
constexpr int cSwitchMarginBottom{10};
constexpr int cSwitchMarginTop{5};
constexpr int cBoxMargin{12};
constexpr int cBoxSpacing{8};
constexpr int cHeaderSpacing{5};
} // namespace

namespace minihildesk::View {

RelayWidget::RelayWidget(int relayId)
    : m_relayId(relayId),
      m_box(Gtk::Orientation::VERTICAL, cBoxSpacing),
      m_headerBox(Gtk::Orientation::HORIZONTAL, cHeaderSpacing) {
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

  m_switch.set_halign(Gtk::Align::CENTER);
  m_switch.set_valign(Gtk::Align::CENTER);
  m_switch.set_margin_bottom(cSwitchMarginBottom);
  m_switch.set_margin_top(cSwitchMarginTop);

  // Block default toggling behavior to handle it asynchronously
  m_switch.signal_state_set().connect(
      sigc::mem_fun(*this, &RelayWidget::onStateSet), false);

  m_box.append(m_headerBox);
  m_box.append(m_switch);
  m_box.set_margin(cBoxMargin);

  set_child(m_box);
}

void RelayWidget::updateState(bool active) {
  m_active = active;

  // Temporarily disable the signal handler logic to prevent feedback loops
  m_updating = true;
  m_switch.set_active(active);
  m_switch.set_state(active);
  m_updating = false;

  if (active) {
    m_indicatorLabel.get_style_context()->remove_class(cLedOffClass.data());
    m_indicatorLabel.get_style_context()->add_class(cLedOnClass.data());
  } else {
    m_indicatorLabel.get_style_context()->remove_class(cLedOnClass.data());
    m_indicatorLabel.get_style_context()->add_class(cLedOffClass.data());
  }
}

bool RelayWidget::getState() const { return m_active; }

bool RelayWidget::onStateSet(bool state) {
  if (m_updating) {
    // Let state propagate visually
    return false;
  }
  // Emit signal to parent, which will request the change via TCP
  m_signalToggled.emit(m_relayId, state);

  // Return false to allow the switch to slide immediately for smooth UI
  // response. If the server fails to update, the next periodic status poll will
  // correct it.
  return false;
}

} // namespace minihildesk::View
