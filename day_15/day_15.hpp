#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 15

NAMESPACE_DEF(DAY) {

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        std::getline(input, line);
        std::istringstream in(line);
        std::ostringstream read;

        int c;
        while ((c = in.get()) != EOF) {
            if (c == ',') { // commit this sequence to the vector, clear the read buffer.
                sequence.emplace_back(std::move(read.str()));
                read.clear();
                read.str(std::string());
            } else { // build the sequence more.
                read << static_cast<char>(c);
            }
        }
        sequence.emplace_back(std::move(read.str())); // eof did not yet commit the final one.
    }

    void v1() const override {
        int sum = 0;
        for (auto& s : sequence) {
            uint8_t seed = 0;
            for (char c : s) {
                seed = hash(c, seed);
            }
            sum += seed;
        }

        reportSolution(sum);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        sequence.clear();
    }

private:
    std::vector<std::string> sequence;

    static uint8_t hash(char in, uint8_t seed) {
        return (seed + in) * 17; // 'remainder' is done automatically by virtue of uint8_t. The + operation casts to int, so there will be no early modulo truncating things.
    }
};

} // namespace

#undef DAY