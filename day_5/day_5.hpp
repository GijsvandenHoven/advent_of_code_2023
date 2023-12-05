#pragma once

#include <iostream>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 5

void assert(bool _, const std::string& why = "unspecified") { if (!_) throw std::logic_error(why); }

struct Mapping {
    int64_t from; // this number
    int64_t to; // is mapped to this number
    int64_t reach; // and this offset from-to offset is repeated for the next {this number} values.

    Mapping(const std::string& s) {
        std::istringstream values(s); // I wonder if it is slow to create one of these objects.
        values >> to >> from >> reach;

        assert((!values.bad()) && values.eof(), "parse error with: " + s);
    }
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
        std::array<std::vector<std::string>, 8> inputs {};
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

        std::array<std::vector<Mapping>, 7> parsed_maps {};
        int index = 0;
        // for everything but the first vector.
        std::for_each(inputs.begin() + 1, inputs.end(), [&index, &parsed_maps](auto& vec){
            // first line is the 'name' which we will throw away, the rest should be our values.
            assert(vec.size() >= 2, "Input parse problem");
            // toss the first line, it's a string describing what it is, which we are hardcoding for.
            std::for_each(vec.begin() + 1, vec.end(), [target = &parsed_maps[index]](auto& str){
                Mapping m { str };
                target->emplace_back(m);
            });
            index++;
        });

        // the first vector, 'seeds', should be exactly one line.
        assert(inputs[0].size() == 1);
        const std::string seed_values = inputs[0][0].substr(6); // "seeds: "
    }
};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY