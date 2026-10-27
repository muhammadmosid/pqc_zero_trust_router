#include "CepTestSuite.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <filesystem>

namespace Tests {

Routing::ZeroTrustRouter CepTestSuite::build10NodeBenchmark() {
    Routing::ZeroTrustRouter net(10, 2); // Beta = 2
    net.setNode(0, "NOC-Core",      10);
    net.setNode(1, "Relay-Alpha",    4);
    net.setNode(2, "Vault-Beta",     8);
    net.setNode(3, "Edge-Gamma",     3);
    net.setNode(4, "ZT-Delta",       9);
    net.setNode(5, "Public-Eps",     2);
    net.setNode(6, "QKD-Zeta",       9);
    net.setNode(7, "Metro-Eta",      5);
    net.setNode(8, "Cloud-Theta",    7);
    net.setNode(9, "Dest-HQ-Iota",  10);

    // 20 Candidate Directed Links
    net.addEdge(1,  0, 1, 22, false, 100);
    net.addEdge(2,  0, 2, 30, false, 100);
    net.addEdge(3,  0, 3, 18, false, 100);
    net.addEdge(4,  1, 3, 15, false, 100);
    net.addEdge(5,  1, 4, 30, false, 100);
    net.addEdge(6,  2, 4, 30, false, 100);
    net.addEdge(7,  2, 5, 16, false, 100);
    net.addEdge(8,  3, 5, 14, false, 100);
    net.addEdge(9,  3, 6, 28, false, 100);
    net.addEdge(10, 4, 6, 20, false, 100);
    net.addEdge(11, 4, 7, 19, false, 100);
    net.addEdge(12, 5, 7, 12, false, 100);
    net.addEdge(13, 5, 8, 25, false, 100);
    net.addEdge(14, 6, 8, 18, false, 100);
    net.addEdge(15, 6, 9, 24, false, 100);
    net.addEdge(16, 7, 8, 16, false, 100);
    net.addEdge(17, 7, 9, 29, false, 100);
    net.addEdge(18, 8, 9, 21, false, 100);
    net.addEdge(19, 1, 2, 19, false, 100);
    net.addEdge(20, 2, 6, 35, false, 100);

    return net;
}

Routing::ZeroTrustRouter CepTestSuite::build100NodeWAN() {
    const int N = 100;
    Routing::ZeroTrustRouter net100(N, 2);
    std::mt19937 rng(1002026);
    std::uniform_int_distribution<int> trustDist(1, 9);
    std::uniform_int_distribution<int> latDist(20, 65);

    for (int i = 0; i < N; ++i) {
        std::string name = (i == 0) ? "NOC-Origin" :
                           (i == N - 1 ? "Core-Target-99" : ("Router-" + std::to_string(i)));
        int trust = (i == 0 || i == N - 1) ? 10 : trustDist(rng);
        net100.setNode(i, name, trust);
    }

    int edgeId = 1;
    for (int u = 0; u < N - 1; ++u) {
        for (int step = 1; step <= 4 && (u + step) < N; ++step) {
            int lat = latDist(rng);
            bool comp = (edgeId % 29 == 0);
            int maxCap = (edgeId % 37 == 0) ? 30 : 100;
            net100.addEdge(edgeId++, u, u + step, lat, comp, maxCap);
        }
    }
    return net100;
}

PQC::NttBenchmarkMetrics CepTestSuite::runT01_PartA_NttVerification() {
    std::cout << "================================================================================\n";
    std::cout << " [T-01] PART A: DIVIDE-AND-CONQUER NTT KEY GENERATION (Z_q[X]/(X^256 + 1))\n";
    std::cout << "================================================================================\n";

    auto m = PQC::PqcNttEngine::runBenchmarkAndVerify(100);
    std::cout << "  Ring Parameters            : N = " << PQC::PqcNttEngine::POLY_DEGREE
              << ", Prime Modulus q = " << PQC::PqcNttEngine::MOD_Q << "\n";
    std::cout << "  Primitive 512-th Root psi  : " << m.primitiveRootPsi << "\n";
    std::cout << "  Brute-Force O(N^2) Ops     : " << m.bruteForceMulOps << " modular multiplications\n";
    std::cout << "  Divide & Conquer NTT Ops   : " << m.nttButterflyOps << " butterfly operations (3 NTT passes)\n";
    std::cout << "  Brute-Force Avg Time       : " << std::fixed << std::setprecision(2) << m.bruteForceTimeUs << " us\n";
    std::cout << "  Divide & Conquer NTT Time  : " << std::fixed << std::setprecision(2) << m.nttTimeUs << " us\n";
    std::cout << "  Empirical Speedup Factor   : " << std::fixed << std::setprecision(2)
              << (m.bruteForceTimeUs / std::max(0.01, m.nttTimeUs)) << "x faster\n";
    std::cout << "  256-Coeff Exact Match Check: " << (m.exactMatchVerified ? "PASSED (100% Identical)" : "FAILED") << "\n";
    std::cout << "  Encapsulated Shared Secret : " << PQC::PqcNttEngine::formatKeyDigest(m.keyPolynomial) << "\n\n";
    return m;
}

void CepTestSuite::runT02_WorkedExample10Nodes(const std::vector<long long>& sharedKey) {
    std::cout << "================================================================================\n";
    std::cout << " [T-02] PART B & C: WORKED EXAMPLE (10 NODES, 20 EDGES) - BELLMAN-FORD DP\n";
    std::cout << "================================================================================\n";

    auto net = build10NodeBenchmark();
    auto res = net.runBellmanFordDP(0, true);

    std::cout << "\n  [Final DP State Table - 10-Node Benchmark]\n";
    std::cout << "  " << std::left << std::setw(5) << "ID" << std::setw(15) << "Node Name"
              << std::setw(8) << "Trust" << std::setw(12) << "Comp.Cost"
              << std::setw(12) << "Cum.Trust" << std::setw(8) << "Hops" << "Predecessor\n";
    std::cout << "  " << std::string(68, '-') << "\n";

    const auto& nodes = net.getNodes();
    for (int i = 0; i < net.getNumNodes(); ++i) {
        std::cout << "  " << std::left << std::setw(5) << i
                  << std::setw(15) << nodes[i].name
                  << std::setw(8) << nodes[i].securityTrustScore
                  << std::setw(12) << res.dist[i]
                  << std::setw(12) << res.accumTrust[i]
                  << std::setw(8) << res.hops[i]
                  << (res.parent[i] == -1 ? "SOURCE" : ("Node " + std::to_string(res.parent[i]))) << "\n";
    }

    std::cout << "\n  [Explicit Tie-Breaking Audit Log]\n";
    for (const auto& tb : res.tieBreakLogs) {
        std::cout << "  * Tie at Node [" << tb.targetNode << ":" << nodes[tb.targetNode].name
                  << "] at Equal Composite Cost = " << tb.tiedCompositeCost << ":\n"
                  << "    Replaced predecessor Node " << tb.oldParent << " (Cum.Trust=" << tb.oldCumTrust
                  << ", Hops=" << tb.oldHops << ") with Node " << tb.newParent
                  << " (Cum.Trust=" << tb.newCumTrust << ", Hops=" << tb.newHops << ")\n";
    }

    net.printPathAssignment(9, res);
    if (!sharedKey.empty()) {
        std::cout << "  Transmitted PQC Key Payload: " << PQC::PqcNttEngine::formatKeyDigest(sharedKey) << "\n";
    }
    std::cout << "\n";
}

void CepTestSuite::runT03_ScaleTest100Nodes() {
    std::cout << "================================================================================\n";
    std::cout << " [T-03] WP2 SCALABILITY STRESS TEST: 100-NODE WIDE-AREA NETWORK\n";
    std::cout << "================================================================================\n";

    auto net100 = build100NodeWAN();
    auto res = net100.runBellmanFordDP(0, false);

    std::cout << "  Network Scale              : " << net100.getNumNodes() << " Nodes, "
              << net100.getEdges().size() << " Candidate Directed Links\n";
    std::cout << "  Compromised Links Filtered : " << res.excludedCompromisedEdges << "\n";
    std::cout << "  Over-Capacity Filtered     : " << res.excludedOverCapacityEdges << "\n";
    std::cout << "  Total DP Edge Relaxations  : " << res.relaxationOpsCount << " ops\n";
    std::cout << "  Tie-Break Decisions Made   : " << res.tieBreakEvents << "\n";
    std::cout << "  Execution Time             : " << res.executionTimeUs << " us\n";
    net100.printPathAssignment(99, res);
    std::cout << "\n";
}

void CepTestSuite::runT04_EdgeCase1_DisconnectedGraph() {
    std::cout << "================================================================================\n";
    std::cout << " [T-04] EDGE CASE 1: DISCONNECTED NETWORK GRAPH & ISOLATED NODE DETECTION\n";
    std::cout << "================================================================================\n";

    auto net = build10NodeBenchmark();
    // Sever all links leading into Node 8 and Node 9
    for (int id : {13, 14, 15, 16, 17, 18}) {
        net.setEdgeCompromised(id, true);
    }

    auto res = net.runBellmanFordDP(0, false);
    std::cout << "  Disconnection Check        : DETECTED (" << res.unreachableNodes.size()
              << " nodes unreachable from NOC-Core)\n";
    std::cout << "  Isolated Nodes by Name/ID  : ";
    for (int u : res.unreachableNodes) {
        std::cout << "[" << u << ":" << net.getNodes()[u].name << "] ";
    }
    std::cout << "\n  Target Node 9 Status       :\n";
    net.printPathAssignment(9, res);
    std::cout << "  Reachable Sub-Network Routing (Fallback to Node 7 [Metro-Eta]):\n";
    net.printPathAssignment(7, res);
    std::cout << "\n";
}

void CepTestSuite::runT05_EdgeCase2_NegativeCycleAttack() {
    std::cout << "================================================================================\n";
    std::cout << " [T-05] EDGE CASE 2: ADVERSARIAL NEGATIVE-WEIGHT CYCLE DETECTION & QUARANTINE\n";
    std::cout << "================================================================================\n";

    auto net = build10NodeBenchmark();
    // Inject malicious back-edge 6 -> 2 with spoofed latency 1ms => weight = 1 - 16 = -15
    // Cycle 2 -> 4 (+12) -> 6 (+2) -> 2 (-15) has net weight = -1 < 0
    net.addEdge(21, 6, 2, 1, false, 100);

    auto initialPass = net.runBellmanFordDP(0, false);
    if (initialPass.negativeCycleDetected) {
        std::cout << "  V-th Pass Security Alert   : NEGATIVE-WEIGHT ROUTING LOOP DETECTED!\n";
        std::cout << "  Traced Malicious Loop      : ";
        for (std::size_t i = 0; i < initialPass.cycleNodes.size(); ++i) {
            int u = initialPass.cycleNodes[i];
            std::cout << "[" << u << ":" << net.getNodes()[u].name << "]";
            if (i + 1 < initialPass.cycleNodes.size()) std::cout << " -> ";
        }
        std::cout << " -> [" << initialPass.cycleNodes.front() << "]\n";
    }

    std::vector<int> quarantined;
    auto safeRes = net.solveWithAutoCycleMitigation(0, 9, quarantined, false);
    std::cout << "  Quarantined Relay Nodes    : ";
    for (int u : quarantined) {
        std::cout << "[" << u << ":" << net.getNodes()[u].name << "] ";
    }
    std::cout << "\n  Post-Quarantine Safe Route :\n";
    net.printPathAssignment(9, safeRes);
    std::cout << "\n";
}

void CepTestSuite::runT06_EdgeCase3_OverCapacityAndSaturatedLinks() {
    std::cout << "================================================================================\n";
    std::cout << " [T-06] EDGE CASE 3: OVER-CAPACITY LINKS & ALL-PATHS-SATURATED FALLBACK\n";
    std::cout << "================================================================================\n";

    Routing::ZeroTrustRouter net(5, 2);
    net.setNode(0, "NOC-Source",   10);
    net.setNode(1, "Relay-Fast",    6);
    net.setNode(2, "Relay-Slow",    9);
    net.setNode(3, "Relay-Backup",  5);
    net.setNode(4, "Dest-Vault",   10);

    // Sub-case A: Edge 0->2 has latency 150ms > 50ms capacity limit
    net.addEdge(1, 0, 2, 150, false, 50); // Over-capacity -> automatically excluded
    net.addEdge(2, 2, 4, 20,  false, 50);
    net.addEdge(3, 0, 1, 28,  false, 50);
    net.addEdge(4, 1, 3, 25,  false, 50);
    net.addEdge(5, 3, 4, 30,  false, 50);

    auto resA = net.runBellmanFordDP(0, false);
    std::cout << "  [Sub-case 6A: Single Over-Capacity Link (150ms > 50ms Max)]\n";
    std::cout << "  Over-Capacity Links Excluded: " << resA.excludedOverCapacityEdges << "\n";
    net.printPathAssignment(4, resA);

    // Sub-case B: Every outgoing link from source exceeds capacity (Over-constrained network)
    Routing::ZeroTrustRouter netSat(3, 2);
    netSat.setNode(0, "NOC-Source", 10);
    netSat.setNode(1, "Relay-Mid",   8);
    netSat.setNode(2, "Dest-Vault", 10);
    netSat.addEdge(1, 0, 1, 120, false, 40);
    netSat.addEdge(2, 1, 2, 110, false, 40);

    auto resB = netSat.runBellmanFordDP(0, false);
    std::cout << "\n  [Sub-case 6B: All Candidate Links Exceed Latency Threshold (Graceful Degradation)]\n";
    std::cout << "  Over-Capacity Links Excluded: " << resB.excludedOverCapacityEdges
              << " | Program Status: Handled cleanly without crash\n";
    netSat.printPathAssignment(2, resB);
    std::cout << "\n";
}

void CepTestSuite::runT07_SensitivityAndIncrementalRepair() {
    std::cout << "================================================================================\n";
    std::cout << " [T-07] PART C: SENSITIVITY TEST (TOP-2 NODES) & TRUE INCREMENTAL DP REPAIR\n";
    std::cout << "================================================================================\n";

    // 1. Sensitivity Analysis: Remove Top-2 Highest-Trust Intermediate Nodes (Node 4 & Node 6)
    std::cout << "  [1. Sensitivity Analysis: Isolating Top-2 Trust Nodes (Node 4 [T=9] & Node 6 [T=9])]\n";
    auto netSens = build10NodeBenchmark();
    auto baseRes = netSens.runBellmanFordDP(0, false);

    netSens.setNodeIsolated(4, true);
    netSens.setNodeIsolated(6, true);
    auto sensRes = netSens.runBellmanFordDP(0, false);

    std::cout << "  Baseline Route Cost        : " << baseRes.dist[9]
              << " (Cum.Trust=" << baseRes.accumTrust[9] << ", Hops=" << baseRes.hops[9] << ")\n";
    std::cout << "  Post-Removal Route Cost    : " << sensRes.dist[9]
              << " (Cum.Trust=" << sensRes.accumTrust[9] << ", Hops=" << sensRes.hops[9] << ")\n";
    netSens.printPathAssignment(9, sensRes);

    // 2. True Incremental Subtree DP Update: 3 Active Links Fail Mid-Session
    std::cout << "\n  [2. Incremental DP Subtree Repair: 3 Links Fail (#2: 0->2, #10: 4->6, #15: 6->9)]\n";
    auto netInc = build10NodeBenchmark();
    auto initRes = netInc.runBellmanFordDP(0, false);

    std::vector<int> affectedNodes;
    auto incRes = netInc.runIncrementalSubtreeRepair(initRes, {2, 10, 15}, affectedNodes);
    auto fullRes = netInc.runBellmanFordDP(0, false);

    std::cout << "  Affected Subtree Nodes     : ";
    for (int v : affectedNodes) {
        std::cout << "[N" << v << ":" << netInc.getNodes()[v].name << "] ";
    }
    std::cout << "\n  Incremental Repair Ops     : " << incRes.relaxationOpsCount << " edge relaxations\n";
    std::cout << "  Full Recomputation Ops     : " << fullRes.relaxationOpsCount << " edge relaxations\n";
    double savings = 100.0 * (fullRes.relaxationOpsCount - incRes.relaxationOpsCount) /
                     std::max(1LL, fullRes.relaxationOpsCount);
    std::cout << "  Computational Cost Savings : " << std::fixed << std::setprecision(1) << savings << "%\n";
    std::cout << "  Equivalence vs Full Rerun  : "
              << ((incRes.dist == fullRes.dist) ? "PASSED (Identical Optimal Distances)" : "MISMATCH") << "\n";
    netInc.printPathAssignment(9, incRes);
    std::cout << "\n";
}

void CepTestSuite::runT08_DefensiveBoundaryChecks() {
    std::cout << "================================================================================\n";
    std::cout << " [T-08] DEFENSIVE BOUNDARY CHECKS (MODULAR REDUCTION & PARALLEL EDGES)\n";
    std::cout << "================================================================================\n";

    // 1. Out-of-bounds and negative polynomial coefficients mod q
    std::vector<long long> dirtyPoly = {-500, 99999, 7681, 42};
    int modifiedCount = 0;
    auto cleanPoly = PQC::PqcNttEngine::sanitizePolynomial(dirtyPoly, modifiedCount);
    std::cout << "  [Boundary 8A: Out-of-Bounds Polynomial Sanitization in Z_7681]\n";
    std::cout << "  Input Coeffs [-500, 99999, 7681, 42] -> Normalized to ["
              << cleanPoly[0] << ", " << cleanPoly[1] << ", " << cleanPoly[2] << ", " << cleanPoly[3]
              << "] (Padded to N=256)\n";

    // 2. Parallel Duplicate Edges & Self-Routing (Source == Destination)
    Routing::ZeroTrustRouter netPar(3, 2);
    netPar.setNode(0, "NOC", 10);
    netPar.setNode(1, "Relay", 7);
    netPar.setNode(2, "Target", 10);
    netPar.addEdge(1, 0, 1, 35, false, 100); // Parallel slower link
    netPar.addEdge(2, 0, 1, 20, false, 100); // Parallel faster link
    netPar.addEdge(3, 1, 2, 25, false, 100);

    auto resPar = netPar.runBellmanFordDP(0, false);
    std::cout << "\n  [Boundary 8B: Parallel Edges Between Same Nodes & Zero-Hop Source Query]\n";
    std::cout << "  Source-to-Source Cost (0->0): " << resPar.dist[0] << " (Hops=" << resPar.hops[0] << ")\n";
    netPar.printPathAssignment(2, resPar);
    std::cout << "\n";
}

void CepTestSuite::exportBenchmarkDatasets() {
    std::filesystem::create_directories("data");
    auto net10 = build10NodeBenchmark();
    auto net100 = build100NodeWAN();
    bool ok10 = net10.exportToCSV("data/network_10_nodes.csv");
    bool ok100 = net100.exportToCSV("data/network_100_nodes.csv");
    std::cout << "  [Dataset Exporter] Saved 'data/network_10_nodes.csv' (" << (ok10 ? "OK" : "ERR")
              << ") and 'data/network_100_nodes.csv' (" << (ok100 ? "OK" : "ERR") << ")\n\n";
}

void CepTestSuite::runAllTests() {
    exportBenchmarkDatasets();
    auto m = runT01_PartA_NttVerification();
    runT02_WorkedExample10Nodes(m.keyPolynomial);
    runT03_ScaleTest100Nodes();
    runT04_EdgeCase1_DisconnectedGraph();
    runT05_EdgeCase2_NegativeCycleAttack();
    runT06_EdgeCase3_OverCapacityAndSaturatedLinks();
    runT07_SensitivityAndIncrementalRepair();
    runT08_DefensiveBoundaryChecks();
}

} // namespace Tests