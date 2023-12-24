#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 24

NAMESPACE_DEF(DAY) {

struct Object {
    struct R3 { int64_t x; int64_t y; int64_t z; };

    R3 start{};
    R3 delta{};

    explicit Object(const std::string& str) {
        constexpr auto getN = [](int64_t& target, auto& stream) -> void {
            stream >> target;
            stream.ignore(2);
        };

        std::istringstream s(str);
        getN(start.x, s);
        getN(start.y, s);
        getN(start.z, s);

        getN(delta.x, s);
        getN(delta.y, s);
        getN(delta.z, s);
    }
};

std::ostream& operator<<(std::ostream& os, const Object::R3& r) {
    os << "x: " << r.x << ", y: " << r.y << ", z: " << r.z;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Object& o) {
    os << "Obj; start: (" << o.start << "), delta: (" << o.delta << ")";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            objects.emplace_back(line);

            std::cout << objects.back() << "\n";
        }
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
    std::vector<Object> objects;
};

} // namespace

#undef DAY