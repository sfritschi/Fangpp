#ifndef FANGPP_PLAYER_HPP
#define FANGPP_PLAYER_HPP

#include <fangpp/move_strategy.hpp>
#include <fangpp/game_state.hpp>

class Game;
class MoveStrategy;

class Player {
public:    
    template <class InputIt>
    Player(uint8_t _id, uint32_t _position, InputIt first, InputIt last, 
        const MoveStrategy *_moveStrategy) :
            position(_position), activeTargets(first, last), 
                moveStrategy(_moveStrategy), id(_id) {}
    
    std::vector<uint32_t> makeMove(Game &state) const;
    
    uint32_t getPosition() const { return position; }
    
    uint8_t getId() const { return id; }
    
    void setPosition(const uint32_t newPosition) { position = newPosition; }
    
    const std::unordered_set<uint32_t> &getActiveTargets() const { return activeTargets; }
    
    bool isBoeg(const Game &state) const;
    
    bool isFinished() const { return activeTargets.empty(); }
    
    bool checkVisitTarget(const uint32_t candidate) 
    { 
        // Try removing candidate position. If successful, return true
        const std::size_t nRemoved = activeTargets.erase(candidate);
        return nRemoved == 1;
    }
    
    bool operator==(const Player &other) const { return id == other.id; }
    
    bool operator!=(const Player &other) const { return id != other.id; }
    
    ~Player();
private:    
    uint32_t position;  // current position (vertex index) of player
    std::unordered_set<uint32_t> activeTargets;  // set of player targets left to visit
    const MoveStrategy *moveStrategy;  // move-making strategy of player (owned)
    const uint8_t id;  // unique number identifying this player
};

#endif /* FANGPP_PLAYER_HPP */
