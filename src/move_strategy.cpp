#include <fangpp/move_strategy.hpp>

std::vector<uint32_t> MoveStrategy::makeMove(Game &state, Player &player) const
{    
    const uint32_t diceRoll = state.rollDice();
    
    if (player.isBoeg(state)) {
        return moveBoeg(state, player, diceRoll);
    } else {
        return movePlayer(state, player, diceRoll);
    }
}

std::vector<uint32_t> GreedyStrategy::moveBoeg(Game &state, Player &player, 
    const uint32_t diceRoll) const
{
    const bool isBoeg = true;  // playing as Boeg
    const uint32_t start = state.getBoegPosition();
    const auto &targets = player.getActiveTargets();
    
    GraphQuery &startQuery = player.getStartQuery();
    GraphQuery &candidateQuery = player.getCandidateQuery();
    // Compute shortest paths from start as Boeg
    state.shortestPaths(start, startQuery, isBoeg);
    
    // Note: Assumes maximum u32 value is never used
    const uint32_t unreachable = std::numeric_limits<uint32_t>::max();
    uint32_t minCost = unreachable;
    uint32_t bestTarget = unreachable;
    uint32_t minDistance = unreachable;
    uint32_t closestTarget = unreachable;
    // Define function for updating current minimum of cost function
    const auto minCostUpdate = [&state, &targets, &minCost, &bestTarget, &candidateQuery]
        (const uint32_t candidate)
    {
        // Compute shortest paths starting from candidate position
        state.shortestPaths(candidate, candidateQuery, isBoeg);
        uint32_t cost = 0;
        for (const uint32_t target : targets) {
            // Note: Distance to self is simply 0 for candidate targets
            cost += candidateQuery.minDistance(target);
        }
        // Pick reachable, unoccupied target that is closest to
        // remaining targets
        if (cost < minCost) {
            minCost = cost;
            bestTarget = candidate;
        }
    };
    
    // Search for unoccupied, reachable and closest targets
    for (const uint32_t target : targets) {
        // Keep track of closest target that is NOT already occupied by opponent
        const uint32_t targetDistance = startQuery.minDistance(target);
        if (targetDistance < minDistance) {
            minDistance = targetDistance;
            closestTarget = target;
        }
        // Check if this active target is reachable and NOT already occupied by opponent
        if (diceRoll >= targetDistance && !state.isOpponentAtTarget(player, target)) {
            minCostUpdate(target);
        }
    }
    
    if (bestTarget != unreachable) {
        // Move to reachable, unoccupied target that is closest to the remaining targets
        return startQuery.followMinPath(bestTarget, diceRoll);
    }
    // From here on, unable to reach any unoccupied active target
    
    if (closestTarget != unreachable) {
        // Try following 'diceRoll' many steps along shortest path to closest target
        const auto closest = startQuery.followMinPath(closestTarget, diceRoll);
        if (!state.isOpponentAtTarget(player, closest.back())) {
            return closest;
        }
        // Already occupied by opponent, keep searching...
    }
    // Iterate over all reachable & unoccupied positions to find closest
    // to all active targets
    minCost = unreachable;
    bestTarget = unreachable;
    const auto reachable = state.findAllReachableVertices(start, diceRoll, isBoeg);
    for (const uint32_t position : reachable) {
        // Skip already occupied positions
        if (!state.isOpponentAtTarget(player, position)) { 
            minCostUpdate(position);
        }
    }
    
    if (bestTarget != unreachable) {
        // Found suitable minimizer among reachable positions
        return state.findPathOfLength(start, bestTarget, diceRoll, isBoeg);
    }
    
    // No valid moves available. Simply stay put at start location
    return std::vector<uint32_t>(1, start);
}

std::vector<uint32_t> GreedyStrategy::movePlayer(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    const uint32_t start = player.getPosition();
    // Compute shortest paths from start as regular player
    GraphQuery &startQuery = player.getStartQuery();
    state.shortestPaths(start, startQuery);
    
    // Move 'diceRoll' many steps along shortest path to Boeg.
    // If Boeg is reachable within 'diceRoll' steps, end is simply the
    // position of the boeg
    return startQuery.followMinPath(state.getBoegPosition(), diceRoll);    
}

std::vector<uint32_t> AvoidantStrategy::moveBoeg(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    (void)state;
    (void)player;
    (void)diceRoll;
    throw std::runtime_error("Not implemented yet...");
}

std::vector<uint32_t> AvoidantStrategy::movePlayer(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    (void)state;
    (void)player;
    (void)diceRoll;
    throw std::runtime_error("Not implemented yet...");
}

std::vector<uint32_t> UserStrategy::moveBoeg(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    (void)state;
    (void)player;
    (void)diceRoll;
    throw std::runtime_error("Not implemented yet...");
}

std::vector<uint32_t> UserStrategy::movePlayer(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    (void)state;
    (void)player;
    (void)diceRoll;
    throw std::runtime_error("Not implemented yet...");
}
