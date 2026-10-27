#ifndef PQC_NTT_ENGINE_HPP
#define PQC_NTT_ENGINE_HPP

#include <vector>
#include <string>

namespace PQC {

struct NttBenchmarkMetrics {
    std::vector<long long> keyPolynomial;
    double bruteForceTimeUs = 0.0;
    double nttTimeUs = 0.0;
    long long bruteForceMulOps = 0;
    long long nttButterflyOps = 0;
    long long primitiveRootPsi = -1;
    bool exactMatchVerified = false;
    int sanitizedCoefficients = 0;
};

class PqcNttEngine {
public:
    static constexpr int POLY_DEGREE = 256;
    static constexpr long long MOD_Q = 7681; // Prime q = 7681 (512 divides q - 1)

    static long long modPow(long long base, long long exp, long long mod);
    static long long modInverse(long long n, long long mod);
    static long long findPrimitive2NRoot(int n = POLY_DEGREE, long long q = MOD_Q);

    // Defensive Edge Case: Sanitize out-of-range or negative coefficients mod q
    static std::vector<long long> sanitizePolynomial(
        const std::vector<long long>& rawPoly, int& outModifiedCount);

    // O(N^2) Brute-Force Negacyclic Multiplication Baseline
    static std::vector<long long> multiplyBruteForce(
        const std::vector<long long>& A,
        const std::vector<long long>& B,
        long long& outMulOps);

    // O(N log N) Recursive Divide-and-Conquer Cooley-Tukey NTT
    static void recursiveNTT(
        std::vector<long long>& a,
        long long omega,
        long long& outButterflyOps);

    // Full O(N log N) Negacyclic Multiplication in Z_q[X]/(X^256 + 1)
    static std::vector<long long> multiplyNTT(
        const std::vector<long long>& A,
        const std::vector<long long>& B,
        long long psi,
        long long& outButterflyOps);

    // End-to-end verification and benchmark runner
    static NttBenchmarkMetrics runBenchmarkAndVerify(int iterations = 100);

    // Deterministic hex digest and polynomial preview
    static std::string formatKeyDigest(const std::vector<long long>& poly);
};

} // namespace PQC

#endif // PQC_NTT_ENGINE_HPP