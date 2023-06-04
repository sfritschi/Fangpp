#ifndef FANGPP_GRAPH_HPP
#define FANGPP_GRAPH_HPP

#include <cstdint>
#include <cassert>
#include <vector>
#include <list>
#include <tuple>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <span>
#include <iostream>  // Debug
#include <fstream>
#include <string>
#include <sstream>
#include <numeric>
#include <algorithm>

#include "pugixml.hpp"  // Parsing .graphml files

struct GraphQuery {
    GraphQuery() = default;
    
    GraphQuery(const uint32_t nVertices) :
        distances(nVertices),
        children(nVertices),
        visited(nVertices) {}
    
    void reset() 
    {
        for (uint32_t i = 0; i < distances.size(); ++i) {
            distances[i] = 0;
            children[i] = 0;
            visited[i] = 0;
        }
    }
    
    uint32_t minDistance(const uint32_t target) const
    { 
        assert(target < distances.size() && "invalid target location");
        
        return distances[target];
    }
    
    std::vector<uint32_t> followMinPath(const uint32_t target, 
        const uint32_t maxPathLength) const
    {
        assert(target < distances.size() && "invalid target location");
        
        const uint32_t distance = distances[target];
        const uint32_t pathLength = std::min(distance, maxPathLength);
        // Follow path in reverse order: "children" are actually parents in this case
        std::vector<uint32_t> path(pathLength + 1);
        for (uint32_t v = target, i = distance ;; v = children[v], --i) {
            if (i <= pathLength)
                path[i] = v;
            // Last iteration
            if (i == 0) break;
        }
        
        return path;
    }
    
    std::vector<uint32_t> distances;  // distance from source to each vertex
    std::vector<uint32_t> children;   // used to reconstruct path from source to target
    std::vector<uint8_t> visited;     // 1 if vertex has been visited before, 0 otherwise
};

struct Edge {
    Edge() = default;
    
    Edge(uint32_t _nborId, uint8_t _isBoegOnly) :
        nborId(_nborId), isBoegOnly(_isBoegOnly) {}
        
    uint32_t nborId;     // vertex id of neighboring "target"-vertex
    uint8_t isBoegOnly;  // this edge is accessible only to the Boeg
};

struct Vertex {
    Vertex() = default;
    
    Vertex(const std::string &_location, float _xpos, float _ypos) :
        location(_location), xpos(_xpos), ypos(_ypos) {}
        
    std::string location;  // name of location represented by vertex
    float xpos;            // screen x-position of vertex
    float ypos;            // screen y-position of vertex
};

class Graph {
public:    
    enum GRAPH_TYPE {
        GRAPH_DIRECTED = 0,
        GRAPH_UNDIRECTED
    };
    
    Graph(const char *graphFile);
    
    uint32_t getNVertices() const noexcept { return nVertices; }
    uint32_t getNEdges() const noexcept { 
        return (graphType == GRAPH_DIRECTED) ? nEdges : nEdges / 2;
    }
    
    GRAPH_TYPE getGraphType() const noexcept { return graphType; }
    
    GraphQuery initializeQuery() const;
    
    void shortestPaths(const uint32_t source, GraphQuery &spQuery,
        const bool isBoeg = false) const;
    
    std::vector<uint32_t> findPathOfLength(const uint32_t source, 
        const uint32_t target, const uint32_t pathLength, 
        const bool isBoeg = false);
    
    std::unordered_set<uint32_t> findAllReachableVertices(
        const uint32_t source, const uint32_t pathLength, 
        const bool isBoeg = false);
    
    bool isValidPath(const std::vector<uint32_t> &path, const uint32_t source,
        const uint32_t target, const uint32_t pathLength, const bool isBoeg);
    
private:
    void setVertexFromEntry(Vertex &vert, const std::string &name, 
        const std::string &value);
        
    bool findPathOfLengthRecursive(const uint32_t v, const uint32_t target,
        const uint32_t pathLength, const bool isBoeg);
    
    void findAllReachableVerticesRecursive(const uint32_t v, 
        const uint32_t pathLength, const bool isBoeg, 
        std::unordered_set<uint32_t> &reachable);
        
    std::pair<uint32_t,uint32_t> vertexBounds(const uint32_t v) const;
    
    GraphQuery query;                       // used to query graph (finding paths)
    uint32_t nVertices;                     // #vertices of graph
    uint32_t nEdges;                        // #edges of graph
    std::vector<Edge> edges;                // contiguous array of edges
    std::vector<Vertex> vertices;           // contiguous array of vertices
    std::vector<uint32_t> offsets;          // offsets to start of edge list for each vertex
    GRAPH_TYPE graphType;                   // type of graph (dir./undir.)
    bool isValidShortestPathsQuery;         // flag to detect if query contains valid shortest paths

protected:
    std::vector<uint32_t> targetVertices;   // special vertices marking target locations
    std::vector<uint32_t> stationVertices;  // regular vertices marking (non-target) stations
};

#endif /* FANGPP_GRAPH_HPP */
