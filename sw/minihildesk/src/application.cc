#include "application.h"
#include <iostream>

namespace minihildesk {

EntryApplication::EntryApplication() : Gtk::Application("io.electux.minihildesk") {}

EntryApplication::~EntryApplication() = default;

Glib::RefPtr<EntryApplication> EntryApplication::create() {
    return Glib::make_refptr_for_instance<EntryApplication>(new EntryApplication());
}

void EntryApplication::on_startup() {
    Gtk::Application::on_startup();

    m_config = std::make_unique<ConfigManager>();
    m_controller = std::make_unique<AppController>(*m_config);
    m_home = std::make_unique<AppHome>(*m_controller);

    m_controller->start();

    add_window(*m_home);
}

void EntryApplication::on_activate() {
    Gtk::Application::on_activate();
    if (m_home) {
        m_home->set_visible(true);
    }
}

void EntryApplication::on_shutdown() {
    if (m_controller) {
        m_controller->stop();
    }
    Gtk::Application::on_shutdown();
}

} // namespace minihildesk
