#include <fangpp/graph.hpp>

Graph::Graph(const char *graphFile)
{
    // Parse XML file containing graph data
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(graphFile);
    
    if (!result) {
        throw std::runtime_error("Parse error: " + std::string(result.description()));
    }
    
    // Root node of XML document
    pugi::xml_node graphml = doc.child("graphml");
    if (!graphml) { throw std::runtime_error("Not a valid GraphML file"); }
    
    // Process keys defining attributes of vertices/edges
    typedef std::pair<std::string, std::string> nameDefaultPair;
        
    std::unordered_map<std::string, nameDefaultPair> vertexKeyMap;
    std::unordered_map<std::string, nameDefaultPair> edgeKeyMap;
    const auto keys = graphml.children("key");
    for (const auto &key : keys) {
        pugi::xml_attribute id = key.attribute("id");
        if (!id) { throw std::runtime_error("Failed to fetch id of key"); }
        pugi::xml_attribute name = key.attribute("attr.name");
        if (!name) { throw std::runtime_error("Failed to fetch attr.name of key"); }
        pugi::xml_attribute attrType = key.attribute("for");
        if (!attrType) { throw std::runtime_error("Failed to fetch which primitive key is for"); }
        
        const std::string defaultValue = key.child_value("default");
        
        const std::string attrTypeStr (attrType.value());
        if (attrTypeStr.compare("node") == 0) {
            vertexKeyMap[id.value()] = std::make_pair(name.value(), defaultValue);
        } else if (attrTypeStr.compare("edge") == 0) {
            edgeKeyMap[id.value()] = std::make_pair(name.value(), defaultValue);
        } else {
            std::cerr << "Warning: key specified for neither node "
                        << "nor edge. Ignoring...\n";
        }
    }
    
    // Note: Assumes there is only one graph. Ignores any potential graphs
    //       defined later.
    pugi::xml_node graph = graphml.child("graph");
    if (!graph) {
        throw std::runtime_error("Missing 'graph' attribute");
    }
    
    // Determine type of graph
    pugi::xml_attribute edgeType = graph.attribute("edgedefault");
    if (!edgeType) { throw std::runtime_error("Missing edge type (default)"); }
    const std::string edgeTypeStr (edgeType.value());
    if (edgeTypeStr == "undirected") {
        graphType = GRAPH_UNDIRECTED;
    } else if (edgeTypeStr == "directed") {
        graphType = GRAPH_DIRECTED;
    } else {
        throw std::runtime_error("Unrecognized edge type. So far "
            "only 'undirected' or 'directed' are supported");
    }
    
    // Note: Typically nodes have ids 'nx' where x is the node index,
    //       however this does not always have to be the case
    // Process graph nodes (vertices)
    std::unordered_map<std::string, uint32_t> vertexIndexMap;
    
    nVertices = 0;
    const auto nodes = graph.children("node");
    for (const auto &node : nodes) {
        pugi::xml_attribute id = node.attribute("id");
        if (!id) { throw std::runtime_error("Failed to fetch id of node"); }
        if (vertexIndexMap.contains(id.value())) {
            std::cerr << "Warning: Duplicate vertex id = " << id.value()
                        << " encountered. Ignored...\n";
        } else {
            // Insert vertex id into map
            vertexIndexMap[id.value()] = nVertices;
            // Process vertex attributes
            Vertex vert{};
            
            // TODO: Move to function.
            // TODO: Better method: Iterate first over entries and
            //       set vertex properties accordingly, while removing
            //       corresponding element in vertexKeyMap. Finally, iterate
            //       over remaining elements in vertexKeyMap and set
            //       associated properties to default value.
            const auto data = node.children("data");
            for (const auto &elem : vertexKeyMap) {
                const std::string &key = elem.first;
                const nameDefaultPair &nameDefault = elem.second;
                
                // Search <data/> nodes for matching key
                bool isKeyFound = false;
                for (const auto &entry : data) {
                    pugi::xml_attribute keyAttr = entry.attribute("key");
                    if (keyAttr && key == keyAttr.value()) {
                        isKeyFound = true;
                        
                        const std::string entryStr(entry.text().get());
                        setVertexFromEntry(vert, nameDefault.first, entryStr);
                        break;  // move on to next elem
                    }
                }
                
                if (!isKeyFound) {
                    // Use default value instead of (missing) entry
                    setVertexFromEntry(vert, nameDefault.first, nameDefault.second);
                }
            }
            
            vertices.push_back(vert);
            // Increment vertex count
            ++nVertices;
        }
    }
    // De-allocate unnecessary capacity
    vertices.shrink_to_fit();
    targetVertices.shrink_to_fit();
    stationVertices.shrink_to_fit();
    // Initialize graph query buffers
    query = GraphQuery(nVertices);
    
    // Process graph edges
    std::vector<uint32_t> counts(nVertices, 0);
    typedef std::tuple<uint32_t,uint32_t,uint8_t> triplet;
    std::vector<triplet> edgeList;
    
    {
        nEdges = 0;
        const auto edges = graph.children("edge");
        for (const auto &edge : edges) {
            pugi::xml_attribute source = edge.attribute("source");
            if (!source) { throw std::runtime_error("Missing source vertex for edge"); }
            pugi::xml_attribute target = edge.attribute("target");
            if (!target) { throw std::runtime_error("Missing target vertex for edge"); }
                
            const auto sourceIt = vertexIndexMap.find(source.value());
            if (sourceIt == vertexIndexMap.end()) { throw std::runtime_error("Invalid source vertex id for edge"); }
            const auto targetIt = vertexIndexMap.find(target.value());
            if (sourceIt == vertexIndexMap.end()) { throw std::runtime_error("Invalid target vertex id for edge"); }
            
            const uint32_t sourceIndex = sourceIt->second;
            const uint32_t targetIndex = targetIt->second;
            
            uint8_t isBoegOnly = 0;
            const auto data = edge.children("data");
            for (const auto &elem : edgeKeyMap) {
                const std::string &key = elem.first;
                const nameDefaultPair &nameDefault = elem.second;
                
                bool isKeyFound = false;
                for (const auto &entry : data) {
                    pugi::xml_attribute keyAttr = entry.attribute("key");
                    if (keyAttr && key == keyAttr.value()) {
                        const std::string entryStr (entry.text().get());
                        
                        if (nameDefault.first == "boegEdge" && entryStr == "true") {
                            isKeyFound = true;
                            isBoegOnly = 1;
                            break;
                        }
                    }
                }
                
                if (!isKeyFound) {
                    // Use default value instead of (missing) entry
                    if (nameDefault.second == "true") isBoegOnly = 1;
                }
            }
            
            // Count outdegree of source vertex
            counts[sourceIndex] += 1;
            // Put edge in edgeList for later use
            edgeList.emplace_back(sourceIndex, targetIndex, isBoegOnly);
            
            if (graphType == GRAPH_UNDIRECTED) {
                // Also add edge going in opposite direction
                counts[targetIndex] += 1;
                edgeList.emplace_back(targetIndex, sourceIndex, isBoegOnly);
            }
        }
    }
    
    offsets.resize(nVertices + 1);
    offsets[0] = 0;  // starting offset
    
    std::inclusive_scan(counts.begin(), counts.end(), offsets.begin() + 1);
    // Reset counts for placement of target vertices in bucket associated
    // with source vertex
    std::fill(counts.begin(), counts.end(), 0);
    
    // Fill edge array
    nEdges = static_cast<uint32_t>(edgeList.size());
    edges.resize(nEdges);
    for (const auto &edge : edgeList) {
        uint32_t sourceIndex, targetIndex;
        uint8_t isBoegOnly;
        std::tie(sourceIndex, targetIndex, isBoegOnly) = edge;
        // Store index of target vertex in bucket of source vertex
        const uint32_t edgeIndex = offsets[sourceIndex] + counts[sourceIndex]++;
        edges[edgeIndex] = Edge(targetIndex, isBoegOnly);
    }
}

void Graph::setVertexFromEntry(Vertex &vert, const std::string &name, 
    const std::string &value)
{
    if (name == "location") {
        vert.location = value;
    } else if (name == "xpos") {
        vert.xpos = std::stof(value);
    } else if (name == "ypos") {
        vert.ypos = std::stof(value);
    } else if (name == "targetLocation") {
        if (value == "true") {
            // Add vertex index to set of target vertices
            targetVertices.push_back(nVertices);
        } else {
            // Add vertex to set of (non-target) station vertices
            stationVertices.push_back(nVertices);
        }
    } else {
        throw std::runtime_error("Unrecognized attribute name: " + name);
    }
}

// TODO: Decide if this amount of abstraction is really necessary...
GraphQuery Graph::initializeQuery() const
{
    return GraphQuery(nVertices);
}

// Visits all vertices starting from source using shortest paths
// Breadth-first search shortest path (SP) algorithm
// Note: Distance of 0 for vertex != source indicates vertex is unreachable
void Graph::shortestPaths(const uint32_t source, GraphQuery &spQuery,
    const bool isBoeg /* = false */) const
{
    if (source >= nVertices)
        throw std::invalid_argument("Invalid source vertex");
    
    // Reset query structure
    spQuery.reset();
        
    // Add source to current search list --> FIFO
    std::list<uint32_t> searchList(1, source);
    spQuery.visited[source] = 1;  // source has been visited already
    
    while (!searchList.empty()) {
        const uint32_t v = searchList.front();
        searchList.pop_front();
        
        // Iterate over all adjacent vertices
        const auto [start, end] = vertexBounds(v);
        for (uint32_t i = start; i < end; ++i) {
            const Edge edge = edges[i];
            const uint32_t n = edge.nborId;
            // Check if this edge is accessible 
            const bool isEdgeAccessible = isBoeg || !edge.isBoegOnly;
            // Make sure that neighbor has not been visited yet
            if (isEdgeAccessible && !spQuery.visited[n]) {
                // Visit neighboring vertex
                spQuery.visited[n] = 1;
                // Update distance and child vertex for v
                spQuery.distances[n] = spQuery.distances[v] + 1;
                // Note: Have to store children in REVERSE order,
                //       otherwise last write wins
                spQuery.children[n] = v;
                // Add neighbor to search list
                searchList.push_back(n);
            }
        }
    }
}

bool Graph::findPathOfLengthRecursive(const uint32_t v, 
    const uint32_t target, const uint32_t pathLength, 
    const bool isBoeg /* = false */)
{
    const uint32_t distance = query.distances[v];
    if (distance == pathLength && v == target) {
        return true;  // path of required length to target found
    } else if ((distance == pathLength && v != target) ||
               (distance < pathLength && v == target)) {
        // Either have already travelled past length limit, or found target
        // vertex too early (cannot possibly result in a simple path)
        return false;  // backtrack
    }
    
    // Visit this vertex
    query.visited[v] = 1;
    // Check all neighboring vertices
    const auto [start, end] = vertexBounds(v);
    for (uint32_t i = start; i < end; ++i) {
        const Edge edge = edges[i];
        const uint32_t n = edge.nborId;  // neighbor
        
        const bool isEdgeAccessible = isBoeg || !edge.isBoegOnly;
        if (isEdgeAccessible && !query.visited[n]) {
            // Update
            query.distances[n] = distance + 1;
            query.children[v] = n;
            
            // Recursively move to neighbor vertex n
            if (findPathOfLengthRecursive(n, target, pathLength, isBoeg)) {
                return true;  // path has been found --> done
            }
        }
    }
    
    // Backtrack
    query.visited[v] = 0;
    
    return false;
}

void Graph::findAllReachableVerticesRecursive(const uint32_t v, 
    const uint32_t pathLength, const bool isBoeg /* = false */,
    std::unordered_set<uint32_t> &reachable)
{
    const uint32_t distance = query.distances[v];
    if (distance == pathLength) {
        // Found new reachable position
        // Note: No effect if vertex is already contained
        reachable.insert(v);
        return;  // backtrack
    }
    // Note: At this point distance < pathLength
    
    // Visit this vertex
    query.visited[v] = 1;
    // Check all neighboring vertices
    const auto [start, end] = vertexBounds(v);
    for (uint32_t i = start; i < end; ++i) {
        const Edge edge = edges[i];
        const uint32_t n = edge.nborId;
        
        // Check if neighbor has not been previously visited and edge
        // to neighbor is accessible
        const bool isEdgeAccessible = isBoeg || !edge.isBoegOnly;
        if (isEdgeAccessible && !query.visited[n]) {
            // Update
            query.distances[n] = distance + 1;
            // Recursively find all reachable vertices from neighbor n
            findAllReachableVerticesRecursive(n, pathLength, isBoeg, reachable);
        }
    }
    
    // Backtrack
    query.visited[v] = 0;
}

std::vector<uint32_t> Graph::findPathOfLength(const uint32_t source, 
    const uint32_t target, const uint32_t pathLength, 
    const bool isBoeg /* = false */)
{    
    if (source >= nVertices || target >= nVertices)
        throw std::invalid_argument("Invalid source/target vertex indexes");
        
    // Reset query structure
    query.reset();
    
    bool isPathFound = false;
    isPathFound = findPathOfLengthRecursive(source, target, pathLength, isBoeg);
    
    if (!isPathFound) {
        throw std::runtime_error(
            "Failed to find path from " + std::to_string(source) + 
            " to " + std::to_string(target) + 
            " of length " + std::to_string(pathLength)
        );
    }
    
    // Prepare final output
    std::vector<uint32_t> path(pathLength + 1);
    for (uint32_t v = source, i = 0; i <= pathLength; v = query.children[v], ++i) {
        path[i] = v;
    }
    
    return path;
}

std::unordered_set<uint32_t> Graph::findAllReachableVertices(
    const uint32_t source, const uint32_t pathLength, 
    const bool isBoeg /* = false */)
{
    if (source >= nVertices)
        throw std::invalid_argument("Invalid source vertex index");
        
    // Reset query structure
    query.reset();
    
    std::unordered_set<uint32_t> reachable;
    findAllReachableVerticesRecursive(source, pathLength, isBoeg, reachable);
    
    return reachable;
}

bool Graph::isValidPath(const std::vector<uint32_t> &path, 
    const uint32_t source, const uint32_t target, const uint32_t pathLength,
    const bool isBoeg)
{
    if (path.size() != pathLength + 1) return false;      // empty
    if (path.size() == 1) return path.front() == source;  // trivial path
    if (path.front() != source || path.back() != target) return false;
    
    // Reset query structure
    query.reset();
    
    for (auto u = path.begin(), v = path.begin() + 1;
            v < path.end(); ++u, ++v)
    {
        // Visit this vertex
        query.visited[*u] = 1;
        
        bool foundNeighbor = false;
        const auto [start, end] = vertexBounds(*u);
        for (uint32_t i = start; i < end; ++i) {
            const Edge e = edges[i];
            const uint32_t n = e.nborId;
            
            const bool isEdgeAccessible = isBoeg || !e.isBoegOnly;
            if (isEdgeAccessible && n == *v && !query.visited[n]) {
                foundNeighbor = true;
                break;
            }
        }
        
        if (!foundNeighbor) return false;
    }
    
    return true;
}

std::pair<uint32_t,uint32_t> Graph::vertexBounds(const uint32_t v) const
{
    assert(v < nVertices && "Invalid vertex index");
    
    return std::pair(offsets[v], offsets[v + 1]);
}
