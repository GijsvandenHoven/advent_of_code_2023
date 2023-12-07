#pragma once

#include <iostream>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) explicit CONCATENATE(Day, D) (const std::filesystem::path& r) : Day(D, r) {}

#define DAY 1

/**
 * Retroactively added to this template, used to be a lone int main() file.
 * The original consumed the ifstream directly, which is hard to replicate with immutability.
 * To do this, we parse the input into one big string, and use a stringstream on it in the solvers.
 */

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {
        std::ostringstream s;
        s << input.rdbuf();
        entire_input_string = s.str();
    }

    void v1() const override {
        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        entire_input_string.clear();
    }

private:
    std::string entire_input_string;
};

#undef CONCATENATE
#undef CLASS_DEF
#undef DEFAULT_CTOR_DEF
#undef DAY