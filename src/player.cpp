#include <fangpp/player.hpp>

std::vector<uint32_t> Player::makeMove(Game &state, const uint32_t diceRoll) 
{
    return moveStrategy->makeMove(state, *this, diceRoll);
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
