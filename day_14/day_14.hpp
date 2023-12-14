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

    [[nodiscard]] bool rollingRock() const {
        return o == Object::ROUND_ROCK;
    }
};

class TileGrid;
std::ostream& operator<<(std::ostream& os, const TileGrid& tg);

class TileGrid : public std::vector<std::vector<Tile>> {
public:
    TileGrid() = default;

    /**
     * given an 'n' representing which tilt cycle it is and a cache:
     *
     * 1) Performs a tilt cycle (N-W-S-E), mutating the TileGrid.
     * 2) stringifies the state of the TileGrid
     * 3) checks this state with the cache.
     *
     * 4a) Returns true if this state exists in the cache, or
     * 4b) Returns false and emplaces the state within the cache, if it does not.
     */
    [[nodiscard]] std::pair<bool, int> simulateTiltCycle(std::map<std::string, int>& cache, int cycleCount) {
        simulateTilt(Direction::NORTH);
        simulateTilt(Direction::WEST);
        simulateTilt(Direction::SOUTH);
        simulateTilt(Direction::EAST);

        std::ostringstream s;
        s << *this;
        std::string state = s.str();
        auto iter = cache.find(state);
        if (iter != cache.end()) {
            return std::make_pair(true, iter->second);
        } else {
            cache.emplace(std::move(state), cycleCount);
            return std::make_pair(false, 0);
        }
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
                    for (int x = static_cast<int>(self[y].size()) - 1; x >= 0; --x) {
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
            case Direction::WEST:
                x += offset;
                break;
            case Direction::EAST:
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

std::ostream& operator<<(std::ostream& os, const Direction& d) {
    switch(d) { case Direction::NORTH: os << "N"; break; case Direction::SOUTH: os << "S"; break; case Direction::WEST: os << "W"; break; case Direction::EAST: os << "E"; break; default: os << "?"; break; }
    return os;
}

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
        static constexpr int TARGET = 1'000'000'000;
        auto copy = tiles;
        std::map<std::string, int> cache;

        int lastCompletedCycle;
        int steadyCycleStart;
        int i = 1;
        for ( ; i <= TARGET; ++i) {
            auto [steadyStateDetected, firstSteadyCycle] = copy.simulateTiltCycle(cache, i);
            if (steadyStateDetected) {
                steadyCycleStart = firstSteadyCycle;
                lastCompletedCycle = i;
                break;
            }
        }
        if (i == TARGET + 1) { // stop warning me about uninitialized variables if this happens IDE.
            throw std::logic_error("You actually completed 1 billion iterations lmao.");
        }

        int stateModulo = (lastCompletedCycle - steadyCycleStart);

        // (target - offset) % modulo
        int cycleOfTarget = (TARGET - steadyCycleStart) % stateModulo;
        // while we could look up the state in the cache already and do the math on that string,
        // I think it's easier if I just get to that cycle again and call the function that does it for me.

        // We are currently on '0', we want to go to 'cycleOfTarget' which is in the range [0, modulo).
        for (int j = 0; j < cycleOfTarget; ++j) {
            (void) copy.simulateTiltCycle(cache, i + j + 1);
        }

        reportSolution(copy.northWeight());
    }

    void parseBenchReset() override {
        tiles.clear();
    }

private:
    TileGrid tiles;
};

} // namespace

#undef DAY