#include <fangpp/player.hpp>

Move Player::makeMove(Game &state) const 
{
    return moveStrategy->makeMove(state, *this);
}

Player::~Player()
{
    // Cleanup owned move strategy
    delete moveStrategy;
}
