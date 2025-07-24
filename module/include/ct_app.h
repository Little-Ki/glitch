#pragma once
#include <memory>

namespace ct::app {
    struct ESP {
        bool enable = false;
    };

    struct AppContext {
        bool show_menu{ false };
        ESP esp;
    };

    AppContext* context();
}