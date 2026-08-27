////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// app_controller_factory.cc
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
#include <app_controller.h>
#include <app_controller_factory.h>
#include <config/iconfig_manager.h>
#include <network/itcp_client.h>

namespace minihildesk {
std::unique_ptr<IAppController>
createAppController(std::unique_ptr<Config::IConfigManager> config,
                    std::unique_ptr<Network::ITcpClient> client) {
  return std::make_unique<AppController>(std::move(config), std::move(client));
}
} // namespace minihildesk
