#include <fangpp/game_state.hpp>

Game::Game(const char *boardFile, const uint32_t _nPlayers, 
    const uint32_t _nTargetsPlayer) :
        Graph(boardFile), moveOrder(_nPlayers),
            nPlayers(_nPlayers), nTargetsPlayer(_nTargetsPlayer)
{
    if (nPlayers == 0) {
        throw std::invalid_argument("Non-zero number of players required");
    }
    
    players.reserve(nPlayers);
    for (uint32_t i = 0; i < nPlayers; ++i) {
        // TODO: Shuffle targets beforehand
        auto targets = getPlayerTargets(i * nPlayers, nTargetsPlayer);
        
        players.emplace_back(
            0, targets.begin(), targets.end(), new GreedyStrategy
        );
        
        // TODO: Randomize move order
        moveOrder[i] = i;
    }
    
    // TODO: Randomize starting position of Boeg
    boegPosition = 0;
}

std::vector<uint32_t> Game::getOpponentPositions(const uint32_t playerIndex) const
{
    if (playerIndex >= nPlayers) {
        throw std::invalid_argument("Player index out of bounds");
    }
    
    std::vector<uint32_t> opponents(nPlayers - 1);
    uint32_t index = 0;
    for (const auto &player : players) {
        if (index != playerIndex) {
            opponents[index++] = player.getPosition();
        }
    }
    
    return opponents;
}
