#include <iostream>
#include <exception>

#include <fangpp/game_state.hpp>

int main(int argc, char *argv[]) 
{
    if (argc != 4) {
        std::cout << "Usage: ./fangpp <source> <target> <pathLength>\n";
        return -1;
    }
    
    try {
        Game graph("graphs/graph_larger.graphml", 2, 2);
        
        const uint32_t nv = graph.getNVertices();
        std::cout << "#vertices: " << nv << '\n';
        std::cout << "#edges: " << graph.getNEdges() << '\n';
        
        const uint32_t source = static_cast<uint32_t>(std::atoi(argv[1]));
        const uint32_t target = static_cast<uint32_t>(std::atoi(argv[2]));
        const uint32_t pathLength = static_cast<uint32_t>(std::atoi(argv[3]));
                
        // Compute shortest paths
        GraphQuery spQuery = graph.initializeQuery();
        graph.shortestPaths(source, spQuery, false);
        
        std::cout << "Shortest distances starting from vertex " << source << '\n';
        for (uint32_t v = 0; v < nv; ++v)
            std::cout << spQuery.minDistance(v) << ' ';
        std::cout << '\n';
        
        const auto shortestPath = spQuery.followMinPath(target, pathLength);
        std::cout << "End pos. of shortest path from " << source << " to " << target 
                  << " with max. length " << pathLength << ": "
                  << shortestPath;
        
        const auto path = graph.findPathOfLength(source, target, pathLength, false);
        std::cout << "Path from " << source << " to " << target
                  << " with length " << pathLength << '\n';
        for (const auto &vertex : path)
            std::cout << vertex << ' ';
        std::cout << '\n';
        
        if (graph.isValidPath(path, source, target, pathLength, false)) {
            std::cout << "Valid\n";
        } else {
            std::cout << "Invalid\n";
        }
        
        // Find all reachable vertex positions from source
        const auto reachable = graph.findAllReachableVertices(source, pathLength, false);
        std::cout << "\nReachable vertices from " << source << " in exactly "
                  << pathLength << " steps:\n";
        for (const auto v : reachable) {
            std::cout << v << '\n';
        }
        
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return -1;
    }
    
    return 0;    
}
