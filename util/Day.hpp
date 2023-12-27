#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <functional>
#include <any>
#include <filesystem>

#include "BenchStats.hpp"

namespace chrono = std::chrono;

using PrinterCallback = std::function<void(const char *)>;

class Day {
public:
    Day() = delete;
    virtual ~Day() = default;
    explicit Day(int number) : Day((number < 10 ? "day_0" : "day_") + std::to_string(number) + "/day" + std::to_string(number) + ".txt") {}

    explicit Day(const std::string& inputFilePath) {
        auto p = std::filesystem::path(inputFilePath).make_preferred();
        text.open(root / p);
        if (! text) {
            throw std::invalid_argument(" could not read: " + (root/p).string());
        }
    }

    virtual void v1() const = 0;
    virtual void v2() const = 0;
    virtual void parse(std::ifstream& text) = 0;
    virtual void parseBenchReset() = 0;

    template<typename T> void reportSolution(const T& s) const {
        solution_printer = [s](const char * prefix) {
            std::cout << prefix << s << "\n";
        };
    }

    void solve() {
        parse(text);
        v1();
        solution_printer("v1: ");
        v2();
        solution_printer("v2: ");
    }

    using StatTriplet = std::array<BenchmarkStats, 3>; // A surprise tool that will help us later.

    void benchmark(int sampleCount = 10'000, double reportEveryPct = 0.05) {
        StatTriplet s;
        benchmark(s, sampleCount, reportEveryPct, true);
    }

    void benchmark(StatTriplet& outStats, int sampleCount, double reportEveryPct, bool printStats) {
        auto bench_w_params = [sampleCount, reportEveryPct](auto& func, auto& stats, auto& str, auto& resetFunc){
            bench(sampleCount, reportEveryPct, func, stats, str, resetFunc);
        };

        auto f0 = [this]() { parse(this->text); };
        auto f1 = [this]() { v1(); };
        auto f2 = [this]() { v2(); };

        BenchmarkStats parse_stats(std::chrono::nanoseconds{1});
        BenchmarkStats v1_stats(std::chrono::milliseconds{1});
        BenchmarkStats v2_stats(std::chrono::milliseconds{1});

        auto resetSolver = [this](){ solution_printer = {}; };
        auto resetParser = [this](){
            text.clear();
            text.seekg(0);
            parseBenchReset(); // resets derived class structs that were parsed into memory.
        };

        {
            bench_w_params(f0, parse_stats, "parse", resetParser);
        }
        {
            // before benchmarking these solvers, parse the text. They need it, or they operate on empty data.
            // Due to immutability, this has to be done only once.
            // Parse benching resets the parser each time, so we must do it at least once.
            parse(text);
            bench_w_params(f1, v1_stats, "v1", resetSolver);
            bench_w_params(f2, v2_stats, "v2", resetSolver);
        }

        if (printStats) {
            std::cout << "parse: " << parse_stats << "\n";
            std::cout << "v1: " << v1_stats << "\n";
            std::cout << "v2: " << v2_stats << "\n";
        }

        outStats[0] = std::move(parse_stats);
        outStats[1] = std::move(v1_stats);
        outStats[2] = std::move(v2_stats);
    }

    static void setRoot(const std::string& r) {
        Day::root = r;
    }

private:
    std::ifstream text;

    mutable PrinterCallback solution_printer;

    static std::filesystem::path root;

    static void bench(
        int sampleCount,
        double reportEveryPct,
        const std::function<void()>& f,
        BenchmarkStats& s,
        const std::string& functionName,
        // resets any values that f needs to be reset. Used for the base class.
        // This should not be necessary for anything else though. Derived Solvers should NOT mutate state!
        const std::function<void()>& resetter = [](){}
    ) {
        s.reset();
        s.reserve(sampleCount);
        const double stepSize = sampleCount * reportEveryPct;
        double targetForReport = stepSize;
        std::cout << "[" << functionName << "] Benchmark: ";
        for (int i = 0; i < sampleCount; ++i) {
            auto start = chrono::steady_clock::now();
            f();
            auto end = chrono::steady_clock::now();
            s.measurement(end - start);
            resetter();

            if (i == static_cast<int>(targetForReport)) {
                auto pct = static_cast<double>(i) / sampleCount;
                std::cout << (100 * pct) << "%  ";
                targetForReport += stepSize;
            }
        }
        std::cout << "\n";
    }
};