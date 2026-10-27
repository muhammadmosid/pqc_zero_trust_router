#include "ZeroTrustRouter.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <algorithm>
#include <queue>
#include <set>
#include <fstream>
#include <sstream>

namespace Routing {

ZeroTrustRouter::ZeroTrustRouter(int n, int betaFactor)
    : numNodes(n), trustWeightFactor(betaFactor) {
    nodes.resize(n);
}

void ZeroTrustRouter::setNode(int id, const std::string& name, int trustScore, bool isIsolated) {
    if (id >= 0 && id < numNodes) {
        nodes[id] = {id, name, trustScore, isIsolated};
    }
}

void ZeroTrustRouter::addEdge(int id, int u, int v, int latency, bool compromised, int maxCap) {
    edges.push_back({id, u, v, latency, compromised, maxCap});
}

void ZeroTrustRouter::setNodeIsolated(int id, bool status) {
    if (id >= 0 && id < numNodes) {
        nodes[id].isolated = status;
    }
}

void ZeroTrustRouter::setEdgeCompromised(int edgeId, bool status) {
    for (auto& e : edges) {
        if (e.id == edgeId) {
            e.compromised = status;
        }
    }
}

bool ZeroTrustRouter::isAncestor(int candidateParent, int targetNode, const std::vector<int>& parent) const {
    int curr = candidateParent;
    int steps = 0;
    while (curr != -1 && steps <= numNodes) {
        if (curr == targetNode) return true;
        curr = parent[curr];
        steps++;
    }
    return false;
}

long long ZeroTrustRouter::computeCompositeWeight(const Edge& e) const {
    return static_cast<long long>(e.latencyCost) -
           static_cast<long long>(trustWeightFactor * nodes[e.v].securityTrustScore);
}

RoutingResult ZeroTrustRouter::runBellmanFordDP(int source, bool verboseTable) const {
    auto startClock = std::chrono::high_resolution_clock::now();
    const long long INF = std::numeric_limits<long long>::max() / 4;

    RoutingResult res;
    res.source = source;
    res.dist.assign(numNodes, INF);
    res.parent.assign(numNodes, -1);
    res.accumTrust.assign(numNodes, 0);
    res.hops.assign(numNodes, 0);

    if (source < 0 || source >= numNodes || nodes[source].isolated) {
        return res;
    }

    res.dist[source] = 0;
    res.accumTrust[source] = nodes[source].securityTrustScore;
    res.hops[source] = 0;

    std::vector<Edge> activeEdges;
    for (const auto& e : edges) {
        if (e.compromised) {
            res.excludedCompromisedEdges++;
            continue;
        }
        if (e.latencyCost > e.maxLatencyCapacity) {
            res.excludedOverCapacityEdges++;
            continue;
        }
        if (nodes[e.u].isolated || nodes[e.v].isolated) {
            continue;
        }
        activeEdges.push_back(e);
    }

    if (verboseTable) {
        std::cout << "\n  [DP Table Trace - Composite Cost Evolution Across Iterations]\n  Iter | ";
        for (int v = 0; v < numNodes; ++v) {
            std::cout << std::setw(6) << ("N" + std::to_string(v)) << " ";
        }
        std::cout << "\n  -----+" << std::string(numNodes * 7, '-') << "\n";
    }

    for (int iter = 1; iter <= numNodes - 1; ++iter) {
        bool changed = false;
        for (const auto& e : activeEdges) {
            res.relaxationOpsCount++;
            if (res.dist[e.u] == INF) continue;

            long long w = computeCompositeWeight(e);
            long long candCost = res.dist[e.u] + w;
            int candTrust = res.accumTrust[e.u] + nodes[e.v].securityTrustScore;
            int candHops  = res.hops[e.u] + 1;

            if (candCost < res.dist[e.v]) {
                res.dist[e.v]       = candCost;
                res.parent[e.v]     = e.u;
                res.accumTrust[e.v] = candTrust;
                res.hops[e.v]       = candHops;
                changed = true;
            } else if (candCost == res.dist[e.v] && res.parent[e.v] != e.u &&
                       !isAncestor(e.u, e.v, res.parent)) {
                if (candTrust > res.accumTrust[e.v] ||
                   (candTrust == res.accumTrust[e.v] && candHops < res.hops[e.v])) {
                    res.tieBreakLogs.push_back({
                        e.v, res.parent[e.v], e.u, candCost,
                        res.accumTrust[e.v], candTrust, res.hops[e.v], candHops
                    });
                    res.parent[e.v]     = e.u;
                    res.accumTrust[e.v] = candTrust;
                    res.hops[e.v]       = candHops;
                    res.tieBreakEvents++;
                    changed = true;
                }
            }
        }

        if (verboseTable && (iter <= 6 || !changed)) {
            std::cout << "  " << std::setw(4) << iter << " | ";
            for (int v = 0; v < numNodes; ++v) {
                if (res.dist[v] == INF) std::cout << std::setw(6) << "INF" << " ";
                else std::cout << std::setw(6) << res.dist[v] << " ";
            }
            std::cout << "\n";
        }
        if (!changed) break;
    }

    // V-th Iteration Check for Negative-Weight Cycles
    int cycleEntry = -1;
    for (const auto& e : activeEdges) {
        if (res.dist[e.u] == INF) continue;
        long long w = computeCompositeWeight(e);
        if (res.dist[e.u] + w < res.dist[e.v]) {
            res.negativeCycleDetected = true;
            res.parent[e.v] = e.u;
            cycleEntry = e.v;
            break;
        }
    }

    if (res.negativeCycleDetected && cycleEntry != -1) {
        int curr = cycleEntry;
        for (int i = 0; i < numNodes; ++i) {
            curr = res.parent[curr];
        }
        std::vector<int> cycle;
        int startNode = curr;
        do {
            cycle.push_back(curr);
            curr = res.parent[curr];
        } while (curr != startNode && curr != -1 &&
                 cycle.size() <= static_cast<std::size_t>(numNodes));
        std::reverse(cycle.begin(), cycle.end());
        res.cycleNodes = cycle;
    }

    for (int v = 0; v < numNodes; ++v) {
        if (!nodes[v].isolated && res.dist[v] == INF) {
            res.unreachableNodes.push_back(v);
        }
    }

    auto endClock = std::chrono::high_resolution_clock::now();
    res.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(
        endClock - startClock).count();
    return res;
}

RoutingResult ZeroTrustRouter::solveWithAutoCycleMitigation(
    int source, int destination, std::vector<int>& outQuarantinedNodes, bool verbose) {
    outQuarantinedNodes.clear();
    int safetyPasses = 0;
    RoutingResult res = runBellmanFordDP(source, verbose);

    while (res.negativeCycleDetected && safetyPasses < numNodes) {
        for (int u : res.cycleNodes) {
            if (u != source && u != destination && !nodes[u].isolated) {
                nodes[u].isolated = true;
                outQuarantinedNodes.push_back(u);
            }
        }
        res = runBellmanFordDP(source, false);
        safetyPasses++;
    }
    return res;
}

RoutingResult ZeroTrustRouter::runIncrementalSubtreeRepair(
    const RoutingResult& previousResult,
    const std::vector<int>& failedEdgeIds,
    std::vector<int>& outAffectedNodes) {
    auto startClock = std::chrono::high_resolution_clock::now();
    const long long INF = std::numeric_limits<long long>::max() / 4;

    for (int id : failedEdgeIds) {
        setEdgeCompromised(id, true);
    }

    // Build shortest-path tree adjacency list from previous DP parent pointers
    std::vector<std::vector<int>> spTreeChildren(numNodes);
    for (int v = 0; v < numNodes; ++v) {
        int p = previousResult.parent[v];
        if (p != -1) {
            spTreeChildren[p].push_back(v);
        }
    }

    // Find roots of subtrees severed by the newly failed links
    std::queue<int> q;
    std::set<int> affectedSet;
    for (const auto& e : edges) {
        if (std::find(failedEdgeIds.begin(), failedEdgeIds.end(), e.id) != failedEdgeIds.end()) {
            if (previousResult.parent[e.v] == e.u && affectedSet.find(e.v) == affectedSet.end()) {
                affectedSet.insert(e.v);
                q.push(e.v);
            }
        }
    }

    // BFS down the shortest-path tree to collect all downstream affected nodes
    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        for (int child : spTreeChildren[curr]) {
            if (affectedSet.find(child) == affectedSet.end()) {
                affectedSet.insert(child);
                q.push(child);
            }
        }
    }

    outAffectedNodes.assign(affectedSet.begin(), affectedSet.end());

    // Initialize incremental result from previous valid state
    RoutingResult incRes = previousResult;
    incRes.relaxationOpsCount = 0;
    incRes.tieBreakEvents = 0;
    incRes.tieBreakLogs.clear();
    incRes.unreachableNodes.clear();

    // Reset only the affected subtree nodes
    for (int v : outAffectedNodes) {
        incRes.dist[v]       = INF;
        incRes.parent[v]     = -1;
        incRes.accumTrust[v] = 0;
        incRes.hops[v]       = 0;
    }

    // Collect only active edges Whose target v lies in the affected subtree
    std::vector<Edge> candidateRepairEdges;
    for (const auto& e : edges) {
        if (e.compromised || e.latencyCost > e.maxLatencyCapacity ||
            nodes[e.u].isolated || nodes[e.v].isolated) {
            continue;
        }
        if (affectedSet.find(e.v) != affectedSet.end()) {
            candidateRepairEdges.push_back(e);
        }
    }

    // Relax only candidate repair edges across at most |V_affected| iterations
    int maxIters = static_cast<int>(outAffectedNodes.size());
    for (int iter = 1; iter <= maxIters; ++iter) {
        bool changed = false;
        for (const auto& e : candidateRepairEdges) {
            incRes.relaxationOpsCount++;
            if (incRes.dist[e.u] == INF) continue;

            long long w = computeCompositeWeight(e);
            long long candCost = incRes.dist[e.u] + w;
            int candTrust = incRes.accumTrust[e.u] + nodes[e.v].securityTrustScore;
            int candHops  = incRes.hops[e.u] + 1;

            if (candCost < incRes.dist[e.v]) {
                incRes.dist[e.v]       = candCost;
                incRes.parent[e.v]     = e.u;
                incRes.accumTrust[e.v] = candTrust;
                incRes.hops[e.v]       = candHops;
                changed = true;
            } else if (candCost == incRes.dist[e.v] && incRes.parent[e.v] != e.u &&
                       !isAncestor(e.u, e.v, incRes.parent)) {
                if (candTrust > incRes.accumTrust[e.v] ||
                   (candTrust == incRes.accumTrust[e.v] && candHops < incRes.hops[e.v])) {
                    incRes.parent[e.v]     = e.u;
                    incRes.accumTrust[e.v] = candTrust;
                    incRes.hops[e.v]       = candHops;
                    incRes.tieBreakEvents++;
                    changed = true;
                }
            }
        }
        if (!changed) break;
    }

    for (int v = 0; v < numNodes; ++v) {
        if (!nodes[v].isolated && incRes.dist[v] == INF) {
            incRes.unreachableNodes.push_back(v);
        }
    }

    auto endClock = std::chrono::high_resolution_clock::now();
    incRes.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(
        endClock - startClock).count();
    return incRes;
}

std::vector<int> ZeroTrustRouter::reconstructPath(int target, const RoutingResult& res) const {
    const long long INF = std::numeric_limits<long long>::max() / 4;
    if (target < 0 || target >= numNodes || res.dist[target] == INF || res.negativeCycleDetected) {
        return {};
    }
    std::vector<int> path;
    int curr = target;
    int guard = 0;
    while (curr != -1 && guard <= numNodes) {
        path.push_back(curr);
        if (curr == res.source) break;
        curr = res.parent[curr];
        guard++;
    }
    if (path.empty() || path.back() != res.source) return {};
    std::reverse(path.begin(), path.end());
    return path;
}

void ZeroTrustRouter::printPathAssignment(int target, const RoutingResult& res) const {
    std::vector<int> path = reconstructPath(target, res);
    if (path.empty()) {
        std::cout << "  -> Target Node [" << target << ": " << nodes[target].name
                  << "] is UNREACHABLE (Disconnected Component).\n";
        return;
    }
    std::cout << "  -> Optimal Zero-Trust Route to [" << target << ": " << nodes[target].name << "]:\n     ";
    for (std::size_t i = 0; i < path.size(); ++i) {
        int u = path[i];
        std::cout << "[" << u << ":" << nodes[u].name << "(T=" << nodes[u].securityTrustScore << ")]";
        if (i + 1 < path.size()) std::cout << " ==> ";
    }
    std::cout << "\n     Net Composite Cost: " << res.dist[target]
              << " | Cumulative Trust Score: " << res.accumTrust[target]
              << " | Total Hops: " << res.hops[target] << "\n";
}

bool ZeroTrustRouter::exportToCSV(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    out << "# NODES: id,name,trustScore,isolated\n";
    for (const auto& n : nodes) {
        out << "NODE," << n.id << "," << n.name << "," << n.securityTrustScore
            << "," << (n.isolated ? 1 : 0) << "\n";
    }
    out << "# EDGES: id,u,v,latencyCost,compromised,maxLatencyCapacity\n";
    for (const auto& e : edges) {
        out << "EDGE," << e.id << "," << e.u << "," << e.v << "," << e.latencyCost
            << "," << (e.compromised ? 1 : 0) << "," << e.maxLatencyCapacity << "\n";
    }
    return true;
}

ZeroTrustRouter ZeroTrustRouter::importFromCSV(const std::string& filename, int betaFactor) {
    std::ifstream in(filename);
    std::vector<Node> loadedNodes;
    std::vector<Edge> loadedEdges;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string type;
        std::getline(ss, type, ',');
        if (type == "NODE") {
            std::string idS, name, trustS, isoS;
            std::getline(ss, idS, ',');
            std::getline(ss, name, ',');
            std::getline(ss, trustS, ',');
            std::getline(ss, isoS, ',');
            loadedNodes.push_back({std::stoi(idS), name, std::stoi(trustS), std::stoi(isoS) != 0});
        } else if (type == "EDGE") {
            std::string idS, uS, vS, latS, compS, capS;
            std::getline(ss, idS, ',');
            std::getline(ss, uS, ',');
            std::getline(ss, vS, ',');
            std::getline(ss, latS, ',');
            std::getline(ss, compS, ',');
            std::getline(ss, capS, ',');
            loadedEdges.push_back({
                std::stoi(idS), std::stoi(uS), std::stoi(vS),
                std::stoi(latS), std::stoi(compS) != 0, std::stoi(capS)
            });
        }
    }

    ZeroTrustRouter net(static_cast<int>(loadedNodes.size()), betaFactor);
    for (const auto& n : loadedNodes) {
        net.setNode(n.id, n.name, n.securityTrustScore, n.isolated);
    }
    for (const auto& e : loadedEdges) {
        net.addEdge(e.id, e.u, e.v, e.latencyCost, e.compromised, e.maxLatencyCapacity);
    }
    return net;
}

} // namespace Routing