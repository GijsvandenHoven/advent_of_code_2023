#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 10

namespace Day10 { // todo: eval namespacing my shit.

constexpr uint8_t none_bit = 0;
constexpr uint8_t left_bit = 1 << 0;
constexpr uint8_t right_bit = 1 << 1;
constexpr uint8_t up_bit = 1 << 2;
constexpr uint8_t down_bit = 1 << 3;
constexpr uint8_t unknown_bit = 0xFF;

enum class PipeType : uint8_t {
    UNKNOWN = unknown_bit, // 'S' piece is assigned this until it can be deduced.
    NONE = none_bit,
    UPDOWN = up_bit | down_bit,
    LEFTRIGHT = left_bit | right_bit,
    LEFTUP = left_bit | up_bit,
    UPRIGHT = up_bit | right_bit,
    RIGHTDOWN = right_bit | down_bit,
    DOWNLEFT = down_bit | left_bit
};

using PipeTypeEnumType = std::underlying_type_t<PipeType>;

struct PipeSegment {
    explicit PipeSegment(PipeType &&p) : type_(p) {}

    [[nodiscard]] bool hasLeft() const { return static_cast<PipeTypeEnumType>(type_) & left_bit; }

    [[nodiscard]] bool hasRight() const { return static_cast<PipeTypeEnumType>(type_) & right_bit; }

    [[nodiscard]] bool hasUp() const { return static_cast<PipeTypeEnumType>(type_) & up_bit; }

    [[nodiscard]] bool hasDown() const { return static_cast<PipeTypeEnumType>(type_) & down_bit; }

    [[nodiscard]] inline PipeType type() const { return type_; }

private:
    PipeType type_; // consider this immutable.
};

enum class Direction : uint8_t {
    NONE = none_bit,
    UP = up_bit,
    DOWN = down_bit,
    LEFT = left_bit,
    RIGHT = right_bit
};

using DirectionEnumType = std::underlying_type_t<Direction>;

// these should be of comparable type.
static_assert(std::is_same_v<PipeTypeEnumType, DirectionEnumType>);

class Maze : public std::vector<std::vector<PipeSegment>> {
public:
    [[nodiscard]] const PipeSegment &at(int x, int y) const { return this->operator[](y)[x]; }
};

struct SearchablePipeSegment : public PipeSegment {
    bool visited = false;

    explicit SearchablePipeSegment(PipeType &&p) : PipeSegment(std::forward<PipeType>(p)) {}
};

//class SearchableMaze : public std::vector<std::vector<SearchablePipeSegment>> {
//public:
//    [[nodiscard]] const PipeSegment &at(int x, int y) const { return this->operator[](y)[x]; }
//};

std::ostream& operator<<(std::ostream& os, const PipeType& pt) {
    os << std::to_string(static_cast<PipeTypeEnumType>(pt));
    return os;
}

std::ostream& operator<<(std::ostream& os, const Direction& pt) {
    os << std::to_string(static_cast<DirectionEnumType>(pt));
    return os;
}

std::ostream& operator<<(std::ostream& os, const PipeSegment& p) {
    switch (p.type()) {
        case PipeType::NONE:            os << '.'; break;
        case PipeType::LEFTRIGHT:       os << '-'; break;
        case PipeType::UPDOWN:          os << '|'; break;
        case PipeType::UPRIGHT:         os << 'L'; break;
        case PipeType::RIGHTDOWN:       os << 'F'; break;
        case PipeType::DOWNLEFT:        os << '7'; break;
        case PipeType::LEFTUP:          os << 'J'; break;
        default: throw std::logic_error("Unknown PipeSegment.type() in operator<< of PipeSegment.");
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Maze& m) {
    std::for_each(m.begin(), m.end(), [&os](const auto& row) {
        std::for_each(row.begin(), row.end(), [&os](const auto& item) {
            os << item;
        });
        os << "\n";
    });
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        int c;
        int SX = 0;
        int SY = 0;

        int x = 1; // both off by 1 since we are giving the maze a 1 layer apron.
        int y = 1;
        maze.emplace_back(); // add the first row of the maze. This shall be filled with a row of nothing, preventing OOB access.
        maze.emplace_back(); // This is the first row of actual data.
        maze.back().emplace_back(PipeType::NONE); // start the row with 1x nothing.
        while ((c = input.get()) != EOF) {
            auto &mazeRow = maze.back();
            switch (c) {
                case '\n':
                    mazeRow.emplace_back(PipeType::NONE); // end the row with 1x nothing.
                    y++;
                    x = 1;
                    maze.emplace_back(); // new row.
                    maze.back().emplace_back(PipeType::NONE); // start the row with 1x nothing.
                    continue; // do not go 'below' the switch, where x is incremented.
                case '.':
                    mazeRow.emplace_back(PipeType::NONE);
                    break; // nothing here.
                case '-': // leftright
                    mazeRow.emplace_back(PipeType::LEFTRIGHT);
                    break;
                case '|': // updown
                    mazeRow.emplace_back(PipeType::UPDOWN);
                    break;
                case 'J': // leftup
                    mazeRow.emplace_back(PipeType::LEFTUP);
                    break;
                case 'L': // upright
                    mazeRow.emplace_back(PipeType::UPRIGHT);
                    break;
                case 'F': // rightdown
                    mazeRow.emplace_back(PipeType::RIGHTDOWN);
                    break;
                case '7': // downleft
                    mazeRow.emplace_back(PipeType::DOWNLEFT);
                    break;
                case 'S': // starting point, unknown pipe segment.
                    mazeRow.emplace_back(PipeType::UNKNOWN);
                    SX = x;
                    SY = y;
                    break;
                default:
                    throw std::logic_error("Unknown char " + std::string{static_cast<char>(c)});
            }
            x++;
        }
        if (SX == 0 || SY == 0) {
            throw std::logic_error("S was never assigned."); // S cannot be 0,0 since the perimeter belongs to the apron.
        }
        // EOF was reached. Add the bottom apron.
        maze.back().emplace_back(PipeType::NONE); // right side of the final row
        maze.emplace_back(); // add the bottom row.
        // Fill the top and bottom apron rows with actual data, they have been empty thus far.
        // An 'evenly sized' maze is assumed, i.e. every row has the same number of columns.
        for (int z = 0; z < maze[1].size(); ++z) {
            maze[0].emplace_back(PipeType::NONE); // first row
            maze.back().emplace_back(PipeType::NONE); // last row
        }

        auto &S = maze[SY][SX]; // can't use at, we want a mutable reference.
        if (S.type() != PipeType::UNKNOWN) {
            throw std::logic_error("Wrongly assigned S coords.");
        }

        // Figure out what 'S' is supposed to be.
        auto &left = maze.at(SX - 1, SY); //[startY][startX - 1];
        auto &right = maze.at(SX + 1, SY);//[startY][startX + 1];
        auto &up = maze.at(SX, SY - 1);   //[startY - 1][startX];
        auto &down = maze.at(SX, SY + 1); //[startY + 1][startX];

        PipeTypeEnumType possible_s_directions = none_bit;
        if (left.hasRight()) possible_s_directions |= left_bit;
        if (right.hasLeft()) possible_s_directions |= right_bit;
        if (down.hasUp()) possible_s_directions |= down_bit;
        if (up.hasDown()) possible_s_directions |= up_bit;

        switch (possible_s_directions) {
            default:
                throw std::logic_error("Possible S directions should be exactly two bits. Got: " + std::to_string(possible_s_directions));
            case static_cast<PipeTypeEnumType>(PipeType::LEFTRIGHT):
            case static_cast<PipeTypeEnumType>(PipeType::UPDOWN):
            case static_cast<PipeTypeEnumType>(PipeType::LEFTUP):
            case static_cast<PipeTypeEnumType>(PipeType::UPRIGHT):
            case static_cast<PipeTypeEnumType>(PipeType::RIGHTDOWN):
            case static_cast<PipeTypeEnumType>(PipeType::DOWNLEFT):
                S = PipeSegment(static_cast<PipeType>(possible_s_directions));
                break;
        }

        startX = SX;
        startY = SY;
//        std::cout << "maze constructed:\n";
//        std::cout << maze << "\n";
    }

    void v1() const override {
        int steps = 0;
        int x = startX;
        int y = startY;
        auto cameFrom = Direction::NONE;
        do {
            auto& place = maze.at(x, y);
            cameFrom = tryTravel(place, cameFrom);
            switch (cameFrom) {
                case Direction::UP:     y--; cameFrom = Direction::DOWN;    break;
                case Direction::DOWN:   y++; cameFrom = Direction::UP;      break;
                case Direction::LEFT:   x--; cameFrom = Direction::RIGHT;   break;
                case Direction::RIGHT:  x++; cameFrom = Direction::LEFT;    break;
                default: throw std::logic_error("Impossible travel direction given by tryTravel().");
            }
            steps++;
        } while (! (x == startX && y == startY));

        reportSolution(steps / 2);
    }

    void v2() const override {
//        std::cout << "SEPARATOR\n\n";
        Maze copy;
        copy.reserve(maze.size());
        for (auto& row : maze) {
            copy.emplace_back();
            for(int i = 0; i < row.size(); ++i) {
                copy.back().emplace_back(PipeType::NONE);
            }
        }

        int x = startX;
        int y = startY;
        auto cameFrom = Direction::NONE;
        do {
            auto& place = maze.at(x, y);
            copy[y][x] = place;
            cameFrom = tryTravel(place, cameFrom);
            switch (cameFrom) {
                case Direction::UP:     y--; cameFrom = Direction::DOWN;    break;
                case Direction::DOWN:   y++; cameFrom = Direction::UP;      break;
                case Direction::LEFT:   x--; cameFrom = Direction::RIGHT;   break;
                case Direction::RIGHT:  x++; cameFrom = Direction::LEFT;    break;
                default: throw std::logic_error("Impossible travel direction given by tryTravel().");
            }
        } while (! (x == startX && y == startY));

        std::cout << copy << "\n";

        reportSolution(0);
    }

    void parseBenchReset() override {
        startX = 0;
        startY = 0;
        maze.clear();
    }

private:
    Maze maze;
    int startX = 0;
    int startY = 0;

    static Direction tryTravel(const PipeSegment &from, const Direction &enterDirection) {
        auto e = static_cast<DirectionEnumType>(enterDirection);

        if (from.hasLeft() && !(e & left_bit)) {
            return Direction::LEFT;
        }
        if (from.hasRight() && !(e & right_bit)) {
            return Direction::RIGHT;
        }
        if (from.hasUp() && !(e & up_bit)) {
            return Direction::UP;
        }
        if (from.hasDown() && !(e & down_bit)) {
            return Direction::DOWN;
        }

        auto t = static_cast<PipeTypeEnumType>(from.type());
        std::string error = "Impossible pipe configuration, cannot travel. Pipe: " + std::to_string(t) + ", Entered from: " + std::to_string(e);
        throw std::logic_error(error);
    }
};

} // namespace

#undef DAY