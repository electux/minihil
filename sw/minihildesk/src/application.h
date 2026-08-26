#pragma once
#include <gtkmm/application.h>
#include <memory>
#include "config/config_manager.h"
#include "app_controller.h"
#include "view/home.h"

namespace minihildesk {

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
    std::unique_ptr<ConfigManager> m_config;
    std::unique_ptr<AppController> m_controller;
    std::unique_ptr<AppHome> m_home;
};

} // namespace minihildesk
