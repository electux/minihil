////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// ihome_view.h
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

#include <string>

namespace Gtk {
class Window;
}

namespace minihildesk::View {
class IHomeView {
public:
  virtual ~IHomeView() = default;

  virtual void postLogMessage(const std::string &msg) = 0;
  virtual void show() = 0;
  virtual void hide() = 0;
  virtual Gtk::Window &getGtkWindow() = 0;
};
} // namespace minihildesk::View
