#include <fangpp/game_state.hpp>

Game::Game(const char *boardFile, const uint8_t _nPlayers, 
    const uint8_t _nTargetsPlayer) :
        Graph(boardFile), moveOrder(_nPlayers),
            nTargetsPlayer(_nTargetsPlayer), nPlayers(_nPlayers)
{
    if (nPlayers == 0) {
        throw std::invalid_argument("Non-zero number of players required");
    }
    
    // Note: + 1 for random Boeg initial position
    const uint8_t minTargets = nPlayers * nTargetsPlayer + 1;
    if (targetVertices.size() < minTargets) {
        throw std::runtime_error("Require at least " + 
            std::to_string(minTargets) + " unique target vertices");
    }
    
    if (stationVertices.size() == 0) {
        throw std::runtime_error("Require at least 1 non-target vertex");
    }
    
    // Seed pseudo random number generator 
    // (non-deterministically if hardware allows for it)
    std::random_device rd;
    prng.seed(rd());
    
    players.reserve(nPlayers);
    
    // Shuffle targets beforehand
    std::shuffle(targetVertices.begin(), targetVertices.end(), prng);
    for (uint8_t i = 0; i < nPlayers; ++i) {
        const auto start = targetVertices.begin() + i * nTargetsPlayer;
        const auto end   = start + nTargetsPlayer;
        
        // Generate random player position from stations
        std::uniform_int_distribution<uint32_t> dist (
            0, static_cast<uint32_t>(stationVertices.size() - 1)
        );
        const uint32_t randomPlayerPos = stationVertices[dist(prng)];
        
        players.emplace_back(
            i, randomPlayerPos, start, end, new GreedyStrategy
        );
        
        // Initialize player move order
        moveOrder[i] = i;
    }
    // Shuffle move order
    std::shuffle(moveOrder.begin(), moveOrder.end(), prng);
    // Randomize starting position of Boeg to a target position NOT
    // assigned to any player
    boegPosition = targetVertices[nPlayers * nTargetsPlayer];
}

// TODO: Maybe use std::list instead of std::vector
std::vector<uint32_t> Game::getOpponentPositions(const Player &player) const
{
    std::vector<uint32_t> opponents;
    opponents.reserve(nPlayers - 1);
    for (const Player &opponent : players) {
        if (opponent != player && !opponent.isFinished()) {
            opponents.push_back(opponent.getPosition());
        }
    }
    
    return opponents;
}

bool Game::isOpponentAtTarget(const Player &player, const uint32_t target) const
{
    for (const Player &opponent : players) {
        if (opponent != player && !opponent.isFinished() &&
            opponent.getPosition() == target)
        {
            return true;
        }
    }
    
    return false;
}

uint32_t Game::rollDice()
{
    std::uniform_int_distribution<uint32_t> dist (1, 6);
    return dist(prng);
}
