#include <fangpp/move_strategy.hpp>

std::vector<uint32_t> MoveStrategy::makeMove(Game &state, Player &player, 
    const uint32_t diceRoll) const
{    
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
    const bool isBoeg = true;  // playing as Boeg
    const uint32_t start = state.getBoegPosition();
    const auto &targets = player.getActiveTargets();
    
    GraphQuery &startQuery = player.getStartQuery();
    GraphQuery &candidateQuery = player.getCandidateQuery();
    // Compute shortest paths from start as Boeg
    state.shortestPaths(start, startQuery, isBoeg);
    
    // Note: Assumes maximum u32 value is never used
    const uint32_t unreachable = std::numeric_limits<uint32_t>::max();
    const double infinity = std::numeric_limits<double>::infinity();
    double minCost = infinity;
    uint32_t bestTarget = unreachable;
    // Define function for updating current minimum of cost function
    const auto minCostUpdate = [this, &state, &player, &targets, &minCost, &bestTarget, &candidateQuery]
        (const uint32_t candidate)
    {
        // Compute shortest paths starting from candidate position
        state.shortestPaths(candidate, candidateQuery, isBoeg);
        double cost = 0.0;
        for (const uint32_t target : targets) 
        {
            // Note: Distance to self is simply 0 for candidate targets
            cost += static_cast<double>(candidateQuery.minDistance(target));
        }
        // Take into account shortest distance from opponents to candidate position.
        // Larger distance from opponent means smaller cost
        for (const uint32_t opponentPos : state.getOpponentPositions(player))
        {
            cost += m_AvoidanceParam / candidateQuery.minDistance(opponentPos);
        }
        // Pick reachable, unoccupied target that is closest to
        // remaining targets
        if (cost < minCost) 
        {
            minCost = cost;
            bestTarget = candidate;
        }
    };
    
    // Search for unoccupied, reachable and closest targets
    for (const uint32_t target : targets) 
    {
        // Keep track of closest target that is NOT already occupied by opponent
        const uint32_t targetDistance = startQuery.minDistance(target);
        // Check if this active target is reachable and NOT already occupied by opponent
        if (diceRoll >= targetDistance && !state.isOpponentAtTarget(player, target)) 
        {
            minCostUpdate(target);
        }
    }
    
    if (bestTarget != unreachable) 
    {
        // Move to reachable, unoccupied target that has the lowest 'cost'
        return startQuery.followMinPath(bestTarget, diceRoll);
    }
    // From here on, unable to reach any unoccupied active target.
    // Iterate over all reachable & unoccupied positions to find one
    // that minimizes the cost function
    minCost = infinity;
    bestTarget = unreachable;
    const auto reachable = state.findAllReachableVertices(start, diceRoll, isBoeg);
    for (const uint32_t position : reachable) 
    {
        // Skip already occupied positions
        if (!state.isOpponentAtTarget(player, position)) 
        { 
            minCostUpdate(position);
        }
    }
    
    if (bestTarget != unreachable) 
    {
        // Found suitable minimizer among reachable positions
        return state.findPathOfLength(start, bestTarget, diceRoll, isBoeg);
    }
    
    // No valid moves available. Simply stay put at start location
    return std::vector<uint32_t>(1, start);
}

std::vector<uint32_t> AvoidantStrategy::movePlayer(Game &state, Player &player,
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

std::vector<uint32_t> UserStrategy::moveBoeg(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    const uint32_t start = state.getBoegPosition();
    
    bool hasReachablePosition = false;
    // Need to first verify if user has any valid moves to begin with
    const auto reachable = state.findAllReachableVertices(start, diceRoll, true);
    for (const uint32_t position : reachable) 
    {
        // Skip already occupied positions
        if (!state.isOpponentAtTarget(player, position)) 
        { 
            hasReachablePosition = true;
            break;  // found at least 1 reachable & valid position
        }
    }
    
    if (!hasReachablePosition)
    {
        // Only valid move by user is to stay put at same position in this case
        return std::vector<uint32_t>(1, start);
    }
    // Boeg is not allowed to move to already occupied position
    if (state.isOpponentAtTarget(player, m_userClickedPosition))
    {
        return {};  // invalid (empty) path
    }
    // In case user (playing as boeg) visited one of their active targets,
    // try to return a shortest path ending at that target position
    for (const uint32_t target : player.getActiveTargets())
    {
        if (m_userClickedPosition == target)
        {
            // Compute shortest paths from boeg position
            GraphQuery &startQuery = player.getStartQuery();
            state.shortestPaths(start, startQuery, true);
        
            if (diceRoll >= startQuery.minDistance(m_userClickedPosition))
            {
                return startQuery.followMinPath(m_userClickedPosition, diceRoll);
            }
            else
            {
                return {};  // invalid (empty) path
            }
        }
    }
    
    // Return a valid simple path ending at clicked position if one exists
    return state.findPathOfLength(state.getBoegPosition(), m_userClickedPosition, diceRoll, true);
}

std::vector<uint32_t> UserStrategy::movePlayer(Game &state, Player &player,
    const uint32_t diceRoll) const
{
    // If user clicked boeg position, check if it is reachable and return
    // a shortest path to that position in that case
    if (m_userClickedPosition == state.getBoegPosition())
    {
        // Compute shortest paths from start as regular player
        GraphQuery &startQuery = player.getStartQuery();
        state.shortestPaths(player.getPosition(), startQuery);
        
        if (diceRoll >= startQuery.minDistance(m_userClickedPosition))
        {
            return startQuery.followMinPath(m_userClickedPosition, diceRoll);
        }
        else
        {
            return {};  // invalid (empty) path
        }
    }
    else
    {
        // Return a valid simple path ending at clicked position if one exists
        return state.findPathOfLength(player.getPosition(), m_userClickedPosition, diceRoll, false);        
    }
}
