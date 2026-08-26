#pragma once
#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/checkbutton.h>
#include <glibmm/dispatcher.h>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include "app_controller.h"
#include "view/relay_widget.h"

namespace minihildesk {

class AppHome : public Gtk::ApplicationWindow {
public:
    AppHome(AppController& controller);
    ~AppHome() override = default;

    void postLogMessage(const std::string& msg);

private:
    void onConnectClicked();
    void onRelayToggled(int relayId, bool state);
    
    // Signal handlers from controller
    void onRelayStateUpdated(int relayId, bool state);
    void onConnectionStateUpdated(bool connected);

    // Thread-safe UI update handlers
    void onLogDispatcher();
    void onRelayDispatcher();
    void onConnectionDispatcher();
    
    bool onPollTimeout();

    AppController& m_controller;

    // Layout boxes
    Gtk::Box m_mainBox{Gtk::Orientation::VERTICAL, 10};
    Gtk::Box m_headerBox{Gtk::Orientation::HORIZONTAL, 10};
    Gtk::Grid m_relayGrid;
    Gtk::ScrolledWindow m_logScrolled;
    Gtk::TextView m_logTextView;

    // Header controls
    Gtk::Label m_ipLabel{"IP Address:"};
    Gtk::Entry m_ipEntry;
    Gtk::Label m_portLabel{"Port:"};
    Gtk::Entry m_portEntry;
    Gtk::CheckButton m_sslCheck{"SSL"};
    Gtk::CheckButton m_mtlsCheck{"mTLS"};
    Gtk::Button m_connectBtn{"Connect"};

    // 8 Relay Widgets
    std::vector<std::unique_ptr<RelayWidget>> m_relayWidgets;

    // Thread synchronization
    Glib::Dispatcher m_logDispatcher;
    std::queue<std::string> m_logQueue;
    std::mutex m_logMutex;

    Glib::Dispatcher m_relayDispatcher;
    struct RelayStateUpdate {
        int id;
        bool state;
    };
    std::queue<RelayStateUpdate> m_relayQueue;
    std::mutex m_relayMutex;

    Glib::Dispatcher m_connectionDispatcher;
    bool m_connectionState{false};
    std::mutex m_connectionMutex;

    // Polling connection
    sigc::connection m_pollConnection;
};

} // namespace minihildesk
