#pragma once

#include <string>
#include <iostream>

namespace cl::console {
    bool allocate(const std::string &title);

    void detach();
}