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

    Tile() : o(Object::NONE) {}
    explicit Tile(Object obj) : o(obj) {}
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

    [[nodiscard]] bool rollingRock() const {
        return o == Object::ROUND_ROCK;
    }
};

// todo remove
class TileGrid;
std::ostream& operator<<(std::ostream& os, const TileGrid& tg);

class TileGrid : public std::vector<std::vector<Tile>> {
public:
    TileGrid() = default;

    void simulateTiltCycle() {
        std::cout << "n\n";
        simulateTilt(Direction::NORTH);
        std::cout << (*this) << "\n\n";
        std::cout << "w\n";
        simulateTilt(Direction::WEST);
        std::cout << (*this) << "\n\n";
        std::cout << "s\n";
        simulateTilt(Direction::SOUTH);
        std::cout << (*this) << "\n\n";
        std::cout << "e\n";
        simulateTilt(Direction::EAST);
        std::cout << (*this) << "\n\n";
        std::cout << "done\n";
    }

    void simulateTilt(const Direction& direction) {
        auto& self = *this;
        for (int y = 0; y < self.size(); ++y) {
            for (int x = 0; x < self[y].size(); ++x) {
                if (self[y][x].rollingRock()) {
                    auto [newX, newY] = simulateRoll(x, y, direction);
                    // update location. Do the 'setting this position empty' first, if the new location == this spot,
                    // doing otherwise would break things.
                    self.at(x, y) = Tile();
                    self.at(newX, newY) = Tile(Object::ROUND_ROCK);
                    // since directions are consistent for each rock it should be safe to update 'on the fly',
                    // before 'future' rocks roll this path. They would still encounter each rock.
                }
            }
        }
    }

    int northWeight() {
        int weight = static_cast<int>(this->size());
        int sum = 0;
        for (auto& row : *this) {
            for (auto& t : row) {
                if (t.rollingRock()) {
                    sum += weight;
                }
            }
            weight--;
        }
        return sum;
    }

    [[nodiscard]] const Tile& at(int x, int y) const { return this->operator[](y)[x]; }
    [[nodiscard]] Tile& at(int x, int y) { return this->operator[](y)[x]; }

private:
    [[nodiscard]] std::pair<int, int> simulateRoll(int startX, int startY, const Direction& d) const {
        int x = startX;
        int y = startY;
        int offset = 0; // offset increases for every round rock found along the roll. They would occupy a space before the stopping space each.

        while (true) {
            auto [valid, xy] = adjacent(d, x, y);
            if (!valid) break; // going out of bounds, we are done.

            auto& current = this->at(xy.first, xy.second);
            switch (current.o) {
                default: throw std::logic_error("Unknown direction in simulateRoll.");
                case Object::ROUND_ROCK:
                    offset++;
                    [[fallthrough]];
                case Object::NONE: // we can keep rolling, so update x and y for the next iteration of the loop.
                    x = xy.first;
                    y = xy.second;
                    break;
                case Object::SQUARE_ROCK:
                    goto loopEnd;
            }
        }
        loopEnd:
        switch (d) {
            default: throw std::logic_error("Unknown direction in simulateRoll.");
            case Direction::NORTH:
                y += offset;
                break;
            case Direction::SOUTH:
                y -= offset;
                break;
            case Direction::EAST:
                x += offset;
                break;
            case Direction::WEST:
                x -= offset;
                break;
        }
        return std::make_pair(x, y);
    }

    [[nodiscard]] std::pair<bool, std::pair<int, int>> adjacent(const Direction& d, int x, int y) const {
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
                if (y+1 == this->size()) return error();
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
    }

    void v1() const override {
        auto copy = tiles; // immutability issue, simulating these rocks rolling is definitely easier by mutating the vector so let's copy it.
        copy.simulateTilt(Direction::NORTH);
        reportSolution(copy.northWeight());
    }

    void v2() const override {
        auto copy = tiles;
        std::cout << "0:\n" << copy << "\n";
        copy.simulateTiltCycle();
        std::cout << "1:\n" << copy << "\n";
        std::cout << "v2 ogre\n";
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