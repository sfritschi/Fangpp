#ifndef FANGPP_GAME_STATE_HPP
#define FANGPP_GAME_STATE_HPP

#include <fangpp/graph.hpp>
#include <fangpp/player.hpp>
#include <fangpp/move_strategy.hpp>

class Player;

class Game : public Graph {
public:
    // TODO: Randomize move order and player positions/targets
    Game(const char *boardFile, const uint32_t _nPlayers, 
        const uint32_t _nTargetsPlayer);
    
    void run();
    
    std::vector<uint32_t> getOpponentPositions(const uint32_t playerIndex) const;
    
private:
    std::vector<Player> players;  // per player data
    std::vector<uint32_t> moveOrder;  // order in which players move
    uint32_t nPlayers;  // #players playing the game
    uint32_t nTargetsPlayer;  // #targets for each player
    uint32_t boegPosition;  // position of Boeg on board
};

#endif /* FANGPP_GAME_STATE_HPP */
