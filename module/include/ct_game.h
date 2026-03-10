#pragma once
#include <cstdint>

namespace ct::game
{
    struct GameState
    {
        uintptr_t handle;
    };

    GameState* state();
}