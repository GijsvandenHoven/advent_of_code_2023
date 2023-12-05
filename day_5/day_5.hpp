#pragma once

#include <iostream>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 5

struct Mapping {
    int64_t from; // this number
    int64_t to; // is mapped to this number
    int64_t reach; // and this offset from-to offset is repeated for the next {this number} values.
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void v1(std::ifstream& input) override {
        parseInput(input);
        reportSolution(0);
    }

    void v2(std::ifstream& input) override {
        reportSolution(0);
    }

private:
    void parseInput(std::ifstream& input) {
        std::array<std::vector<std::string>, 8> inputs;
        std::string line;

        int i = 0;
        std::vector<std::string> * currentVector = &(inputs[i]);
        while (std::getline(input, line)) {
            if (line.empty()) {
                i++;
                currentVector = &(inputs[i]); // this would never ever break. just don't put empty lines at the end of the file pls :)
            } else {
                currentVector->push_back(line);
            }
        }

//        std::cout << "done with parse 1, how busted is it?\n";
//        std::for_each(inputs.begin(), inputs.end(), [](auto& vec) {
//            std::cout << "\tvec\n";
//            std::for_each(vec.begin(), vec.end(), [](auto& str){
//                std::cout << "\t\t" << str << "\n";
//            });
//        });

        auto& [
            seeds,
            seed_to_soil,
            soil_to_fert,
            fert_to_water,
            water_to_light,
            light_to_temp,
            temp_to_hum,
            hum_to_loc
        ] = inputs;


    }
};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY