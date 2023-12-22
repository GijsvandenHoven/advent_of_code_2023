#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 22

NAMESPACE_DEF(DAY) {

struct Point;
std::istream& operator>>(std::istream& s, Point& p);

struct Point { int x; int y; int z; };

struct Cube {

    Point begin{};
    Point end{};
    int id;

    explicit Cube(const std::string& from, int id) : id(id) {
        std::istringstream s(from);

        s >> begin;
        s.ignore(1);
        s >> end;

        if (begin.x > end.x || begin.y > end.y || begin.z > end.z) { // this warning is bogus.
            throw std::logic_error("Cube: begin Point should be less or equal to end Point.");
        }
    }
};

std::istream& operator>>(std::istream& s, Point& p) {
    s >> p.x; s.ignore(1);
    s >> p.y; s.ignore(1);
    s >> p.z;

    return s;
}

std::ostream& operator<<(std::ostream& os, const Cube& c) {
    os << "Cube {\n";
    os << "\tbegin: { " << c.begin.x << ", " << c.begin.y << ", " << c.begin.z << " }\n";
    os << "\tend: { " << c.end.x << ", " << c.end.y << ", " << c.end.z << " }\n";
    os << "}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        int i = 0;
        while(std::getline(input, line)) {
            cubes.emplace_back(line, i++);
        }

        // sort cubes by (bottm) Z ascending.
        std::sort(cubes.begin(), cubes.end(), [](auto& l, auto& r){
            return l.begin.z < r.begin.z;
        });
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
    std::vector<Cube> cubes;
};

} // namespace

#undef DAY