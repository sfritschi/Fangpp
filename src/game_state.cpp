#include <fangpp/game_state.hpp>

Game::Game(const char *boardFile, const uint8_t _nPlayers, 
    const uint8_t _nTargetsPlayer) :
        Graph(boardFile), moveOrder(_nPlayers),
            nTargetsPlayer(_nTargetsPlayer), nPlayers(_nPlayers)
{
    if (nPlayers <= 1) {
        throw std::invalid_argument("Require at least 2 players to play");
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
    std::uniform_int_distribution<uint32_t> dist (
        0, static_cast<uint32_t>(stationVertices.size() - 1)
    );
    // Seed pseudo random number generator 
    // (non-deterministically if hardware allows for it)
    //std::random_device rd;
    //prng.seed(rd());
    prng.seed(42);  // debug: use fixed seed
    
    players.reserve(nPlayers);
    
    // Shuffle targets beforehand
    std::shuffle(targetVertices.begin(), targetVertices.end(), prng);
    for (uint8_t i = 0; i < nPlayers; ++i) {
        const auto start = targetVertices.begin() + i * nTargetsPlayer;
        const auto end   = start + nTargetsPlayer;
        
        // Generate random player position from stations
        const uint32_t randomPlayerPos = stationVertices[dist(prng)];
        
        players.emplace_back(
            i, randomPlayerPos, start, end, new GreedyStrategy,
            initializeQuery(), initializeQuery()
        );
        
        // Initialize player move order
        moveOrder[i] = i;
    }
    // Shuffle move order
    std::shuffle(moveOrder.begin(), moveOrder.end(), prng);
    // Randomize starting position of Boeg to a target position NOT
    // assigned to any player
    boeg.position = targetVertices[nPlayers * nTargetsPlayer];
    boeg.playerId = nPlayers;  // invalid id
    // Reset index of first to move
    moveIndex = 0;
    nActivePlayers = nPlayers;
}

void Game::run()
{
    std::vector<uint32_t> path;
    
    while (nActivePlayers > 1) {
        for (uint8_t i = 0; i < nPlayers; ++i) {
            // Fetch next player according to move order
            Player &player = players[moveOrder[i]];
            if (!player.isFinished()) {
move_again:                 
                const uint32_t diceRoll = rollDice();
                path = player.makeMove(*this, diceRoll);
                printMove(path);  // debug
                validateMove(player, path, diceRoll);  // debug
                
                const uint32_t endPosition = path.back();
                
                if (player.isBoeg(*this)) {
                    // Update position of boeg
                    boeg.position = endPosition;
                    // Check if player hit active target as Boeg
                    if (checkGameOver(player, endPosition))
                    {
                        break;
                    }
                } else {
                    // Update position of player
                    player.setPosition(endPosition);
                    
                    if (endPosition == boeg.position) {
                        // Player captured Boeg
                        boeg.playerId = player.getId();
                        // Check if capture position of Boeg is active player target
                        if (checkGameOver(player, endPosition))
                        {
                            break;
                        }
                        // Move again, now playing as Boeg
                        goto move_again;
                    }
                }
            }
        }
    }
}

Game::Status Game::nextMove()
{
    Status status = CONTINUE;
    
    // Fetch next player according to move order
    Player &player = players[moveOrder[moveIndex]];
    if (!player.isFinished())
    {
        const uint32_t diceRoll = rollDice();
        const std::vector<uint32_t> path = player.makeMove(*this, diceRoll);
        printMove(path);  // debug
        validateMove(player, path, diceRoll);  // debug
        
        const uint32_t endPosition = path.back();
        
        if (player.isBoeg(*this)) {
            // Update position of boeg
            boeg.position = endPosition;
            // Check if player hit active target as Boeg
            if (checkGameOver(player, endPosition))
            {
                return GAME_OVER;
            }
        } else {
            // Update position of player
            player.setPosition(endPosition);
            
            if (endPosition == boeg.position) {
                // Player captured Boeg
                boeg.playerId = player.getId();
                // Check if capture position of Boeg is active player target
                if (checkGameOver(player, endPosition))
                {
                    return GAME_OVER;
                }
                // Move again, now playing as Boeg
                status = CAPTURE;
            }
        }
    }
    
    // If player captured the Boeg this move, they get to move again immediately
    if (status != CAPTURE)
    {
        // Increment index and wrap around
        moveIndex = (moveIndex + 1) % nPlayers;
    }
    
    return status;
}

void Game::validateMove(const Player &player, const std::vector<uint32_t> &path, 
    const uint32_t diceRoll)
{
    const uint32_t maxPathLength = diceRoll + 1;
    if (path.size() > maxPathLength)
    {
        throw std::runtime_error("Path is too long!");
    } 
    
    // Check if path is plausible
    const bool isBoeg = player.isBoeg(*this);
    const uint32_t startPosition = (isBoeg) ? boeg.position : player.getPosition();
    const uint32_t endPosition = path.back();
    
    if (!isValidPath(path, startPosition, isBoeg))
    {
        throw std::runtime_error("Path is not a valid simple path!");
    }
    
    // In case player travelled less than 'diceRoll', check if
    // this was a valid move
    if (path.size() < maxPathLength)
    {
        if (isBoeg)
        {
            // Check if player had no valid moves
            if (path.size() == 1)
            {
                // Check if there are any unoccupied targets within reach
                shortestPaths(path[0], query, isBoeg);
                for (const uint32_t target : player.getActiveTargets())
                {
                    if (diceRoll >= query.minDistance(target) &&
                        !isOpponentAtTarget(player, target))
                    {
                        throw std::runtime_error("No player move while there is an active unoccupied target in reach!");
                    }
                }
                
                const auto reachable = findAllReachableVertices(path[0], diceRoll, isBoeg);
                for (const uint32_t position : reachable)
                {
                    if (!isOpponentAtTarget(player, position))
                    {
                        throw std::runtime_error("No player move while there is a valid reachable position!");
                    }
                }
            }
            else if (!player.isActiveTarget(endPosition) ||
                     isOpponentAtTarget(player, endPosition))
            {
                throw std::runtime_error("Path is too short for not visiting an (unoccupied) active target!");
            }
        }
        else
        {
            // Verify that player captured the Boeg
            if (endPosition != boeg.position)
            {
                throw std::runtime_error("Path is too short for not capturing the Boeg!");
            }
        }
    }
    else
    {
        if (isBoeg && isOpponentAtTarget(player, endPosition))
        {
            throw std::runtime_error("Player tried to move on occupied position as Boeg!");
        }
    }
}

bool Game::checkGameOver(Player &player, uint32_t endPosition)
{
    assert(player.isBoeg(*this) && "expected player to be playing as Boeg");
    
    if (player.checkVisitTarget(endPosition) && player.isFinished()) 
    {
        // Reset id of player playing Boeg
        boeg.playerId = nPlayers;
        // Decrement active player count.
        // If at most 1 remaining player, game over
        return --nActivePlayers <= 1;
    }
    
    return false;
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

void Game::printMove(const std::vector<uint32_t> &move) const
{
    for (const auto position : move) {
        std::cout << position << " ";
    }
    std::cout << '\n';
}
