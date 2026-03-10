#include "ct_game.h"
#include <memory>

namespace ct::game
{
    GameState *state()
    {
        static auto t = std::make_unique<GameState>();
        return t.get();
    }
}