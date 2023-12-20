#include <iostream>
#include <memory>
#include <map>

#include "_template/placeholders.hpp"
#include "day_01/day_1.hpp"
#include "day_02/day_2.hpp"
#include "day_03/day_3.hpp"
#include "day_04/day_4.hpp"
#include "day_05/day_5.hpp"
#include "day_06/day_6.hpp"
#include "day_07/day_7.hpp"
#include "day_08/day_8.hpp"
#include "day_09/day_9.hpp"
#include "day_10/day_10.hpp"
#include "day_11/day_11.hpp"
#include "day_12/day_12.hpp"
#include "day_13/day_13.hpp"
#include "day_14/day_14.hpp"
#include "day_15/day_15.hpp"
#include "day_16/day_16.hpp"
#include "day_17/day_17.hpp"
#include "day_18/day_18.hpp"
#include "day_19/day_19.hpp"
#include "day_20/day_20.hpp"
#include "day_21/day_21.hpp"

enum class ExitCodes {
    OK = 0,
    NO_INPUT = -1,
    BAD_INPUT = -2,
};

std::map<int, std::function<std::unique_ptr<Day>()>> day_constructor_functions = {
        { 1, [](){ return std::make_unique<Day1::Day1>(); } },
        { 2, [](){ return std::make_unique<Day2::Day2>(); } },
        { 3, [](){ return std::make_unique<Day3::Day3>(); } },
        { 4, [](){ return std::make_unique<Day4::Day4>(); } },
        { 5, [](){ return std::make_unique<Day5::Day5>(); } },
        { 6, [](){ return std::make_unique<Day6::Day6>(); } },
        { 7, [](){ return std::make_unique<Day7::Day7>(); } },
        { 8, [](){ return std::make_unique<Day8::Day8>(); } },
        { 9, [](){ return std::make_unique<Day9::Day9>(); } },
        { 10,[](){ return std::make_unique<Day10::Day10>(); } },
        { 11,[](){ return std::make_unique<Day11::Day11>(); } },
        { 12,[](){ return std::make_unique<Day12::Day12>(); } },
        { 13,[](){ return std::make_unique<Day13::Day13>(); } },
        { 14,[](){ return std::make_unique<Day14::Day14>(); } },
        { 15,[](){ return std::make_unique<Day15::Day15>(); } },
        { 16,[](){ return std::make_unique<Day16::Day16>(); } },
        { 17,[](){ return std::make_unique<Day17::Day17>(); } },
        { 18,[](){ return std::make_unique<Day18::Day18>(); } },
        { 19,[](){ return std::make_unique<Day19::Day19>(); } },
        { 20,[](){ return std::make_unique<Day20::Day20>(); } },
        { 21,[](){ return std::make_unique<Day21::Day21>(); } },
        { 22,[](){ return std::make_unique<Day22::Day22>(); } },
        { 23,[](){ return std::make_unique<Day23::Day23>(); } },
        { 24,[](){ return std::make_unique<Day24::Day24>(); } },
        { 25,[](){ return std::make_unique<Day25::Day25>(); } },
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Require input: [rootFolder] [solve|bench] [dayNumber] (bench_sample_size)\n";
        return static_cast<int>(ExitCodes::NO_INPUT);
    }

    Day::setRoot(argv[1]);
    std::string mode = argv[2];
    int day = std::stoi(argv[3]);

    std::cout << mode << " day " << day << "\n";

    // looking up a day that does not exist will cause std::bad_function_call to be thrown,
    // because operator[] creates a new default-initialized value if the key is not found.
    // looking up a day that is not implemented will cause std::logic_error to be thrown,
    // because you shouldn't do that.
    auto solver = day_constructor_functions[day]();

    if (mode == "solve") {
        solver->solve();
    } else if (mode == "bench") {
        if (argc > 4) {
            solver->benchmark(std::stoi(argv[4]));
        } else {
            solver->benchmark();
        }
    } else {
        std::cout << "unknown mode '" << mode << "'\n";
        return static_cast<int>(ExitCodes::BAD_INPUT);
    }

    return static_cast<int>(ExitCodes::OK);
}