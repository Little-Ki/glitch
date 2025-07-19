#pragma once
#include <memory>

namespace ct::app {

    struct AppContext {
        bool show_menu{ false };
    };

    AppContext* context();

}