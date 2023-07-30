#ifndef FANGPP_GAME_STATE_HPP
#define FANGPP_GAME_STATE_HPP

#include <fangpp/graph.hpp>
#include <fangpp/player.hpp>
#include <fangpp/move_strategy.hpp>

#include <random>

class Player;

struct Boeg {
    uint32_t position;  // position of Boeg on board
    uint8_t playerId;   // id of player that is currently controlling Boeg
};

class Game : public Graph {
public:

    enum Status {
        CONTINUE = 0,  // move on to next player in order
        CAPTURE,       // same player moves again (Boeg was captured)
        GAME_OVER      // all (but one) player have finished the game
    };
    
    Game(const char *boardFile, const uint8_t _nPlayers, 
        const uint8_t _nTargetsPlayer);
    
    void run();
    
    // Run a single player move of game
    Status nextMove();
    
    std::vector<uint32_t> getOpponentPositions(const Player &player) const;
    
    void validateMove(const Player &player, const std::vector<uint32_t> &path, 
        const uint32_t diceRoll);
    
    bool checkGameOver(Player &player, uint32_t endPosition);
    
    bool isOpponentAtTarget(const Player &player, const uint32_t target) const;
    
    uint32_t getBoegPosition() const { return boeg.position; }
    
    uint8_t getBoegId() const { return boeg.playerId; }
    
    uint32_t rollDice();
    
private:
    
    void printMove(const std::vector<uint32_t> &move) const;
    
    std::vector<Player> players;  // per player data
    std::vector<uint8_t> moveOrder;  // order in which players move
    Boeg boeg;  // special player character
    uint8_t moveIndex;  // current index in moveOrder array
    uint8_t nTargetsPlayer;  // #targets for each player
    uint8_t nPlayers;  // #players playing the game
    uint8_t nActivePlayers;  // #players actively playing the game
    std::mt19937 prng;  // pseudo-random number generator
};

#endif /* FANGPP_GAME_STATE_HPP */
