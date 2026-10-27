#ifndef CEP_TEST_SUITE_HPP
#define CEP_TEST_SUITE_HPP

#include "ZeroTrustRouter.hpp"
#include "PqcNttEngine.hpp"
#include <vector>
#include <string>

namespace Tests {

    class CepTestSuite {
    public:
        static Routing::ZeroTrustRouter build10NodeBenchmark();
        static Routing::ZeroTrustRouter build100NodeWAN();

        // T-01: Part A Divide-and-Conquer NTT vs Brute-Force O(N^2) Verification
        static PQC::NttBenchmarkMetrics runT01_PartA_NttVerification();

        // T-02: Part B & C Worked Example (10 Nodes, 20 Edges) + Tie-Breaking Trace
        static void runT02_WorkedExample10Nodes(const std::vector<long long>& sharedKey);

        // T-03: WP2 Scalability Stress Test (100-Node WAN Topology)
        static void runT03_ScaleTest100Nodes();

        // T-04: Edge Case 1 - Disconnected Graph & Isolated Node Reporting
        static void runT04_EdgeCase1_DisconnectedGraph();

        // T-05: Edge Case 2 - Adversarial Negative-Weight Cycle Detection & Quarantine
        static void runT05_EdgeCase2_NegativeCycleAttack();

        // T-06: Edge Case 3 - Over-Capacity Link Filtering & All-Links-Saturated Fallback
        static void runT06_EdgeCase3_OverCapacityAndSaturatedLinks();

        // T-07: Part C Sensitivity Analysis (Top-2 Nodes Removed) & True Incremental Repair (3 Failed Links)
        static void runT07_SensitivityAndIncrementalRepair();

        // T-08: Defensive Boundary Checks (Out-of-Bounds Polynomials, Parallel Edges, Self-Routing)
        static void runT08_DefensiveBoundaryChecks();

        // Export CSV datasets to data/ directory
        static void exportBenchmarkDatasets();

        // Run complete test matrix T-01 through T-08
        static void runAllTests();
    };

} // namespace Tests

#endif // CEP_TEST_SUITE_HPP