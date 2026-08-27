////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// application.cc
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
#include <app_controller_factory.h>
#include <application.h>
#include <config/config_factory.h>
#include <config/iconfig_manager.h>
#include <iapp_controller.h>
#include <iostream>
#include <network/itcp_client.h>
#include <network/tcp_client_factory.h>
#include <view/ihome_view.h>
#include <view/view_factory.h>

namespace {
constexpr std::string_view cApplicationId{"io.electux.minihildesk"};
} // namespace

namespace minihildesk {

EntryApplication::EntryApplication()
    : Gtk::Application(cApplicationId.data()) {}

EntryApplication::~EntryApplication() = default;

Glib::RefPtr<EntryApplication> EntryApplication::create() {
  return Glib::make_refptr_for_instance<EntryApplication>(
      new EntryApplication());
}

void EntryApplication::on_startup() {
  Gtk::Application::on_startup();

  auto config = Config::createConfigManager();
  auto client = Network::createTcpClient();

  m_controller = createAppController(std::move(config), std::move(client));
  m_home = View::createHomeView(*m_controller);

  m_controller->start();

  add_window(m_home->getGtkWindow());
}

void EntryApplication::on_activate() {
  Gtk::Application::on_activate();
  if (m_home) {
    m_home->show();
  }
}

void EntryApplication::on_shutdown() {
  if (m_controller) {
    m_controller->stop();
  }
  Gtk::Application::on_shutdown();
}

} // namespace minihildesk
