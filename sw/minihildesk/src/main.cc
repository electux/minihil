#include "application.h"
#include <csignal>

int main(int argc, char* argv[]) {
    std::signal(SIGPIPE, SIG_IGN);
    auto app = minihildesk::EntryApplication::create();
    return app->run(argc, argv);
}
