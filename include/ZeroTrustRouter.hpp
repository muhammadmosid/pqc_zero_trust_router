#ifndef ZERO_TRUST_ROUTER_HPP
#define ZERO_TRUST_ROUTER_HPP

#include <vector>
#include <string>

namespace Routing {

struct Node {
    int id = 0;
    std::string name;
    int securityTrustScore = 1; // 1..10
    bool isolated = false;
};

struct Edge {
    int id = 0;
    int u = 0;
    int v = 0;
    int latencyCost = 0;
    bool compromised = false;
    int maxLatencyCapacity = 100;
};

struct TieBreakRecord {
    int targetNode;
    int oldParent;
    int newParent;
    long long tiedCompositeCost;
    int oldCumTrust;
    int newCumTrust;
    int oldHops;
    int newHops;
};

struct RoutingResult {
    int source = 0;
    std::vector<long long> dist;
    std::vector<int> parent;
    std::vector<int> accumTrust;
    std::vector<int> hops;
    bool negativeCycleDetected = false;
    std::vector<int> cycleNodes;
    std::vector<int> unreachableNodes;
    int excludedCompromisedEdges = 0;
    int excludedOverCapacityEdges = 0;
    int tieBreakEvents = 0;
    std::vector<TieBreakRecord> tieBreakLogs;
    long long relaxationOpsCount = 0;
    long long executionTimeUs = 0;
};

class ZeroTrustRouter {
private:
    int numNodes;
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    int trustWeightFactor; // Beta multiplier

    bool isAncestor(int candidateParent, int targetNode, const std::vector<int>& parent) const;

public:
    explicit ZeroTrustRouter(int n = 0, int betaFactor = 2);

    void setNode(int id, const std::string& name, int trustScore, bool isIsolated = false);
    void addEdge(int id, int u, int v, int latency, bool compromised = false, int maxCap = 100);
    void setNodeIsolated(int id, bool status);
    void setEdgeCompromised(int edgeId, bool status);

    int getNumNodes() const { return numNodes; }
    const std::vector<Node>& getNodes() const { return nodes; }
    const std::vector<Edge>& getEdges() const { return edges; }

    long long computeCompositeWeight(const Edge& e) const;

    // Core Bellman-Ford DP Solver
    RoutingResult runBellmanFordDP(int source, bool verboseTable = false) const;

    // Automatic Negative-Cycle Quarantine & Safe Route Recalculation
    RoutingResult solveWithAutoCycleMitigation(
        int source, int destination, std::vector<int>& outQuarantinedNodes, bool verbose = false);

    // True Incremental Subtree DP Update after dynamic link failures (Requirement A.6 / Part C)
    RoutingResult runIncrementalSubtreeRepair(
        const RoutingResult& previousResult,
        const std::vector<int>& failedEdgeIds,
        std::vector<int>& outAffectedNodes);

    std::vector<int> reconstructPath(int target, const RoutingResult& res) const;
    void printPathAssignment(int target, const RoutingResult& res) const;

    // CSV Dataset Export & Import for Instructor Grading
    bool exportToCSV(const std::string& filename) const;
    static ZeroTrustRouter importFromCSV(const std::string& filename, int betaFactor = 2);
};

} // namespace Routing

#endif // ZERO_TRUST_ROUTER_HPP