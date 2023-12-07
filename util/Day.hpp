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
auto root = std::filesystem::path(R"(C:\Users\20173607\CLionProjects\advent_of_code_2023)").make_preferred();// todo unhardcode pls?

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

    virtual void v1() const = 0;
    virtual void v2() const = 0;
    virtual void parse(std::ifstream& text) = 0;

    template<typename T> void reportSolution(const T& s) const {
        // We are mutating solution_printer, despite const-ness. This is to let the solution be conditionally printed.
        // i.e. in solve() we want prints, in bench() we do not. solving functions themselves are const,
        // So it's either this or assigning a global variable. I choose this.
        auto * solution_printer_ptr = const_cast<PrinterCallback *>(& solution_printer);
        *solution_printer_ptr = [s](const char * prefix) {
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

    void benchmark(int sampleCount = 10'000, double reportEveryPct = 0.05) {
        const double stepSize = sampleCount * reportEveryPct;
        auto bench = [=](
                const std::function<void()>& f,
                BenchmarkStats& s,
                const std::string& functionName,
                // resets any values that f needs to be reset. Used for the base class.
                // This should not be necessary for anything else though. Derived Solvers should NOT mutate state!
                const std::function<void()>& resetter = [](){}
        ) {
            s.reset();
            s.reserve(sampleCount);
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
                    std::cout << (100 * pct) << "% ... ";
                    targetForReport += stepSize;
                }
            }
            std::cout << "\n";
        };

        auto f0 = [this]() { parse(this->text); };
        auto f1 = [this]() { v1(); };
        auto f2 = [this]() { v2(); };

        BenchmarkStats parse_stats;
        BenchmarkStats v1_stats;
        BenchmarkStats v2_stats;
        v1_stats.unit = std::chrono::milliseconds{1};
        v2_stats.unit = std::chrono::milliseconds{1};

        bench(f0, parse_stats, "parse", [this]() { text.clear(); text.seekg(0); });
        bench(f1, v1_stats, "v1", [this](){ solution_printer = {}; });
        bench(f2, v2_stats, "v2", [this](){ solution_printer = {}; });

        std::cout << "v1: " << v1_stats << "\n";
        std::cout << "v2: " << v2_stats << "\n";
    }

private:
    std::ifstream text;

    PrinterCallback solution_printer;
};