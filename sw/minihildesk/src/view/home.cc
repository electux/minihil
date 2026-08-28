////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// home.cc
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
#include <chrono>
#include <config/iconfig_manager.h>
#include <gdkmm/display.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <iapp_controller.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <view/home.h>
#include <view/relay_widget.h>
#include <iostream>

namespace {
constexpr std::string_view cTitle{"minihildesk"};
constexpr int cDefaultWidth{1080};
constexpr int cDefaultHeight{820};
constexpr std::string_view cStyleResourcePath{
    "/io/electux/minihildesk/style.css"};

// Controls configuration
constexpr int cIpEntryWidth{15};
constexpr int cPortEntryWidth{6};
constexpr int cHeaderMargin{10};
constexpr int cRelaySpacing{10};
constexpr int cRelayMargin{10};
constexpr int cRelayCols{4};
constexpr int cRelayCount{8};
constexpr int cLogHeight{220};
constexpr int cLogMargin{10};
constexpr int cPollInterval{1000};
constexpr int cDefaultPortFallback{9000};

// Labels
constexpr std::string_view cIpLabel{"IP Address:"};
constexpr std::string_view cPortLabel{"Port:"};
constexpr std::string_view cSslLabel{"SSL"};
constexpr std::string_view cMtlsLabel{"mTLS"};
constexpr std::string_view cConnectLabel{"Connect"};
constexpr std::string_view cDisconnectLabel{"Disconnect"};

// Format
constexpr std::string_view cTimestampFormat{"%Y-%m-%d %H:%M:%S"};
constexpr std::string_view cNewLine{"\n"};
} // namespace

namespace minihildesk::View {

AppHome::AppHome(IAppController &controller)
    : m_controller(controller),
      m_mainBox(Gtk::Orientation::VERTICAL, cHeaderMargin),
      m_headerBox(Gtk::Orientation::HORIZONTAL, cHeaderMargin) {
  m_ipLabel.set_text(cIpLabel.data());
  m_portLabel.set_text(cPortLabel.data());
  m_sslCheck.set_label(cSslLabel.data());
  m_mtlsCheck.set_label(cMtlsLabel.data());
  m_connectBtn.set_label(cConnectLabel.data());

  set_title(cTitle.data());
  set_default_size(cDefaultWidth, cDefaultHeight);
  set_resizable(false);

  // Apply premium styling from compiled resources
  auto cssProvider = Gtk::CssProvider::create();
  cssProvider->load_from_resource(cStyleResourcePath.data());
  Gtk::StyleContext::add_provider_for_display(
      Gdk::Display::get_default(), cssProvider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);

  // Header Setup
  m_ipEntry.set_text(m_controller.getConfig().getIp());
  m_ipEntry.set_width_chars(cIpEntryWidth);
  m_portEntry.set_text(std::to_string(m_controller.getConfig().getPort()));
  m_portEntry.set_width_chars(cPortEntryWidth);
  m_sslCheck.set_active(m_controller.getConfig().getUseSsl());
  m_mtlsCheck.set_active(m_controller.getConfig().getUseMtls());

  m_connectBtn.signal_clicked().connect(
      sigc::mem_fun(*this, &AppHome::onConnectClicked));

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

  m_headerBox.set_margin(cHeaderMargin);
  m_headerBox.get_style_context()->add_class("connection-bar");
  m_connectBtn.get_style_context()->add_class("connect-btn");

  m_headerBox.append(m_ipLabel);
  m_headerBox.append(m_ipEntry);
  m_headerBox.append(m_portLabel);
  m_headerBox.append(m_portEntry);
  m_headerBox.append(m_sslCheck);
  m_headerBox.append(m_mtlsCheck);
  m_headerBox.append(m_connectBtn);

  // Grid of Relays (2 rows x 4 columns)
  m_relayGrid.set_row_spacing(cRelaySpacing);
  m_relayGrid.set_column_spacing(cRelaySpacing);
  m_relayGrid.set_margin(cRelayMargin);

  for (int i = 0; i < cRelayCount; ++i) {
    int relayId = i + 1;
    auto widget = std::make_unique<RelayWidget>(relayId);
    widget->signal_toggled().connect(
        sigc::mem_fun(*this, &AppHome::onRelayToggled));
    widget->signal_timer_started().connect(
        sigc::mem_fun(*this, &AppHome::onRelayTimerStarted));
    widget->signal_pulse_triggered().connect(
        sigc::mem_fun(*this, &AppHome::onRelayPulseTriggered));
    widget->signal_blink_started().connect(
        sigc::mem_fun(*this, &AppHome::onRelayBlinkStarted));

    int row = i / cRelayCols;
    int col = i % cRelayCols;
    m_relayGrid.attach(*widget, col, row, 1, 1);
    m_relayWidgets.push_back(std::move(widget));
  }

  // Traffic Log Setup
  m_logTextView.set_editable(false);
  m_logTextView.set_cursor_visible(false);
  m_logTextView.set_wrap_mode(Gtk::WrapMode::CHAR);
  m_logTextView.set_margin_start(cLogMargin);
  m_logTextView.set_margin_end(cLogMargin);
  m_logTextView.set_margin_bottom(cLogMargin);

  m_logScrolled.set_child(m_logTextView);
  m_logScrolled.set_vexpand(true);
  m_logScrolled.set_hexpand(true);
  m_logScrolled.set_size_request(-1, cLogHeight);
  m_logScrolled.set_margin_start(cLogMargin);
  m_logScrolled.set_margin_end(cLogMargin);
  m_logScrolled.set_margin_bottom(cLogMargin);

  // Main Box
  m_mainBox.append(m_headerBox);
  m_mainBox.append(m_relayGrid);
  m_mainBox.append(m_logScrolled);
  set_child(m_mainBox);

  // Dispatcher connects
  m_logDispatcher.connect(sigc::mem_fun(*this, &AppHome::onLogDispatcher));
  m_relayDispatcher.connect(sigc::mem_fun(*this, &AppHome::onRelayDispatcher));
  m_connectionDispatcher.connect(
      sigc::mem_fun(*this, &AppHome::onConnectionDispatcher));

  // Connect controller signals
  m_controller.signal_log().connect(
      [this](const std::string &msg) { postLogMessage(msg); });
  m_controller.signal_relay_state().connect(
      sigc::mem_fun(*this, &AppHome::onRelayStateUpdated));
  m_controller.signal_connection_state().connect(
      sigc::mem_fun(*this, &AppHome::onConnectionStateUpdated));

  // Setup periodic polling timeout (polls states if connected)
  m_pollConnection = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &AppHome::onPollTimeout), cPollInterval);

  // Set initial sensitivites
  onConnectionStateUpdated(m_controller.isConnected());
}

void AppHome::onConnectClicked() {
  if (m_controller.isConnected()) {
    m_controller.requestDisconnect();
  } else {
    std::string ip = m_ipEntry.get_text();
    int port = cDefaultPortFallback;
    try {
      port = std::stoi(m_portEntry.get_text());
    } catch (...) {
    }
    bool useSsl = m_sslCheck.get_active();
    bool useMtls = m_mtlsCheck.get_active();
    m_controller.requestConnect(ip, port, useSsl, useMtls);
  }
}

void AppHome::onRelayToggled(int relayId, bool state) {
  std::cout << "[AppHome] onRelayToggled for relay " << relayId << ", state: " << state << std::endl;
  m_controller.toggleRelay(relayId, state);
}

void AppHome::onRelayTimerStarted(int relayId, uint32_t seconds) {
  std::cout << "[AppHome] onRelayTimerStarted for relay " << relayId << ", seconds: " << seconds << std::endl;
  m_controller.startTimer(relayId, seconds);
}

void AppHome::onRelayPulseTriggered(int relayId, uint32_t durationMs) {
  std::cout << "[AppHome] onRelayPulseTriggered for relay " << relayId << ", durationMs: " << durationMs << std::endl;
  m_controller.startPulse(relayId, durationMs);
}

void AppHome::onRelayBlinkStarted(int relayId, uint32_t onMs, uint32_t offMs,
                                  uint32_t count) {
  std::cout << "[AppHome] onRelayBlinkStarted for relay " << relayId << ", onMs: " << onMs << ", offMs: " << offMs << ", count: " << count << std::endl;
  m_controller.startBlink(relayId, onMs, offMs, count);
}

void AppHome::postLogMessage(const std::string &msg) {
  std::lock_guard<std::mutex> lock(m_logMutex);

  // Prefix log with timestamp
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), cTimestampFormat.data())
     << " " << msg;

  m_logQueue.push(ss.str());
  m_logDispatcher.emit();
}

void AppHome::onLogDispatcher() {
  std::lock_guard<std::mutex> lock(m_logMutex);
  auto buffer = m_logTextView.get_buffer();
  while (!m_logQueue.empty()) {
    buffer->insert(buffer->end(), m_logQueue.front() + cNewLine.data());
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

    if (update.id >= 1 && update.id <= cRelayCount) {
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
    m_connectBtn.set_label(cDisconnectLabel.data());
    m_ipEntry.set_sensitive(false);
    m_portEntry.set_sensitive(false);
    m_sslCheck.set_sensitive(false);
    m_mtlsCheck.set_sensitive(false);
    for (auto &widget : m_relayWidgets) {
      widget->set_sensitive(true);
    }
  } else {
    m_connectBtn.set_label(cConnectLabel.data());
    m_ipEntry.set_sensitive(true);
    m_portEntry.set_sensitive(true);
    m_sslCheck.set_sensitive(true);
    m_mtlsCheck.set_sensitive(true);
    for (auto &widget : m_relayWidgets) {
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

} // namespace minihildesk::View
