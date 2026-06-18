#include <fangpp/game_state.hpp>

#include <stdexcept>

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
        // TODO: For now user is always the first player (red).
        //       Make user choose or assign a random player to them in the future.
        MoveStrategy *strategy;
        if (i == 0)
        {
            strategy = new UserStrategy;
        }
        else
        {
            strategy = new GreedyStrategy;
        }
        
        players.emplace_back(
            i, randomPlayerPos, start, end, strategy,
            initializeQuery(), initializeQuery()
        );
        
        // Initialize player move order
        moveOrder[i] = i;
    }
    // Shuffle move order
    std::shuffle(moveOrder.begin(), moveOrder.end(), prng);
    // Randomize starting position of Boeg to a target position NOT
    // assigned to any player
    boeg = {
        .position = targetVertices[nPlayers * nTargetsPlayer],
        .playerId = nPlayers  // invalid id
    };
    // Reset index of first to move
    moveIndex = 0;
    nActivePlayers = nPlayers;
    
    rollDice();  // initialize m_diceRoll and current player
}

Game::Status Game::makeMove()
{
    if (isGameOver())
    {
        return GAME_OVER;  // nothing to do...
    }
    
    Status status = CONTINUE;
    
    // Fetch next player according to move order
    Player &player = getCurrentPlayer();
    assert(!player.isFinished());  // prepareNextMove() assures the current player is active

    // TODO: Should write makeMoveAs(player&, m_diceRoll) instead
    const std::vector<uint32_t> path = player.makeMove(*this, m_diceRoll);
    if (path.empty() && player.isPlayerUser())
    {
        // The user is allowed to make invalid moves; ignore and try again
        return TRY_AGAIN;
    }
    
    validateMove(player, path, m_diceRoll);  // debug
    printMove(path);  // debug
    
    const uint32_t endPosition = path.back();
    // TODO: Should write isBoeg(player) instead of player.isBoeg(*this)
    if (player.isBoeg(*this)) {
        // Update position of boeg
        boeg.position = endPosition;
        // Check if player hit active target and potentially finished the game as Boeg
        status = checkPlayerFinished(player, endPosition);
    } else {
        // Update position of player
        player.setPosition(endPosition);
        
        if (endPosition == boeg.position) {
            // Player captured Boeg
            boeg.playerId = player.getId();
            // Check if capture position of Boeg is active player target
            status = checkPlayerFinished(player, endPosition);
            status = static_cast<Status>(status | CAPTURE);
        }
    }
    
    return status;
}

void Game::validateMove(const Player &player, const std::vector<uint32_t> &path, 
    const uint32_t diceRoll)
{
    if (path.empty())
    {
        throw std::runtime_error("Invalid empty path encountered!");
    }
    
    const uint32_t maxPathLength = diceRoll + 1;
    if (path.size() > maxPathLength)
    {
        throw std::runtime_error("Path is too long!");
    } 
    
    for (const uint32_t position : path)
    {
        if (position >= getNVertices())
        {
            throw std::runtime_error("Invalid position (vertex index) encountered in path");
        }
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

Game::Status Game::checkPlayerFinished(Player &player, uint32_t endPosition)
{
    assert(player.isBoeg(*this) && "expected player to be playing as Boeg");
    
    Status status = CONTINUE;
    if (player.checkVisitTarget(endPosition)) 
    {
        status = static_cast<Status>(status | TARGET_VISITED);
        if (player.isFinished())
        {
            // Reset id of player playing Boeg
            boeg.playerId = nPlayers;
            // Decrement active player count.
            --nActivePlayers;
            // If at most 1 remaining player or if user has finished, game over
            if (nActivePlayers == 1 || player.isPlayerUser())
            {
                // Unset continue bit
                status = static_cast<Status>(status & ~CONTINUE);
                status = static_cast<Status>(status | GAME_OVER);
            }
        }
    }
    
    return status;
}

Player &Game::getCurrentPlayer()
{
    return players[moveOrder[moveIndex]];    
}

const Player &Game::getUserPlayer() const
{
    for (const auto &player : players)
    {
        if (player.isPlayerUser())
        {
            return player;
        }
    }
    
    throw std::runtime_error("Failed to find player associated with user");
}

uint32_t Game::getCurrentPlayerNumTargets() const
{
    return players[moveOrder[moveIndex]].getActiveTargets().size();
}

bool Game::checkIfUserTurn() const
{
    return players[moveOrder[moveIndex]].isPlayerUser();    
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

void Game::setUserClickedPosition(const uint32_t pos)
{
    assert(checkIfUserTurn());
    
    players[moveOrder[moveIndex]].setPlayerClickedPosition(pos);
}

void Game::prepareNextMove(Status status)
{
    if (isGameOver())
    {
        return;  // nothing to do
    }
    assert(!(status & GAME_OVER));
    // If player captured the Boeg this move, or they made an invalid move,
    // they get to move again immediately. Otherwise, advance index into moveOrder
    if (!(status & TRY_AGAIN) && !(status & CAPTURE)) {
        // Increment index and wrap around
        moveIndex = (moveIndex + 1) % nPlayers;
    }
    // Advance moveIndex until it corresponds to the next active player
    // according to the move order
    while (getCurrentPlayer().isFinished())
    {
        moveIndex = (moveIndex + 1) % nPlayers;
    }
    
    // Roll the dice for the next player
    rollDice();
}

std::array<uint32_t, 7> Game::prepareCharacterPositions() const
{
    assert(players.size() + 1 <= Player::maxPlayableCharacters);
    
    // Assumes that UINT32_MAX is not a valid vertex/position index
    const uint32_t invalidPosition = std::numeric_limits<uint32_t>::max();
    std::array<uint32_t, Player::maxPlayableCharacters> positions;
    positions.fill(invalidPosition);
    
    for (uint32_t i = 0; i < players.size(); ++i)
    {
        const auto &player = players[i];
        
        if (!player.isFinished())
            positions[i] = player.getPosition();
    }
    
    positions.back() = boeg.position;
    
    return positions;
}

bool Game::isGameOver() const
{
    // Either the user playing the game is the only player left,
    // or they have finished the game before at least 1 NPC player
    return nActivePlayers == 1 || getUserPlayer().isFinished();
}

bool Game::isUserPlayingAsBoeg() const
{
    return getUserPlayer().getId() == boeg.playerId;    
}

void Game::rollDice()
{
    std::uniform_int_distribution<uint32_t> dist (1, 6);
    m_diceRoll = dist(prng);    
}

void Game::printMove(const std::vector<uint32_t> &move) const
{
    const auto &vertices = getVertices();
    
    for (uint32_t i = 0; i + 1 < move.size(); ++i) {
        const uint32_t position = move[i];
        const auto &location = vertices[position].location;
        std::cout << location << " -> ";
    }
    
    const auto &location = vertices[move.back()].location;
    std::cout << location;
    std::cout << '\n';
}
