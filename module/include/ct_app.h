#pragma once
#include <memory>

namespace ct::app
{

    struct AppContext
    {
        bool show_menu = false;

        bool esp_enable;
        float esp_distance = 10.0f;
    };

    AppContext *context();
}