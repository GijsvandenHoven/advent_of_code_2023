#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 10

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {

    }

    void v1() const override {
        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
};

#undef DAY