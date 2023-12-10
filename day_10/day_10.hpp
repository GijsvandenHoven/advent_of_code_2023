#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 10

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
    explicit PipeSegment(PipeType && p) : type_(p) {}

    [[nodiscard]] bool hasLeft() const  { return static_cast<PipeTypeEnumType>(type_) & left_bit; }
    [[nodiscard]] bool hasRight() const { return static_cast<PipeTypeEnumType>(type_) & right_bit; }
    [[nodiscard]] bool hasUp() const    { return static_cast<PipeTypeEnumType>(type_) & up_bit; }
    [[nodiscard]] bool hasDown() const  { return static_cast<PipeTypeEnumType>(type_) & down_bit; }

    [[nodiscard]] inline PipeType type() const { return type_; }
private:
    PipeType type_; // consider this immutable.
};

using Maze = std::vector<std::vector<PipeSegment>>;

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {
        int c = 0;
        int startX = 0;
        int startY = 0;

        int x = 1; // both off by 1 since we are giving the maze a 1 layer apron.
        int y = 1;
        maze.emplace_back(); // add the first row of the maze. This shall be filled with a row of nothing, preventing OOB access.
        maze.emplace_back(); // This is the first row of actual data.
        maze.back().emplace_back(PipeType::NONE); // start the row with 1x nothing.
        while ((c = input.get()) != EOF) {
            auto& mazeRow = maze.back();
            switch(c) {
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
                    startX = x;
                    startY = y;
                    break;
                default:
                    throw std::logic_error("Unknown char " + std::string{static_cast<char>(c)});
            }
            x++;
        }
        if (startX == 0 || startY == 0) {
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

        auto &S = maze[startY][startX];
        if (S.type() != PipeType::UNKNOWN) {
            throw std::logic_error("Wrongly assigned S coords.");
        }

        // Figure out what 'S' is supposed to be.
        auto& left = maze[startY][startX - 1];
        auto& right = maze[startY][startX + 1];
        auto& up = maze[startY - 1][startX];
        auto& down = maze[startY + 1][startX];

        PipeTypeEnumType possible_s_directions = none_bit;
        if (left.hasRight()) possible_s_directions |= left_bit;
        if (right.hasLeft()) possible_s_directions |= right_bit;
        if (down.hasUp()) possible_s_directions |= down_bit;
        if (up.hasDown()) possible_s_directions |= up_bit;

        switch(possible_s_directions) {
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

        startPosition = std::make_pair(startY, startX);
    }

    void v1() const override {
        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        startPosition = {};
        maze.clear();
    }

private:
    Maze maze;
    std::pair<int, int> startPosition; // Y , X format. Index/Assign as such: maze[pair.first][pair.second].
};

#undef DAY