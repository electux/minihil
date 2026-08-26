#pragma once
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/switch.h>
#include <sigc++/sigc++.h>

namespace minihildesk {

class RelayWidget : public Gtk::Frame {
public:
    explicit RelayWidget(int relayId);
    ~RelayWidget() override = default;

    void updateState(bool active);
    bool getState() const;

    sigc::signal<void(int, bool)>& signal_toggled() { return m_signalToggled; }

private:
    bool onStateSet(bool state);

    int m_relayId;
    bool m_active{false};
    bool m_updating{false};

    Gtk::Box m_box{Gtk::Orientation::VERTICAL, 8};
    Gtk::Box m_headerBox{Gtk::Orientation::HORIZONTAL, 5};
    Gtk::Label m_titleLabel;
    Gtk::Label m_indicatorLabel;
    Gtk::Switch m_switch;

    sigc::signal<void(int, bool)> m_signalToggled;
};

} // namespace minihildesk
