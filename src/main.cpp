#include "CepTestSuite.hpp"
#include <iostream>
#include <string>

void printHeader() {
    std::cout << "********************************************************************************\n";
    std::cout << "* CT-363 DESIGN & ANALYSIS OF ALGORITHMS - COMPLEX ENGINEERING PROBLEM (CEP)   *\n";
    std::cout << "* Quantum-Resistant Secure Network Routing & Key Distribution System           *\n";
    std::cout << "* Author: Muhammad Mosid Khan (Roll No: CT-24100) | NEDUET                     *\n";
    std::cout << "********************************************************************************\n\n";
}

int main(int argc, char* argv[]) {
    printHeader();

    // If command-line flags are passed (e.g., ./pqc_router --all)
    if (argc > 1) {
        std::string flag = argv[1];
        if (flag == "--all") {
            Tests::CepTestSuite::runAllTests();
        } else if (flag == "--t01") {
            Tests::CepTestSuite::runT01_PartA_NttVerification();
        } else if (flag == "--t02") {
            auto m = Tests::CepTestSuite::runT01_PartA_NttVerification();
            Tests::CepTestSuite::runT02_WorkedExample10Nodes(m.keyPolynomial);
        } else if (flag == "--t03") {
            Tests::CepTestSuite::runT03_ScaleTest100Nodes();
        } else if (flag == "--t04") {
            Tests::CepTestSuite::runT04_EdgeCase1_DisconnectedGraph();
        } else if (flag == "--t05") {
            Tests::CepTestSuite::runT05_EdgeCase2_NegativeCycleAttack();
        } else if (flag == "--t06") {
            Tests::CepTestSuite::runT06_EdgeCase3_OverCapacityAndSaturatedLinks();
        } else if (flag == "--t07") {
            Tests::CepTestSuite::runT07_SensitivityAndIncrementalRepair();
        } else if (flag == "--t08") {
            Tests::CepTestSuite::runT08_DefensiveBoundaryChecks();
        } else if (flag == "--csv" && argc > 2) {
            auto customNet = Routing::ZeroTrustRouter::importFromCSV(argv[2], 2);
            auto res = customNet.runBellmanFordDP(0, true);
            customNet.printPathAssignment(customNet.getNumNodes() - 1, res);
        } else {
            Tests::CepTestSuite::runAllTests();
        }
        return 0;
    }

    // Default behavior inside CLion IDE: Run full verification suite, then offer interactive menu
    Tests::CepTestSuite::runAllTests();

    while (true) {
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << " INTERACTIVE VIVA / EVALUATION MENU (CLion Runner)\n";
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << "  [1] Run T-01: Part A Divide-and-Conquer NTT vs Brute-Force Benchmark\n";
        std::cout << "  [2] Run T-02: Part B/C 10-Node, 20-Edge Worked Example (Full DP Table)\n";
        std::cout << "  [3] Run T-03: WP2 100-Node WAN Scalability Stress Test\n";
        std::cout << "  [4] Run T-04: Edge Case 1 - Disconnected Graph & Isolated Nodes\n";
        std::cout << "  [5] Run T-05: Edge Case 2 - Negative-Weight Cycle Attack & Quarantine\n";
        std::cout << "  [6] Run T-06: Edge Case 3 - Over-Capacity & Saturated Link Filtering\n";
        std::cout << "  [7] Run T-07: Part C Sensitivity (Top-2 Removed) & Incremental DP Repair\n";
        std::cout << "  [8] Run T-08: Defensive Boundary Checks (Polynomial Mod & Parallel Links)\n";
        std::cout << "  [9] Re-run Complete Verification Matrix (T-01 to T-08)\n";
        std::cout << "  [0] Exit Application\n";
        std::cout << "Select option [0-9]: ";

        int choice = 0;
        if (!(std::cin >> choice) || choice == 0) {
            std::cout << "Exiting CEP verification suite.\n";
            break;
        }
        std::cout << "\n";
        switch (choice) {
            case 1: Tests::CepTestSuite::runT01_PartA_NttVerification(); break;
            case 2: {
                auto m = Tests::CepTestSuite::runT01_PartA_NttVerification();
                Tests::CepTestSuite::runT02_WorkedExample10Nodes(m.keyPolynomial);
                break;
            }
            case 3: Tests::CepTestSuite::runT03_ScaleTest100Nodes(); break;
            case 4: Tests::CepTestSuite::runT04_EdgeCase1_DisconnectedGraph(); break;
            case 5: Tests::CepTestSuite::runT05_EdgeCase2_NegativeCycleAttack(); break;
            case 6: Tests::CepTestSuite::runT06_EdgeCase3_OverCapacityAndSaturatedLinks(); break;
            case 7: Tests::CepTestSuite::runT07_SensitivityAndIncrementalRepair(); break;
            case 8: Tests::CepTestSuite::runT08_DefensiveBoundaryChecks(); break;
            case 9: Tests::CepTestSuite::runAllTests(); break;
            default: std::cout << "Invalid option. Choose 0-9.\n"; break;
        }
    }
    return 0;
}