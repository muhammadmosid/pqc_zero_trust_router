#include "PqcNttEngine.hpp"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace PQC {

long long PqcNttEngine::modPow(long long base, long long exp, long long mod) {
    long long res = 1;
    base %= mod;
    if (base < 0) base += mod;
    while (exp > 0) {
        if (exp & 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return res;
}

long long PqcNttEngine::modInverse(long long n, long long mod) {
    return modPow(n, mod - 2, mod);
}

long long PqcNttEngine::findPrimitive2NRoot(int n, long long q) {
    long long twoN = 2LL * n;
    for (long long cand = 2; cand < q; ++cand) {
        long long psi = modPow(cand, (q - 1) / twoN, q);
        if (modPow(psi, n, q) == q - 1) {
            return psi;
        }
    }
    return -1;
}

std::vector<long long> PqcNttEngine::sanitizePolynomial(
    const std::vector<long long>& rawPoly, int& outModifiedCount) {
    outModifiedCount = 0;
    std::vector<long long> clean(POLY_DEGREE, 0);
    std::size_t limit = std::min(rawPoly.size(), static_cast<std::size_t>(POLY_DEGREE));
    for (std::size_t i = 0; i < limit; ++i) {
        long long val = rawPoly[i];
        if (val < 0 || val >= MOD_Q) {
            outModifiedCount++;
            val %= MOD_Q;
            if (val < 0) val += MOD_Q;
        }
        clean[i] = val;
    }
    if (rawPoly.size() != static_cast<std::size_t>(POLY_DEGREE)) {
        outModifiedCount++;
    }
    return clean;
}

std::vector<long long> PqcNttEngine::multiplyBruteForce(
    const std::vector<long long>& A,
    const std::vector<long long>& B,
    long long& outMulOps) {
    int n = static_cast<int>(A.size());
    std::vector<long long> C(n, 0);
    outMulOps = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            long long prod = (A[i] * B[j]) % MOD_Q;
            outMulOps++;
            if (i + j < n) {
                C[i + j] = (C[i + j] + prod) % MOD_Q;
            } else {
                C[i + j - n] = (C[i + j - n] - prod + MOD_Q) % MOD_Q;
            }
        }
    }
    return C;
}

void PqcNttEngine::recursiveNTT(
    std::vector<long long>& a,
    long long omega,
    long long& outButterflyOps) {
    int n = static_cast<int>(a.size());
    if (n == 1) return;

    int half = n / 2;
    std::vector<long long> a_even(half), a_odd(half);
    for (int i = 0; i < half; ++i) {
        a_even[i] = a[2 * i];
        a_odd[i]  = a[2 * i + 1];
    }

    long long omega_sq = (omega * omega) % MOD_Q;
    recursiveNTT(a_even, omega_sq, outButterflyOps);
    recursiveNTT(a_odd, omega_sq, outButterflyOps);

    long long w = 1;
    for (int i = 0; i < half; ++i) {
        long long u = a_even[i];
        long long v = (w * a_odd[i]) % MOD_Q;
        a[i]        = (u + v) % MOD_Q;
        a[i + half] = (u - v + MOD_Q) % MOD_Q;
        w = (w * omega) % MOD_Q;
        outButterflyOps++;
    }
}

std::vector<long long> PqcNttEngine::multiplyNTT(
    const std::vector<long long>& A,
    const std::vector<long long>& B,
    long long psi,
    long long& outButterflyOps) {
    int n = static_cast<int>(A.size());
    long long psi_inv   = modInverse(psi, MOD_Q);
    long long omega     = (psi * psi) % MOD_Q;
    long long omega_inv = modInverse(omega, MOD_Q);
    long long n_inv     = modInverse(n, MOD_Q);

    outButterflyOps = 0;

    std::vector<long long> A_twisted(n), B_twisted(n);
    long long p_pow = 1;
    for (int i = 0; i < n; ++i) {
        A_twisted[i] = (A[i] * p_pow) % MOD_Q;
        B_twisted[i] = (B[i] * p_pow) % MOD_Q;
        p_pow = (p_pow * psi) % MOD_Q;
    }

    recursiveNTT(A_twisted, omega, outButterflyOps);
    recursiveNTT(B_twisted, omega, outButterflyOps);

    std::vector<long long> C_hat(n);
    for (int i = 0; i < n; ++i) {
        C_hat[i] = (A_twisted[i] * B_twisted[i]) % MOD_Q;
    }

    recursiveNTT(C_hat, omega_inv, outButterflyOps);

    std::vector<long long> C(n);
    long long pinv_pow = 1;
    for (int i = 0; i < n; ++i) {
        long long val = (C_hat[i] * n_inv) % MOD_Q;
        C[i] = (val * pinv_pow) % MOD_Q;
        pinv_pow = (pinv_pow * psi_inv) % MOD_Q;
    }
    return C;
}

NttBenchmarkMetrics PqcNttEngine::runBenchmarkAndVerify(int iterations) {
    NttBenchmarkMetrics metrics;
    std::mt19937 rng(24100);
    std::uniform_int_distribution<long long> dist(0, MOD_Q - 1);

    std::vector<long long> rawA(POLY_DEGREE), rawB(POLY_DEGREE);
    for (int i = 0; i < POLY_DEGREE; ++i) {
        rawA[i] = dist(rng);
        rawB[i] = dist(rng);
    }

    int modA = 0, modB = 0;
    auto A = sanitizePolynomial(rawA, modA);
    auto B = sanitizePolynomial(rawB, modB);
    metrics.sanitizedCoefficients = modA + modB;
    metrics.primitiveRootPsi = findPrimitive2NRoot(POLY_DEGREE, MOD_Q);

    std::vector<long long> c_brute, c_ntt;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < iterations; ++r) {
        c_brute = multiplyBruteForce(A, B, metrics.bruteForceMulOps);
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    auto t3 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < iterations; ++r) {
        c_ntt = multiplyNTT(A, B, metrics.primitiveRootPsi, metrics.nttButterflyOps);
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    metrics.bruteForceTimeUs = std::chrono::duration<double, std::micro>(t2 - t1).count() / iterations;
    metrics.nttTimeUs        = std::chrono::duration<double, std::micro>(t4 - t3).count() / iterations;
    metrics.exactMatchVerified = (c_brute == c_ntt);
    metrics.keyPolynomial    = c_ntt;

    return metrics;
}

std::string PqcNttEngine::formatKeyDigest(const std::vector<long long>& poly) {
    unsigned long long hashVal = 1469598103934665603ULL;
    for (long long coeff : poly) {
        hashVal ^= static_cast<unsigned long long>(coeff);
        hashVal *= 1099511628211ULL;
    }
    std::stringstream ss;
    ss << "ML-KEM-256 Digest [0x" << std::hex << std::uppercase
       << std::setw(16) << std::setfill('0') << hashVal << std::dec << "] | "
       << "C(X) = " << poly[0] << " + " << poly[1] << "X + " << poly[2]
       << "X^2 + ... + " << poly.back() << "X^255 (mod " << MOD_Q << ")";
    return ss.str();
}

} // namespace PQC