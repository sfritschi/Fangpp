#ifndef FANGPP_PLAYER_HPP
#define FANGPP_PLAYER_HPP

#include <fangpp/move_strategy.hpp>
#include <fangpp/game_state.hpp>
#include <fangpp/graph.hpp>

#include <memory>

class Game;
class MoveStrategy;

class Player {
public:
    using const_iterator_t = std::vector<uint32_t>::const_iterator;
        
    Player(uint8_t _id, uint32_t _position, const_iterator_t first, const_iterator_t last, 
        MoveStrategy *_moveStrategy, GraphQuery sQuery, GraphQuery cQuery) :
            position(_position), activeTargets(first, last), 
                moveStrategy(_moveStrategy), id(_id), startQuery(sQuery),
                    candidateQuery(cQuery) {}
    
    std::vector<uint32_t> makeMove(Game &state, const uint32_t diceRoll);
    
    uint32_t getPosition() const { return position; }
    
    uint8_t getId() const { return id; }
    
    GraphQuery &getStartQuery() { return startQuery; }
    GraphQuery &getCandidateQuery() { return candidateQuery; }
    
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
    
    bool isActiveTarget(const uint32_t candidate) const
    {
        return activeTargets.contains(candidate);
    }
    
    // Returns true if this player is controlled by the user of this program
    bool isPlayerUser() const;
    
    void setPlayerClickedPosition(const uint32_t pos);
    
    bool operator==(const Player &other) const { return id == other.id; }
    
    bool operator!=(const Player &other) const { return id != other.id; }
        
    ~Player();
    
    static const constexpr uint32_t maxPlayableCharacters = 7;  // 6 players + 1 for boeg
    
private:   
    uint32_t position;  // current position (vertex index) of player
    std::unordered_set<uint32_t> activeTargets;  // set of player targets left to visit
    MoveStrategy *moveStrategy;  // move-making strategy of player (owned)
    const uint8_t id;  // unique number identifying this player
    GraphQuery startQuery;  // Used to query shortest distances from starting position
    GraphQuery candidateQuery;  // Used to query shortest distances from candidate position
};

#endif /* FANGPP_PLAYER_HPP */
