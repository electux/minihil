#include "view/relay_widget.h"
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>

namespace minihildesk {

RelayWidget::RelayWidget(int relayId) : m_relayId(relayId) {
    set_margin(5);
    
    // Set card styling class
    get_style_context()->add_class("relay-card");

    m_titleLabel.set_markup("<b>Relay " + std::to_string(relayId) + "</b>");
    m_titleLabel.set_halign(Gtk::Align::START);

    // Indicator label displays filled circle unicode
    m_indicatorLabel.set_text("●");
    m_indicatorLabel.set_halign(Gtk::Align::END);
    m_indicatorLabel.set_hexpand(true);
    m_indicatorLabel.get_style_context()->add_class("led-indicator");
    m_indicatorLabel.get_style_context()->add_class("led-off");

    m_headerBox.append(m_titleLabel);
    m_headerBox.append(m_indicatorLabel);

    m_switch.set_halign(Gtk::Align::CENTER);
    m_switch.set_valign(Gtk::Align::CENTER);
    m_switch.set_margin_bottom(10);
    m_switch.set_margin_top(5);

    // Block default toggling behavior to handle it asynchronously
    m_switch.signal_state_set().connect(
        sigc::mem_fun(*this, &RelayWidget::onStateSet), false
    );

    m_box.append(m_headerBox);
    m_box.append(m_switch);
    m_box.set_margin(12);

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
        m_indicatorLabel.get_style_context()->remove_class("led-off");
        m_indicatorLabel.get_style_context()->add_class("led-on");
    } else {
        m_indicatorLabel.get_style_context()->remove_class("led-on");
        m_indicatorLabel.get_style_context()->add_class("led-off");
    }
}

bool RelayWidget::getState() const {
    return m_active;
}

bool RelayWidget::onStateSet(bool state) {
    if (m_updating) {
        // Let state propagate visually
        return false;
    }
    // Emit signal to parent, which will request the change via TCP
    m_signalToggled.emit(m_relayId, state);
    
    // Return false to allow the switch to slide immediately for smooth UI response.
    // If the server fails to update, the next periodic status poll will correct it.
    return false;
}

} // namespace minihildesk
