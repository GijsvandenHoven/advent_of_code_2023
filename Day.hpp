#ifndef ADVENT_OF_CODE_2023_DAY_HPP
#define ADVENT_OF_CODE_2023_DAY_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <functional>

#include "BenchStats.hpp"

namespace chrono = std::chrono;

class Day {
public:
    Day() = delete;
    virtual ~Day() = default;
    explicit Day(int number) : Day("day_" + std::to_string(number) + "/day" + std::to_string(number) + ".txt") {}

    explicit Day(const std::string& inputFilePath) {
        text.open(inputFilePath);
        if (! text) {
            throw std::invalid_argument(" could not read: " + inputFilePath);
        }
    }

    virtual void v1(std::ifstream& text) = 0;
    virtual void v2(std::ifstream& text) = 0;

    template<typename T> void reportSolution(const T& s) {
        std::cout << s << "\n";
    }

    void solve() {
        std::cout << "v1: ";
        v1(text);
        resetStream();
        std::cout << "v2: ";
        v2(text);
        resetStream();
    }

    void benchmark(int sampleCount = 100'000, double reportEveryPct = 0.1) {
        const double stepSize = sampleCount * reportEveryPct;
        auto bench = [=](
                std::function<void(std::ifstream&)> f,
                BenchmarkStats& s,
                const std::string& functionName
        ) {
            s.reset();
            s.reserve(sampleCount);
            double targetForReport = stepSize;
            for (int i = 0; i < sampleCount; ++i) {
                auto start = chrono::steady_clock::now();
                f(this->text);
                auto end = chrono::steady_clock::now();
                s.measurement(end - start);
                this->resetStream();

                if (i == static_cast<int>(targetForReport)) {
                    auto pct = static_cast<double>(i) / sampleCount;
                    std::cout << "[" << functionName << "] Benchmark: " << (100 * pct) << "%\n";
                    targetForReport += stepSize;
                }
            }
        };

        auto f1 = [this](auto && t) { v1(std::forward<decltype(t)>(t)); };
        auto f2 = [this](auto && t) { v2(std::forward<decltype(t)>(t)); };

        BenchmarkStats v1_stats;
        BenchmarkStats v2_stats;
        v1_stats.unit = std::chrono::microseconds{1};

        bench(f1, v1_stats, "v1");
        bench(f2, v2_stats, "v2");

        std::cout << "v1: " << v1_stats << "\n";
        std::cout << "v2: " << v2_stats << "\n";
    }

private:
    std::ifstream text;

    void resetStream() {
        text.clear();
        text.seekg(0);
    }
};


#endif //ADVENT_OF_CODE_2023_DAY_HPP
