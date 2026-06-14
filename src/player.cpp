#include <fangpp/player.hpp>

std::vector<uint32_t> Player::makeMove(Game &state, const uint32_t diceRoll) 
{
    return moveStrategy->makeMove(state, *this, diceRoll);
}

bool Player::isBoeg(const Game &state) const
{
    return id == state.getBoegId();
}

bool Player::isPlayerUser() const 
{ 
    return moveStrategy->isUserStrategy(); 
}

void Player::setPlayerClickedPosition(const uint32_t pos)
{
    assert(isPlayerUser());
    
    moveStrategy->setUserClickedPosition(pos);
}

Player::~Player()
{
    // Cleanup owned moveStrategy
    delete moveStrategy;
}
