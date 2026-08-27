////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// application.h
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

#include <gtkmm/application.h>
#include <memory>

namespace minihildesk {
class IAppController;

namespace View {
class IHomeView;
} // namespace View

class EntryApplication : public Gtk::Application {
public:
  EntryApplication();
  ~EntryApplication() override;

  static Glib::RefPtr<EntryApplication> create();

protected:
  void on_startup() override;
  void on_activate() override;
  void on_shutdown() override;

private:
  std::unique_ptr<IAppController> m_controller;
  std::unique_ptr<View::IHomeView> m_home;
};

} // namespace minihildesk
