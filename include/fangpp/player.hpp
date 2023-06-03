#ifndef FANGPP_PLAYER_HPP
#define FANGPP_PLAYER_HPP

#include <fangpp/move_strategy.hpp>
#include <fangpp/game_state.hpp>

class Game;
class MoveStrategy;
struct Move;

class Player {
public:    
    template <class InputIt>
    Player(uint32_t _position, InputIt first, InputIt last, 
        const MoveStrategy *_moveStrategy) :
            position(_position), activeTargets(first, last), 
                moveStrategy(_moveStrategy), isBoeg(false) {}
    
    Move makeMove(Game &state) const;
    
    uint32_t getPosition() const { return position; }
    
    bool isControllingBoeg() const { return isBoeg; }
    
    ~Player();
private:
    uint32_t position;  // current position (vertex index) of player
    std::unordered_set<uint32_t> activeTargets;  // set of player targets left to visit
    const MoveStrategy *moveStrategy;  // move-making strategy of player (owned)
    bool isBoeg;  // is this player controlling the 'Boeg'?
};

#endif /* FANGPP_PLAYER_HPP */
