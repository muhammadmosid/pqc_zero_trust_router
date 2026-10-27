# Quantum-Resistant Secure Network Routing and Key Distribution System

**Course:** CT-363 — Design and Analysis of Algorithms (Complex Engineering Problem / CEP)  
**Department:** Computer Science and Information Systems, NED University of Engineering and Technology  
**Language:** C++17  
**Build System:** CMake 3.20+  
**IDE:** JetBrains CLion / GCC

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Problem Statement](#2-problem-statement)
3. [Algorithmic Approach](#3-algorithmic-approach)
4. [Project Structure](#4-project-structure)
5. [Build and Run](#5-build-and-run)
6. [Automated Verification Tests](#6-automated-verification-tests)
7. [Custom Network Datasets](#7-custom-network-datasets)
8. [CEP Rubric and CLO/PLO Alignment](#8-cep-rubric-and-cloplo-alignment)

## 1. Project Overview

The **Quantum-Resistant Secure Network Routing and Key Distribution System** is a C++17 decision-support system designed to combine post-quantum cryptographic computation with security-aware network routing.

The system addresses two major engineering challenges:

- **Efficient cryptographic computation:** Polynomial multiplication used in post-quantum cryptography can be computationally expensive.
- **Secure network routing:** Cryptographic keys must travel through networks containing links with different latency, capacity, and security characteristics.

The project combines the Number Theoretic Transform (NTT) with Bellman-Ford dynamic programming to optimize cryptographic computation and identify suitable network routes.

### Key Features

- Divide-and-Conquer polynomial multiplication using NTT.
- Security-aware routing using the Bellman-Ford algorithm.
- Support for 10-node and 100-node network datasets.
- Compromised-link and over-capacity filtering.
- Negative-weight cycle detection and quarantine.
- Deterministic route tie-breaking.
- Incremental network repair after link failures.
- Automated verification tests T-01 through T-08.
- CSV-based custom network input.

## 2. Problem Statement

The system addresses two interconnected engineering problems.

### Stage A: Post-Quantum Cryptographic Computation

Traditional polynomial multiplication can require \(O(N^2)\) operations. For large polynomial inputs, this can increase computation time.

**Proposed solution:** Use the Number Theoretic Transform to perform polynomial multiplication efficiently through divide-and-conquer techniques.

### Stage B: Zero-Trust Network Routing

Network links have different latency values and security characteristics. Compromised links, capacity restrictions, and malicious routing loops can prevent safe and efficient delivery.

**Proposed solution:** Use Bellman-Ford dynamic programming to calculate minimum composite route costs, detect negative-weight cycles, and support mitigation strategies.

## 3. Algorithmic Approach

### 3.1 Divide-and-Conquer: Number Theoretic Transform

**Course mapping:** Divide and Conquer and Multiplication of Large Integers.

The NTT component performs polynomial multiplication through the following steps:

1. **Divide:** Transform polynomial coefficients into a suitable mathematical representation.
2. **Transform:** Apply the NTT using modular arithmetic and suitable roots of unity.
3. **Multiply:** Multiply corresponding transformed coefficients.
4. **Inverse transform:** Recover the polynomial product.
5. **Verify:** Compare the result against a reference implementation.

The project describes a 256-coefficient polynomial workload and includes operation-count and performance comparisons.

### 3.2 Dynamic Programming: Bellman-Ford Zero-Trust Router

**Course mapping:** Dynamic Programming and the Bellman-Ford Algorithm.

The routing component evaluates network edges using a composite cost that incorporates latency and destination-node security.

Its main operations include:

- Filtering compromised and over-capacity links.
- Calculating minimum-cost paths through repeated edge relaxation.
- Resolving equal-cost paths using cumulative security trust and hop count.
- Detecting negative-weight cycles.
- Quarantining compromised relay nodes and seeking an alternative route.
- Repairing affected downstream branches after link failures.

The routing implementation is designed to evaluate both network efficiency and security requirements.

## 4. Project Structure

```text
pqc-zero-trust-router/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── include/
│   ├── PqcNttEngine.hpp
│   ├── ZeroTrustRouter.hpp
│   └── CepTestSuite.hpp
├── src/
│   ├── PqcNttEngine.cpp
│   ├── ZeroTrustRouter.cpp
│   ├── CepTestSuite.cpp
│   └── main.cpp
├── data/
│   ├── network_10_nodes.csv
│   └── network_100_nodes.csv
└── docs/
    ├── cep_proposal_ct363.tex
    └── cep_execution_logs.txt
```

### Directory Description

| Path | Purpose |
|---|---|
| `include/` | Header files containing class and structure declarations |
| `src/` | C++ source files containing algorithm implementations |
| `data/` | Network topology datasets in CSV format |
| `docs/` | Project proposal and execution logs |
| `CMakeLists.txt` | CMake build configuration |
| `.gitignore` | Files and directories excluded from Git |
| `README.md` | Project documentation |

## 5. Build and Run

### Option A: Run Using JetBrains CLion

1. Open CLion.
2. Select **File → Open** and choose the `pqc-zero-trust-router` directory.
3. Allow CLion to detect and configure `CMakeLists.txt`.
4. Select the appropriate run configuration.
5. Click the green **Run** button or press `Shift + F10`.
6. Follow the interactive console menu to execute the verification tests.

The documented interactive menu provides the following options:

```text
[1] Run T-01: NTT vs. Brute-Force Benchmark
[2] Run T-02: 10-Node Worked Example
[3] Run T-03: 100-Node Scalability Test
[4] Run T-04: Disconnected Graph Test
[5] Run T-05: Negative-Weight Cycle Attack
[6] Run T-06: Over-Capacity Link Filtering
[7] Run T-07: Sensitivity and Incremental Repair
[8] Run T-08: Defensive Boundary Checks
[9] Run Complete Verification Matrix
[0] Exit
```

### Option B: Build Using CMake

Run the following commands from the project root:

```bash
cmake -S . -B build
cmake --build build
```

### Option C: Compile Directly Using GCC

If the project does not require additional libraries, compile the source files using:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/*.cpp -o pqc_router
```

### Run the Verification Suite

```bash
./pqc_router --all
```

### Save Execution Logs

```bash
./pqc_router --all > docs/cep_execution_logs.txt
```

### Run Individual Tests

```bash
./pqc_router --t01
./pqc_router --t02
./pqc_router --t03
./pqc_router --t04
./pqc_router --t05
./pqc_router --t06
./pqc_router --t07
./pqc_router --t08
```

### Load a Custom Dataset

```bash
./pqc_router --csv data/network_10_nodes.csv
```

**Note:** These commands assume that the executable implements the listed command-line arguments and that the corresponding CMake targets and source files are configured correctly.

## 6. Automated Verification Tests

The project defines eight verification tests, T-01 through T-08, to evaluate its algorithmic functionality and edge-case handling.

| Test | Description | Verification Goal |
|---|---|---|
| T-01 | NTT benchmark | Compare NTT output against reference polynomial multiplication |
| T-02 | 10-node network | Evaluate route selection, dynamic programming, and tie-breaking |
| T-03 | 100-node network | Assess routing scalability |
| T-04 | Disconnected graph | Identify unreachable and isolated nodes |
| T-05 | Negative-weight cycle | Detect cycles and test quarantine and mitigation |
| T-06 | Capacity filtering | Exclude links that exceed allowed capacity |
| T-07 | Sensitivity and incremental repair | Evaluate route changes and repair after link failures |
| T-08 | Boundary checks | Test coefficient normalization, parallel edges, and source-to-source routing |

Use the execution logs to record the observed results for each test.

## 7. Custom Network Datasets

Network topologies can be supplied through CSV files, allowing different scenarios to be tested without changing the C++ source code.

### 7.1 Node Format

```csv
# NODES: NODE,id,name,trustScore(1-10),isolated(0 or 1)
NODE,0,NOC-Core,10,0
NODE,1,Relay-Alpha,4,0
NODE,2,Vault-Beta,8,0
```

### 7.2 Edge Format

```csv
# EDGES: EDGE,id,u,v,latencyCost,compromised(0 or 1),maxLatencyCapacity
EDGE,1,0,1,22,0,100
EDGE,2,0,2,30,0,100
EDGE,3,1,2,19,0,100
```

### 7.3 Simulating Network Conditions

- **Compromised link:** Set the `compromised` field to `1`.
- **Over-capacity link:** Set `latencyCost` higher than `maxLatencyCapacity`.
- **Custom topology:** Add or modify node and edge records according to the CSV format.

Run the application with the desired dataset:

```bash
./pqc_router --csv data/network_10_nodes.csv
```

## 8. CEP Rubric and CLO/PLO Alignment

The project maps algorithmic implementation and verification activities to the course's Complex Engineering Problem requirements.

| CEP Attribute | CLO/PLO Target | Implementation |
|---|---|---|
| WP1: Depth of Engineering Knowledge | CLO-2 / PLO-02 | NTT and Bellman-Ford algorithm implementations |
| WP2: Range of Conflicting Requirements | CLO-3 / PLO-03 | Latency-security trade-offs, capacity filtering, and 100-node routing |
| WP3: Depth of Analysis Required | CLO-3 / PLO-03 | Mathematical models, complexity analysis, and operation counters |
| WP5: Extent of Applicable Code | CLO-2 / PLO-02 | Automated verification suite and defensive routing mechanisms |

Refer to `docs/cep_proposal_ct363.tex` for the detailed proposal and `docs/cep_execution_logs.txt` for recorded test output.

---

## Verification Checklist

- [ ] CMake configures successfully.
- [ ] All source and header files are present.
- [ ] NTT output matches the reference implementation.
- [ ] The 10-node and 100-node routing scenarios execute successfully.
- [ ] Compromised and over-capacity links are handled correctly.
- [ ] Negative-weight cycle detection and mitigation work as intended.
- [ ] Incremental repair produces valid routing results.
- [ ] All eight verification tests pass.
- [ ] Execution logs are saved for project evaluation.

**Important:** The features and test outcomes described above are project specifications. Actual performance, security properties, and test success must be confirmed by running the implementation. A custom NTT implementation should not be assumed to provide production-grade, standards-compliant ML-KEM cryptography without further validation.
