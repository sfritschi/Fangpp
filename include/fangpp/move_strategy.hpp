#ifndef FANGPP_MOVE_STRATEGY_HPP
#define FANGPP_MOVE_STRATEGY_HPP

#include <fangpp/game_state.hpp>
#include <fangpp/player.hpp>

// Forward-declarations
class Game;
class Player;

class MoveStrategy {
public:
    /**
     *  Make move based on current state of game
     *  Needed information to make move:
     *  - graph describing playing board (state)
     *  - player position (player)
     *  - active player targets (player)
     *  - isControllingBoeg (player)
     *  - opponent positions (state)
     *  - dice roll (number of eyes)
     */
    std::vector<uint32_t> makeMove(Game &state, Player &player) const;
    
    // Make move as Boeg character
    virtual std::vector<uint32_t> moveBoeg(Game &state, Player &player, const uint32_t diceRoll) const = 0;
    
    // Make move as player character
    virtual std::vector<uint32_t> movePlayer(Game &state, Player &player, const uint32_t diceRoll) const = 0;
    
    virtual ~MoveStrategy() = default;
};

// Greedily make move towards targets without consideration of other players
class GreedyStrategy : public MoveStrategy {
public:
    virtual std::vector<uint32_t> moveBoeg(Game &state, Player &player, const uint32_t diceRoll) const override;
    virtual std::vector<uint32_t> movePlayer(Game &state, Player &player, const uint32_t diceRoll) const override;
};

// Try to avoid players, while also getting closer to own targets 
class AvoidantStrategy : public MoveStrategy {
public:
    virtual std::vector<uint32_t> moveBoeg(Game &state, Player &player, const uint32_t diceRoll) const override;
    virtual std::vector<uint32_t> movePlayer(Game &state, Player &player, const uint32_t diceRoll) const override;
};

// User decides what move to make
class UserStrategy : public MoveStrategy {
public:
    virtual std::vector<uint32_t> moveBoeg(Game &state, Player &player, const uint32_t diceRoll) const override;
    virtual std::vector<uint32_t> movePlayer(Game &state, Player &player, const uint32_t diceRoll) const override;
};

#endif /* FANGPP_MOVE_STRATEGY_HPP */
