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
        std::cout << (*this) << "\n\n"; std::cout << this->countRocks();
        std::cout << "w\n";
        simulateTilt(Direction::WEST);
        std::cout << (*this) << "\n\n"; std::cout << this->countRocks();
        std::cout << "s\n";
        simulateTilt(Direction::SOUTH);
        std::cout << (*this) << "\n\n"; std::cout << this->countRocks();
        std::cout << "e\n";
        simulateTilt(Direction::EAST);
        std::cout << (*this) << "\n\n"; std::cout << this->countRocks();
        std::cout << "done\n";
    }

    void simulateTilt(const Direction& direction) {
        auto& self = *this;

        /**
         * This operation mutates the TileGrid simulating a tilt, given direction and x,y coordinates.
         * Only rolling rocks are affected by the tilt.
         * The operation is not aware of update order, so it could overwrite rocks not yet handled in isolation.
         * (e.g. south tilt, operating on a top-to-bottom loop, could overwrite a rock that was not yet rolled,
         * which is then rolled netting minus one rock.)
         *
         * Therefore the lambda should be called in the right loop structure. That is,
         * 'normal' (top-bottom, left-right) for NORTH and WEST tilts.
         * 'reverse' (bottom-top, right-left) for SOUTH and EAST tilts.
         */
        auto tileTiltSimulation = [&self](int x, int y, const Direction& direction) {
            if (self.at(x, y).rollingRock()) {
                auto [newX, newY] = self.simulateRoll(x, y, direction);
                // first set old location to empty. If the new location == old location, doing otherwise would break things.
                self.at(x, y) = Tile();
                self.at(newX, newY) = Tile(Object::ROUND_ROCK);
            }
        };

        switch (direction) {
            case Direction::NORTH:
            case Direction::WEST:
                for (int y = 0; y < self.size(); ++y) {
                    for (int x = 0; x < self[y].size(); ++x) {
                        tileTiltSimulation(x, y, direction);
                    }
                }
                break;
            case Direction::EAST:
            case Direction::SOUTH:
                for (int y = static_cast<int>(self.size()) - 1; y >= 0; --y) {
                    for (int x = static_cast<int>(self[y].size()); x >= 0; --x) {
                        tileTiltSimulation(x, y, direction);
                    }
                }
                break;
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

    int countRocks() const { // used for debugging, my tilt function keeps eating rocks!
        int sum = 0;
        std::for_each(this->begin(), this->end(), [&sum](auto& row) {
            std::for_each(row.begin(), row.end(), [&sum](auto& tile) {
                sum += tile.rollingRock();
            });
        });
        return sum;
    }

    [[nodiscard]] const Tile& at(int x, int y) const { return this->operator[](y)[x]; }
    [[nodiscard]] Tile& at(int x, int y) { return this->operator[](y)[x]; }

private:
    [[nodiscard]] std::pair<int, int> simulateRoll(int startX, int startY, const Direction& d) const {
        std::cout << "roll sim with " << startX << ", " << startY << "\n";
        int x = startX;
        int y = startY;
        int offset = 0; // offset increases for every round rock found along the roll. They would occupy a space before the stopping space each.

        while (true) {
            auto [valid, xy] = adjacent(d, x, y);
            std::cout << "\tadjacent func says v: " << valid << " ( " << xy.first << ", " << xy.second << " )\n";
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
        std::cout << "\tloopEnd with variables x: " << x << ", y: " << y << ", offset: " << offset << "\n";
        switch (d) {
            default: throw std::logic_error("Unknown direction in simulateRoll.");
            case Direction::NORTH:
                y += offset;
                break;
            case Direction::SOUTH:
                y -= offset;
                break;
            case Direction::WEST:
                x += offset;
                break;
            case Direction::EAST:
                x -= offset;
                break;
        }
        std::cout << "result " << x << ", " << y << "\n";
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
//        auto copy = tiles; // immutability issue, simulating these rocks rolling is definitely easier by mutating the vector so let's copy it.
//        copy.simulateTilt(Direction::NORTH);
//        reportSolution(copy.northWeight());
        reportSolution(0);
    }

    void v2() const override {
//        auto copy = tiles;
//        std::cout << "0:\n" << copy << "\n";
//        copy.simulateTiltCycle();
//        std::cout << "1:\n" << copy << "\n";
//        std::cout << "v2 ogre\n";
//        reportSolution(0);

        auto copy = tiles; std::cout << copy << "\n";
        copy.simulateTilt(Direction::SOUTH);
        std::cout << copy << "\n";
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