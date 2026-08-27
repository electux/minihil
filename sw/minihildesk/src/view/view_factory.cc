////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// view_factory.cc
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
#include <iapp_controller.h>
#include <view/home.h>
#include <view/relay_widget.h>
#include <view/view_factory.h>

namespace minihildesk::View {
std::unique_ptr<IHomeView> createHomeView(IAppController &controller) {
  return std::make_unique<AppHome>(controller);
}
} // namespace minihildesk::View
