// bench_throughput.cpp — Throughput benchmark for constraint checking
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include "../include/flux/constraint.hpp"
#include "../include/flux/avx512_check.hpp"

using namespace flux;
using namespace std::chrono;

int main(int argc, char** argv) {
    size_t n_values = 1000000;
    size_t n_constraints = 5;
    size_t iterations = 10;

    if (argc > 1) n_values = std::atoll(argv[1]);
    if (argc > 2) n_constraints = std::atoll(argv[2]);
    if (argc > 3) iterations = std::atoll(argv[3]);

    // Generate random test data
    std::mt19937 rng(42);
    std::uniform_int_distribution<int32_t> dist(-200, 200);
    std::vector<int32_t> values(n_values);
    for (auto& v : values) v = dist(rng);

    // Constraints
    std::vector<Constraint> constraints;
    constraints.emplace_back(0, 100, Severity::WARNING, "speed");
    constraints.emplace_back(-40, 85, Severity::CRITICAL, "temp");
    constraints.emplace_back(15, 55, Severity::CRITICAL, "battery");
    constraints.emplace_back(0, 250, Severity::WARNING, "voltage");
    constraints.emplace_back(-20, 60, Severity::CAUTION, "pressure");

    std::cout << "=== Throughput Benchmark ===\n";
    std::cout << "Values: " << n_values << "\n";
    std::cout << "Constraints: " << n_constraints << "\n";
    std::cout << "Iterations: " << iterations << "\n\n";

    // Scalar benchmark
    size_t total_checks = 0;
    auto start = high_resolution_clock::now();

    for (size_t iter = 0; iter < iterations; ++iter) {
        BatchResult result = ConstraintChecker::check_batch(
            values.data(), values.size(),
            constraints.data(), std::min(constraints.size(), n_constraints)
        );
        total_checks += result.total * n_constraints;
    }

    auto end = high_resolution_clock::now();
    double elapsed = duration_cast<duration<double>>(end - start).count();
    double rate = total_checks / elapsed;

    std::cout << "Scalar throughput:\n";
    std::cout << "  " << total_checks << " checks in " << elapsed << " s\n";
    std::cout << "  " << (rate / 1e6) << " M checks/sec\n";

    // AVX-512 / scalar fallback benchmark (16-wide)
    total_checks = 0;
    start = high_resolution_clock::now();

    for (size_t iter = 0; iter < iterations; ++iter) {
        size_t batches = values.size() / 16;
        for (size_t b = 0; b < batches; ++b) {
            alignas(64) int32_t lo[16], hi[16];
            for (int i = 0; i < 16; ++i) { lo[i] = 0; hi[i] = 100; }
            uint16_t mask = AVX512Checker::check_16(values.data() + b * 16, lo, hi);
            total_checks += 16;
            (void)mask;
        }
    }

    end = high_resolution_clock::now();
    elapsed = duration_cast<duration<double>>(end - start).count();
    rate = total_checks / elapsed;

    std::cout << "\n16-wide batch throughput:\n";
    std::cout << "  " << total_checks << " checks in " << elapsed << " s\n";
    std::cout << "  " << (rate / 1e6) << " M checks/sec\n";

    return 0;
}
