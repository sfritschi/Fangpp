#include <fangpp/player.hpp>

std::vector<uint32_t> Player::makeMove(Game &state) const 
{
    return moveStrategy->makeMove(state, *this);
}

bool Player::isBoeg(const Game &state) const
{
    return id == state.getBoegId();
}

Player::~Player()
{
    // Cleanup owned move strategy
    delete moveStrategy;
}
