#include "view/home.h"
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <gdkmm/display.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace minihildesk {

AppHome::AppHome(AppController& controller) : m_controller(controller) {
    set_title("minihildesk");
    set_default_size(720, 560);
    set_resizable(false);

    // Apply premium styling from compiled resources
    auto cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_resource("/io/electux/minihildesk/style.css");
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(), cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Header Setup
    m_ipEntry.set_text(m_controller.getConfig().getIp());
    m_ipEntry.set_width_chars(15);
    m_portEntry.set_text(std::to_string(m_controller.getConfig().getPort()));
    m_portEntry.set_width_chars(6);
    m_sslCheck.set_active(m_controller.getConfig().getUseSsl());
    m_mtlsCheck.set_active(m_controller.getConfig().getUseMtls());

    m_connectBtn.signal_clicked().connect(
        sigc::mem_fun(*this, &AppHome::onConnectClicked)
    );

    // Smart toggling rules: mTLS requires SSL
    m_mtlsCheck.signal_toggled().connect([this]() {
        if (m_mtlsCheck.get_active()) {
            m_sslCheck.set_active(true);
        }
    });
    m_sslCheck.signal_toggled().connect([this]() {
        if (!m_sslCheck.get_active()) {
            m_mtlsCheck.set_active(false);
        }
    });

    m_headerBox.set_margin(10);
    m_headerBox.append(m_ipLabel);
    m_headerBox.append(m_ipEntry);
    m_headerBox.append(m_portLabel);
    m_headerBox.append(m_portEntry);
    m_headerBox.append(m_sslCheck);
    m_headerBox.append(m_mtlsCheck);
    m_headerBox.append(m_connectBtn);

    // Grid of Relays (2 rows x 4 columns)
    m_relayGrid.set_row_spacing(10);
    m_relayGrid.set_column_spacing(10);
    m_relayGrid.set_margin(10);

    for (int i = 0; i < 8; ++i) {
        int relayId = i + 1;
        auto widget = std::make_unique<RelayWidget>(relayId);
        widget->signal_toggled().connect(
            sigc::mem_fun(*this, &AppHome::onRelayToggled)
        );
        
        int row = i / 4;
        int col = i % 4;
        m_relayGrid.attach(*widget, col, row, 1, 1);
        m_relayWidgets.push_back(std::move(widget));
    }

    // Traffic Log Setup
    m_logTextView.set_editable(false);
    m_logTextView.set_cursor_visible(false);
    m_logTextView.set_wrap_mode(Gtk::WrapMode::CHAR);
    m_logTextView.set_margin_start(10);
    m_logTextView.set_margin_end(10);
    m_logTextView.set_margin_bottom(10);

    m_logScrolled.set_child(m_logTextView);
    m_logScrolled.set_vexpand(true);
    m_logScrolled.set_hexpand(true);
    m_logScrolled.set_size_request(-1, 220);
    m_logScrolled.set_margin_start(10);
    m_logScrolled.set_margin_end(10);
    m_logScrolled.set_margin_bottom(10);

    // Main Box
    m_mainBox.append(m_headerBox);
    m_mainBox.append(m_relayGrid);
    m_mainBox.append(m_logScrolled);
    set_child(m_mainBox);

    // Dispatcher connects
    m_logDispatcher.connect(sigc::mem_fun(*this, &AppHome::onLogDispatcher));
    m_relayDispatcher.connect(sigc::mem_fun(*this, &AppHome::onRelayDispatcher));
    m_connectionDispatcher.connect(sigc::mem_fun(*this, &AppHome::onConnectionDispatcher));

    // Connect controller signals
    m_controller.signal_log().connect([this](const std::string& msg) { postLogMessage(msg); });
    m_controller.signal_relay_state().connect(sigc::mem_fun(*this, &AppHome::onRelayStateUpdated));
    m_controller.signal_connection_state().connect(sigc::mem_fun(*this, &AppHome::onConnectionStateUpdated));

    // Setup periodic polling timeout (polls states if connected)
    m_pollConnection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &AppHome::onPollTimeout), 1000
    );

    // Set initial sensitivites
    onConnectionStateUpdated(m_controller.isConnected());
}

void AppHome::onConnectClicked() {
    if (m_controller.isConnected()) {
        m_controller.requestDisconnect();
    } else {
        std::string ip = m_ipEntry.get_text();
        int port = 9000;
        try {
            port = std::stoi(m_portEntry.get_text());
        } catch (...) {}
        bool useSsl = m_sslCheck.get_active();
        bool useMtls = m_mtlsCheck.get_active();
        m_controller.requestConnect(ip, port, useSsl, useMtls);
    }
}

void AppHome::onRelayToggled(int relayId, bool state) {
    m_controller.toggleRelay(relayId, state);
}

void AppHome::postLogMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    // Prefix log with timestamp
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << " " << msg;

    m_logQueue.push(ss.str());
    m_logDispatcher.emit();
}

void AppHome::onLogDispatcher() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    auto buffer = m_logTextView.get_buffer();
    while (!m_logQueue.empty()) {
        buffer->insert(buffer->end(), m_logQueue.front() + "\n");
        m_logQueue.pop();
    }

    // Scroll to end of text view
    auto mark = buffer->get_insert();
    m_logTextView.scroll_to(mark, 0.0);
}

void AppHome::onRelayStateUpdated(int relayId, bool state) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_relayQueue.push(RelayStateUpdate{relayId, state});
    m_relayDispatcher.emit();
}

void AppHome::onRelayDispatcher() {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    while (!m_relayQueue.empty()) {
        auto update = m_relayQueue.front();
        m_relayQueue.pop();

        if (update.id >= 1 && update.id <= 8) {
            m_relayWidgets[update.id - 1]->updateState(update.state);
        }
    }
}

void AppHome::onConnectionStateUpdated(bool connected) {
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    m_connectionState = connected;
    m_connectionDispatcher.emit();
}

void AppHome::onConnectionDispatcher() {
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    if (m_connectionState) {
        m_connectBtn.set_label("Disconnect");
        m_ipEntry.set_sensitive(false);
        m_portEntry.set_sensitive(false);
        m_sslCheck.set_sensitive(false);
        m_mtlsCheck.set_sensitive(false);
        for (auto& widget : m_relayWidgets) {
            widget->set_sensitive(true);
        }
    } else {
        m_connectBtn.set_label("Connect");
        m_ipEntry.set_sensitive(true);
        m_portEntry.set_sensitive(true);
        m_sslCheck.set_sensitive(true);
        m_mtlsCheck.set_sensitive(true);
        for (auto& widget : m_relayWidgets) {
            widget->updateState(false);
            widget->set_sensitive(false);
        }
    }
}

bool AppHome::onPollTimeout() {
    if (m_controller.isConnected()) {
        m_controller.queryAllRelays();
    }
    return true; // Keep timer running
}

} // namespace minihildesk
