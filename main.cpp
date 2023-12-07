#include <iostream>
#include <memory>
#include <map>

#include "_template/placeholders.hpp"
#include "day_4/day_4_templated.hpp"
#include "day_5/day_5.hpp"
#include "day_6/day_6.hpp"
#include "day_7/day_7.hpp"
#include "day_8/day_8.hpp"

enum class ExitCodes {
    OK = 0,
    NO_INPUT = -1,
    BAD_INPUT = -2,
};

std::map<int, std::function<std::unique_ptr<Day>()>> day_constructor_functions = {
        { 1, [](){ return std::make_unique<Day1>(); } },
        { 2, [](){ return std::make_unique<Day2>(); } },
        { 3, [](){ return std::make_unique<Day3>(); } },
        { 4, [](){ return std::make_unique<Day4>(); } },
        { 5, [](){ return std::make_unique<Day5>(); } },
        { 6, [](){ return std::make_unique<Day6>(); } },
        { 7, [](){ return std::make_unique<Day7>(); } },
        { 8, [](){ return std::make_unique<Day8>(); } },
        { 9, [](){ return std::make_unique<Day9>(); } },
        { 10,[](){ return std::make_unique<Day10>(); } },
        { 11,[](){ return std::make_unique<Day11>(); } },
        { 12,[](){ return std::make_unique<Day12>(); } },
        { 13,[](){ return std::make_unique<Day13>(); } },
        { 14,[](){ return std::make_unique<Day14>(); } },
        { 15,[](){ return std::make_unique<Day15>(); } },
        { 16,[](){ return std::make_unique<Day16>(); } },
        { 17,[](){ return std::make_unique<Day17>(); } },
        { 18,[](){ return std::make_unique<Day18>(); } },
        { 19,[](){ return std::make_unique<Day19>(); } },
        { 20,[](){ return std::make_unique<Day20>(); } },
        { 21,[](){ return std::make_unique<Day21>(); } },
        { 22,[](){ return std::make_unique<Day22>(); } },
        { 23,[](){ return std::make_unique<Day23>(); } },
        { 24,[](){ return std::make_unique<Day24>(); } },
        { 25,[](){ return std::make_unique<Day25>(); } },
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Require input: [solve|bench] [dayNumber] (bench_sample_size)\n";
        return static_cast<int>(ExitCodes::NO_INPUT);
    }

    std::string mode = argv[1];
    int day = std::stoi(argv[2]);

    std::cout << mode << " day " << day << "\n";

    // looking up a day that does not exist will cause std::bad_function_call to be thrown,
    // because operator[] creates a new default-initialized value if the key is not found.
    // looking up a day that is not implemented will cause std::logic_error to be thrown,
    // because you shouldn't do that.
    auto solver = day_constructor_functions[day]();

    if (mode == "solve") {
        solver->solve();
    } else if (mode == "bench") {
        if (argc > 3) {
            solver->benchmark(std::stoi(argv[3]));
        } else {
            solver->benchmark();
        }
    } else {
        std::cout << "unknown mode '" << mode << "'\n";
        return static_cast<int>(ExitCodes::BAD_INPUT);
    }

    return static_cast<int>(ExitCodes::OK);
}