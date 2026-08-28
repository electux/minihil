////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// home.h
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

#include <glibmm/dispatcher.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <view/ihome_view.h>

namespace minihildesk {
class IAppController;
}

namespace minihildesk::View {
class RelayWidget;

class AppHome : public Gtk::ApplicationWindow, public IHomeView {
public:
  AppHome(IAppController &controller);
  ~AppHome() override = default;

  void postLogMessage(const std::string &msg) override;
  void show() override { set_visible(true); }
  void hide() override { set_visible(false); }
  Gtk::Window &getGtkWindow() override { return *this; }

private:
  void onConnectClicked();
  void onRelayToggled(int relayId, bool state);
  void onRelayTimerStarted(int relayId, uint32_t seconds);
  void onRelayPulseTriggered(int relayId, uint32_t durationMs);
  void onRelayBlinkStarted(int relayId, uint32_t onMs, uint32_t offMs,
                           uint32_t count);

  // Signal handlers from controller
  void onRelayStateUpdated(int relayId, bool state);
  void onConnectionStateUpdated(bool connected);

  // Thread-safe UI update handlers
  void onLogDispatcher();
  void onRelayDispatcher();
  void onConnectionDispatcher();

  bool onPollTimeout();

  IAppController &m_controller;

  // Layout boxes
  Gtk::Box m_mainBox;
  Gtk::Box m_headerBox;
  Gtk::Grid m_relayGrid;
  Gtk::ScrolledWindow m_logScrolled;
  Gtk::TextView m_logTextView;

  // Header controls
  Gtk::Label m_ipLabel;
  Gtk::Entry m_ipEntry;
  Gtk::Label m_portLabel;
  Gtk::Entry m_portEntry;
  Gtk::CheckButton m_sslCheck;
  Gtk::CheckButton m_mtlsCheck;
  Gtk::Button m_connectBtn;

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

} // namespace minihildesk::View
