#ifndef FANGPP_GAME_STATE_HPP
#define FANGPP_GAME_STATE_HPP

#include <fangpp/graph.hpp>
#include <fangpp/player.hpp>
#include <fangpp/move_strategy.hpp>

#include <random>

class Player;

class Game : public Graph {
public:
    Game(const char *boardFile, const uint8_t _nPlayers, 
        const uint8_t _nTargetsPlayer);
    
    void run();
    
    std::vector<uint32_t> getOpponentPositions(const Player &player) const;
    
    bool isOpponentAtTarget(const Player &player, const uint32_t target) const;
    
    uint32_t getBoegPosition() const { return boegPosition; }
    
    uint32_t rollDice();
    
private:
    std::vector<Player> players;  // per player data
    std::vector<uint8_t> moveOrder;  // order in which players move
    uint32_t boegPosition;  // position of Boeg on board
    uint8_t nTargetsPlayer;  // #targets for each player
    uint8_t nPlayers;  // #players playing the game
    std::mt19937 prng;  // pseudo-random number generator
};

#endif /* FANGPP_GAME_STATE_HPP */
