#ifndef ADVENT_OF_CODE_2023_DAY_HPP
#define ADVENT_OF_CODE_2023_DAY_HPP

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
auto root = std::filesystem::path(R"(C:\Users\20173607\CLionProjects\advent_of_code_2023)").make_preferred();// todo unhardcode pls

using PrinterCallback = std::function<void(const char *)>;

class Day {
public:
    Day() = delete;
    virtual ~Day() = default;
    explicit Day(int number) : Day("day_" + std::to_string(number) + "/day" + std::to_string(number) + ".txt") {}

    explicit Day(const std::string& inputFilePath) {
        auto p = std::filesystem::path(inputFilePath).make_preferred();
        text.open(root / p);
        if (! text) {
            throw std::invalid_argument(" could not read: " + (root/p).string());
        }
    }

    virtual void v1(std::ifstream& text) = 0;
    virtual void v2(std::ifstream& text) = 0;

    template<typename T> void reportSolution(const T& s) {
        solution_printer = [s](const char * prefix) {
            std::cout << prefix << s << "\n";
        };
    }

    void solve() {
        {
            v1(text);
            solution_printer("v1: ");
            reset();
        }
        {
            v2(text);
            solution_printer("v2: ");
            reset();
        }
    }

    void benchmark(int sampleCount = 10'000, double reportEveryPct = 0.05) {
        const double stepSize = sampleCount * reportEveryPct;
        auto bench = [=, this](
                const std::function<void(std::ifstream&)>& f,
                BenchmarkStats& s,
                const std::string& functionName
        ) {
            s.reset();
            s.reserve(sampleCount);
            double targetForReport = stepSize;
            std::cout << "[" << functionName << "] Benchmark: ";
            for (int i = 0; i < sampleCount; ++i) {
                auto start = chrono::steady_clock::now();
                f(this->text);
                auto end = chrono::steady_clock::now();
                s.measurement(end - start);
                this->reset();

                if (i == static_cast<int>(targetForReport)) {
                    auto pct = static_cast<double>(i) / sampleCount;
                    std::cout << (100 * pct) << "% ... ";
                    targetForReport += stepSize;
                }
            }
            std::cout << "\n";
        };

        auto f1 = [this](auto && t) { v1(std::forward<decltype(t)>(t)); };
        auto f2 = [this](auto && t) { v2(std::forward<decltype(t)>(t)); };

        BenchmarkStats v1_stats;
        BenchmarkStats v2_stats;
        v1_stats.unit = std::chrono::milliseconds{1};
        v2_stats.unit = std::chrono::milliseconds{1};

        bench(f1, v1_stats, "v1");
        bench(f2, v2_stats, "v2");

        std::cout << "v1: " << v1_stats << "\n";
        std::cout << "v2: " << v2_stats << "\n";
    }

protected:
// Call to reset input stream as well as solution (output) stream.
    virtual void reset() {
        text.clear();
        text.seekg(0);
        solution_printer = PrinterCallback{};
    }

private:
    std::ifstream text;

    PrinterCallback solution_printer;
};


#endif //ADVENT_OF_CODE_2023_DAY_HPP
