#ifndef FANGPP_MOVE_STRATEGY_HPP
#define FANGPP_MOVE_STRATEGY_HPP

#include <fangpp/game_state.hpp>
#include <fangpp/player.hpp>

// Forward-declarations
class Game;
class Player;

struct Move {
    enum STATUS {
        PLAYING = 0,  // continue playing as normal
        MOVE_AGAIN,   // if player catches up to Boeg, move a second time playing as Boeg
        HIT_TARGET,   // player has hit one of its active targets, while playing as Boeg
        FINISHED,     // player finished the game
        INVALID
    };
    
    Move(STATUS _status = INVALID) : status(_status) {}
    Move(uint32_t _startPosition, uint32_t _endPosition) :
        startPosition(_startPosition), endPosition(_endPosition), status(INVALID) {}
        
    uint32_t startPosition;
    uint32_t endPosition;
    STATUS status;
};

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
     */
    virtual Move makeMove(Game &state, const Player &player) const = 0;
    
    virtual ~MoveStrategy() = default;
};

// Greedily make move towards targets without consideration of other players
class GreedyStrategy : public MoveStrategy {
public:
    virtual Move makeMove(Game &state, const Player &player) const override;    
};

// Try to avoid players, while also getting closer to own targets 
class AvoidantStrategy : public MoveStrategy {
public:
    virtual Move makeMove(Game &state, const Player &player) const override;    
};

// User decides what move to make
class UserStrategy : public MoveStrategy {
public:
    virtual Move makeMove(Game &state, const Player &player) const override;
};

#endif /* FANGPP_MOVE_STRATEGY_HPP */
