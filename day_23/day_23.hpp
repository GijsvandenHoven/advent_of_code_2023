#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 23

NAMESPACE_DEF(DAY) {

struct Block {
    int cost;
    std::list<Block> successors;
};

enum class Direction : uint8_t {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

constexpr char END_MARKER = 'X';
constexpr char WALL_MARKER = '#';
constexpr char PATH_MARKER = '.';

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string s;
        while (std::getline(input, s)) {
            grid.emplace_back(std::move(s));
        }

        int xStart = static_cast<int>(grid[0].find('.'));
        int xEnd = static_cast<int>(grid.back().find('.'));

        grid.back()[xEnd] = END_MARKER; // mark the end.

        start = {xStart, 0};
    }

    void v1() const override {
        auto [x, y] = start;

        // build blocks
        Block startBlock {};
        calcBlock(startBlock, x, y, Direction::SOUTH);

        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
    std::vector<std::string> grid;
    std::pair<int,int> start;

    /**
     * Walk to the successor incrementing a count until...
     * ...a direction marker is reached, at which point,
     * Recursively push onto the successor list calls to this function.
     */
    void calcBlock(Block& target, int x, int y, Direction facing) const {
        int steps = 1; // including the current x, y.
        std::cout << "build block " << x << ", " << y << "\n";
        do {
            std::cout << "\titer " << x << ", " << y << "\n";
            auto [nx, ny, nfacing] = successor(x, y, facing);
            x = nx;
            y = ny;
            facing = nfacing;
            ++steps;

            if (grid[y][x] == END_MARKER) { // we are done here, write the cost and exit this recursion.
                target.cost = steps;
                break;
            }

            if (grid[y][x] != PATH_MARKER) {
                auto [interX, interY, _] = successor(x, y, facing); // we are now on the intersection
                ++steps;

                std::cout << "This block is " << steps << " long\n";
                std::cout << "Intersection square at " << interX << ", " << interY << "\n";
                target.cost = steps;
                std::array<std::pair<int,int>,4> possible{{ {interX,interY-1}, {interX,interY+1}, {interX-1,interY}, {interX+1,interY} }};
                for (auto& [px, py] : possible) {
                    if (px == x && py == y) continue;

                    auto possibleFacing = whatIsFacing(px, py, interX, interY);

                    char prospect = grid[py][px];
                    switch (prospect) {
                        case PATH_MARKER: [[fallthrough]]; // This should be an intersection of one-ways!
                        default: throw std::logic_error("Unexpected char in grid");
                        case WALL_MARKER: continue; // do not recurse here.
                        case '>': if (possibleFacing == Direction::EAST) break; else continue;
                        case 'v': if (possibleFacing == Direction::SOUTH) break; else continue;
                        case '<': if (possibleFacing == Direction::WEST) break; else continue;
                        case '^': if (possibleFacing == Direction::NORTH) break; else continue;
                    }

                    std::cout << "\trecurse " << px << ", " << py << "\n";
                    Block succeeding;
                    calcBlock(succeeding, px, py, possibleFacing);
                    target.successors.emplace_back(std::move(succeeding));
                }

                break; // we are done with this block now. We found the intersection square, appended blocks. No more walking fwd to do.
            }
        } while (grid[y][x] != END_MARKER);

        std::cout << "recursion end " << x << ", " << y << "\n";
    }

    std::tuple<int, int, Direction> successor(int x, int y, Direction facing) const {

        if (grid[y][x] == END_MARKER) throw std::logic_error("Trying to find successor of end.");

        std::array<std::pair<int,int>,3> inspect;
        switch (facing) {
            case Direction::NORTH:
                inspect = decltype(inspect){{ {x+1,y}, {x-1,y}, {x,y-1} }}; // right left up
                break;
            case Direction::SOUTH:
                inspect = decltype(inspect){{ {x+1,y}, {x-1,y}, {x,y+1} }}; // right left down
                break;
            case Direction::EAST:
                inspect = decltype(inspect){{ {x+1,y}, {x,y+1}, {x,y-1} }}; // right up down
                break;
            case Direction::WEST:
                inspect = decltype(inspect){{ {x-1,y}, {x,y+1}, {x,y-1} }}; // left up down
                break;
            default: throw std::logic_error("Unknown enum value.");
        }

        int validCount = 0;
        int outX;
        int outY;
        for (auto [nx, ny] : inspect) {
            char here = grid[ny][nx];
            if (here != WALL_MARKER) {
                validCount ++;
                outX = nx;
                outY = ny;
            }
        }

        if (validCount != 1) {
            throw std::logic_error("Expected exactly one successor to exist, got " + std::to_string(validCount) + " at " + std::to_string(x) + ", " + std::to_string(y) + ".");
        }

        Direction newFacing = whatIsFacing(outX, outY, x, y);

        return std::make_tuple(outX, outY, newFacing);
    }

    static Direction whatIsFacing(int hereX, int hereY, int lastX, int lastY) {
        if (hereX == lastX) { // up or down
            return hereY > lastY ? Direction::SOUTH : Direction::NORTH;
        } else if (hereY == lastY) { // left or right
            return hereX > lastX ? Direction::EAST : Direction::WEST;
        } else throw std::logic_error("Non-cardinal successor");
    }
};

} // namespace

#undef DAY