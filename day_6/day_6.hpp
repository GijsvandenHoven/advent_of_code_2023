#include "../util/Day.hpp"

#pragma once

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 6

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void v1(std::ifstream& input) override {
        reportSolution(0);
    }

    void v2(std::ifstream& input) override {
        reportSolution(0);
    }

private:

};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY