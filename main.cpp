#include <iostream>
#include <memory>
#include <map>

#include "_template/placeholders.hpp"
#include "day_1/day_1.hpp"
#include "day_2/day_2.hpp"
#include "day_3/day_3.hpp"
#include "day_4/day_4.hpp"
#include "day_5/day_5.hpp"
#include "day_6/day_6.hpp"
#include "day_7/day_7.hpp"
#include "day_8/day_8.hpp"

enum class ExitCodes {
    OK = 0,
    NO_INPUT = -1,
    BAD_INPUT = -2,
};

std::map<int, std::function<std::unique_ptr<Day>(const char *)>> day_constructor_functions = {
        { 1, [](const char * c){ return std::make_unique<Day1>(c); } },
        { 2, [](const char * c){ return std::make_unique<Day2>(c); } },
        { 3, [](const char * c){ return std::make_unique<Day3>(c); } },
        { 4, [](const char * c){ return std::make_unique<Day4>(c); } },
        { 5, [](const char * c){ return std::make_unique<Day5>(c); } },
        { 6, [](const char * c){ return std::make_unique<Day6>(c); } },
        { 7, [](const char * c){ return std::make_unique<Day7>(c); } },
//        { 8, [](const char * c){ return std::make_unique<Day8>(c); } },
//        { 9, [](const char * c){ return std::make_unique<Day9>(c); } },
//        { 10,[](const char * c){ return std::make_unique<Day10>(c); } },
//        { 11,[](const char * c){ return std::make_unique<Day11>(c); } },
//        { 12,[](const char * c){ return std::make_unique<Day12>(c); } },
//        { 13,[](const char * c){ return std::make_unique<Day13>(c); } },
//        { 14,[](const char * c){ return std::make_unique<Day14>(c); } },
//        { 15,[](const char * c){ return std::make_unique<Day15>(c); } },
//        { 16,[](const char * c){ return std::make_unique<Day16>(c); } },
//        { 17,[](const char * c){ return std::make_unique<Day17>(c); } },
//        { 18,[](const char * c){ return std::make_unique<Day18>(c); } },
//        { 19,[](const char * c){ return std::make_unique<Day19>(c); } },
//        { 20,[](const char * c){ return std::make_unique<Day20>(c); } },
//        { 21,[](const char * c){ return std::make_unique<Day21>(c); } },
//        { 22,[](const char * c){ return std::make_unique<Day22>(c); } },
//        { 23,[](const char * c){ return std::make_unique<Day23>(c); } },
//        { 24,[](const char * c){ return std::make_unique<Day24>(c); } },
//        { 25,[](const char * c){ return std::make_unique<Day25>(c); } },
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Require input: [rootFolder] [solve|bench] [dayNumber] (bench_sample_size)\n";
        return static_cast<int>(ExitCodes::NO_INPUT);
    }

    // Day::setRoot(argv[1]);
    std::string mode = argv[2];
    int day = std::stoi(argv[3]);

    std::cout << mode << " day " << day << "\n";

    // looking up a day that does not exist will cause std::bad_function_call to be thrown,
    // because operator[] creates a new default-initialized value if the key is not found.
    // looking up a day that is not implemented will cause std::logic_error to be thrown,
    // because you shouldn't do that.
    auto solver = day_constructor_functions[day](argv[1]);

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