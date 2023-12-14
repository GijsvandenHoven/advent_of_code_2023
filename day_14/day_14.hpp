#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 14

NAMESPACE_DEF(DAY) {

enum class Direction : uint8_t {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

enum class Object : uint8_t {
    NONE = 0,
    ROUND_ROCK = 1,
    SQUARE_ROCK = 2
};

struct Tile {
    Object o;

    explicit Tile(Object& obj) : o(obj) {}
    explicit Tile(char c) {
        switch (c) {
            case '.': o = Object::NONE; break;
            case '#': o = Object::SQUARE_ROCK; break;
            case 'O': o = Object::ROUND_ROCK; break;
            default: throw std::logic_error("Unknown input to Tile constructor: " + std::string{c});
        }
    }

    [[nodiscard]] bool occupied() const {
        return o != Object::NONE;
    }
};

class TileGrid : public std::vector<std::vector<Tile>> {
public:
    TileGrid() = default;

    void simulateTilt(const Direction& direction) {

    }

private:
    std::pair<bool, std::pair<int, int>> adjacent(const Direction& d, int x, int y) {
        auto error = [](){ return std::make_pair<bool, std::pair<int,int>>(false, {-1, -1}); };
        auto ok = [](int x, int y) { return std::make_pair<bool, std::pair<int,int>>(true, {x, y}); };
        switch (d) {
            case Direction::NORTH:
                if (y == 0) return error();
                return ok(x, y-1);
            case Direction::EAST:
                if (x+1 == this->operator[](0).size()) return error();
                return ok(x+1, y);
            case Direction::SOUTH:
                if (x+1 == this->size()) return error();
                return ok(x, y+1);
            case Direction::WEST:
                if (x == 0) return error();
                return ok(x-1, y);
            default: throw std::logic_error("Unknown direction in 'adjacent' function");
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Object& o) {
    switch (o) { case Object::NONE: os << '.'; break; case Object::ROUND_ROCK: os << 'O'; break; case Object::SQUARE_ROCK: os << '#'; break; default: os << '?'; break; }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Tile& t) {
    os << t.o;
    return os;
}

std::ostream& operator<<(std::ostream& os, const TileGrid& tg) {
    for (auto& row : tg) {
        for (auto& t : row) {
            os << t;
        }
        os << "\n";
    }
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            std::istringstream s(line);
            tiles.emplace_back(); // new row on the grid.
            int c;
            while ((c = s.get()) != EOF) {
                tiles.back().emplace_back(static_cast<char>(c));
            }
        }
//        std::cout << "PARSED: \n";
//        std::cout << tiles << "\n";
    }

    void v1() const override {
        auto copy = tiles; // immutability issue, simulating these rorcks rolling is definitely easier by mutating the vector so let's copy it.
        copy.simulateTilt(Direction::NORTH);
        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        tiles.clear();
    }

private:
    TileGrid tiles;
};

} // namespace

#undef DAY