#include <iostream>
#include <memory>
#include <map>

#include "_template/placeholders.hpp"
#include "day_5/day_5.hpp"
#include "day_6/day_6.hpp"

enum class ExitCodes {
    OK = 0,
    NO_INPUT = 1,
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
    if (argc == 0) {
        std::cout << "Require input: DayNumber\n";
        return static_cast<int>(ExitCodes::NO_INPUT);
    }

    std::cout << "RUNNING DAY " << argv[1] << "\n";

    // looking up a day that does not exist will return in std::bad_function_call to be thrown.
    // because operator[] creates a new default-initialized value if the key is not found.
    int day = std::stoi(argv[1]);
    auto solver = day_constructor_functions[day]();

    solver->solve();

    return static_cast<int>(ExitCodes::OK);
}