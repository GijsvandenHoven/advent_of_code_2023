#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 23

NAMESPACE_DEF(DAY) {

struct Block {
    int startX = 0;
    int startY = 0;
    int cost = 0;
    std::list<std::shared_ptr<Block>> successors;
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

std::ostream& operator<<(std::ostream& os, const Block& b) {
    os << "Block with coords (" << b.startX << ", " << b.startY << ") len (" << b.cost << ") successors (" << b.successors.size() << "):\n";
    for (auto& s : b.successors) {
        std::cout << *s << "\n";
    }
    os << "End block " << b.startX << ", " << b.startY << " successor list.";

    return os;
}

struct BlockComparator {
    bool operator()(const std::shared_ptr<Block>& a, const std::shared_ptr<Block>& b) const {
        return a->startX < b->startX || (a->startX == b->startX && a->startY < b->startY);
    }
};

using BlockCache = std::set<std::shared_ptr<Block>, BlockComparator>;

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

        // reduce the graph by collapsing the labyrinth sections into "blocks" with appropriate length.
        BlockCache cache;
        auto iterToStart = calcBlock(x, y, Direction::SOUTH, cache);

        int longest = calcLongestPath(**iterToStart);
        std::cout << "ref case " << longest-1 << "\n";
        cache.clear();

        BlockCache neoCache;
        auto iterToNeoStart = neoCalcBlock(x, y + 1, Direction::SOUTH, neoCache);
        int neoLongest = calcLongestPath(**iterToNeoStart);

        std::cout << **iterToNeoStart << "\n";

        reportSolution(neoLongest);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
    std::vector<std::string> grid;
    std::pair<int,int> start;

    // NP-hard :) that's why we collapsed to blocks.
    int calcLongestPath(const Block& first) const {
        int max = 0;
        for (auto& s : first.successors) {
            max = std::max(max, calcLongestPath(*s));
        }

        return first.cost + max;
    }

    BlockCache::iterator neoCalcBlock(const int x, const int y, const Direction facing, BlockCache& cache) const {
        auto block = std::make_shared<Block>(x, y);
        block->startX = x;
        block->startY = y;
        auto [iter, success] = cache.emplace(std::move(block));
        if (! success) throw std::logic_error("Inserting block which already exists. " + std::to_string(x) + ", " + std::to_string(y));
        auto& target = **iter;
        int steps = 0; // including the current x, y.

        int cx = x;
        int cy = y;
        Direction cfacing = facing;
        bool pathToWalk = true;
        while (pathToWalk) {
            steps++;

            //std::cout << "at " << cx << ", " << cy << "\n";
            std::array<std::pair<int,int>, 4> options;
            int count = numberOfPaths(cx, cy, options);
            for (int i = 0; i < count; ++i) {
                auto opt = options[i];
                //std::cout << "option: " << opt.first << ", " << opt.second << "\n";
            }

            Direction oppositeFacing;
            switch (cfacing) {
                case Direction::SOUTH: oppositeFacing = Direction::NORTH; break;
                case Direction::NORTH: oppositeFacing = Direction::SOUTH; break;
                case Direction::EAST: oppositeFacing = Direction::WEST; break;
                case Direction::WEST: oppositeFacing = Direction::EAST; break;
            }

            int skipThisIndex = 512;
            for (int i = 0; i < count; ++i) {
                auto [nx, ny] = options[i];
                auto nFacing = whatIsFacing(nx, ny, cx, cy);

                if (nFacing == oppositeFacing) skipThisIndex = i;
            }
            // banish the skip index to the last index.
            std::swap(options[count-1], options[skipThisIndex]);

            for (int i = 0; i < count-1; ++i) { // for everything in the valid portion of the array minus the banished element
                auto [nx, ny] = options[i];
                char next = grid[ny][nx];
                //std::cout << "from " << cx << ", " << cy << " try " << nx << ", " << ny << "\n";
                auto potentialFacing = whatIsFacing(nx, ny, cx, cy);

                if (next != PATH_MARKER) {
                    pathToWalk = false;

                    if (next == END_MARKER) {
                        target.cost = steps+1;
                        return iter; // exit immediately.
                    }

                    if (count == 2) { // This is the end of a block, we are entering an intersection square.
                        target.cost = steps + 1;
                        switch (potentialFacing) { // no need to update facing, it's a straight line to the center dot.
                            case Direction::NORTH:
                                --ny;
                                break;
                            case Direction::SOUTH:
                                ++ny;
                                break;
                            case Direction::EAST:
                                ++nx;
                                break;
                            case Direction::WEST:
                                --nx;
                                break;
                        }

                        auto iterToCache = std::find_if(cache.begin(), cache.end(), [_x = nx, _y = ny](auto& c){
                            return c->startX == _x && c->startY == _y;
                        });

                        if (iterToCache == cache.end()) {
                            auto newBlock = neoCalcBlock(nx, ny, potentialFacing, cache);
                            target.successors.emplace_back(*newBlock);
                        } else {
                            target.successors.emplace_back(*iterToCache);
                        }

                        //pathToWalk = false; // end of loop.

                    } else { // intersection square
                        //std::cout << "\tIntersection after " << steps << "\n";
                        if (steps != 1) throw std::logic_error("Intersection square but steps not 1.");

                        target.cost = 1;

                        // Is this legal? only if we are facing the same way as the one-way after moving onto it.
                        switch (next) {
                            default: throw std::logic_error("not a one-way?");
                            case 'v': if (potentialFacing != Direction::SOUTH) continue; break;
                            case '^': if (potentialFacing != Direction::NORTH) continue; break;
                            case '>': if (potentialFacing != Direction::EAST) continue; break;
                            case '<': if (potentialFacing != Direction::WEST) continue; break;
                        }

                        auto iterToCache = std::find_if(cache.begin(), cache.end(), [_x = nx, _y = ny](auto& c){
                            return c->startX == _x && c->startY == _y;
                        });

                        if (iterToCache == cache.end()) {
                            auto newBlock = neoCalcBlock(nx, ny, potentialFacing, cache);
                            target.successors.emplace_back(*newBlock);
                        } else {
                            target.successors.emplace_back(*iterToCache);
                        }

                        //pathToWalk = false;
                    }
                } else {
                    if (count != 2) throw std::logic_error("Path marker & Option inconsistency.");
                    // just go to the next spot.
                    cfacing = potentialFacing;
                    cx = nx;
                    cy = ny;
                }
            }

            //std::cout << "end of for loop with " << cx << ", " << cy << "\n";
        }

        //std::cout << "end of while loop with " << cx << ", " << cy << "\n";

        return iter;
    }

    /**
     * Walk to the successor incrementing a count until...
     * ...a direction marker is reached, at which point,
     * Recursively push onto the successor list calls to this function.
     */
    BlockCache::iterator calcBlock(int x, int y, Direction facing, BlockCache& cache) const {
        auto block = std::make_shared<Block>(x, y);
        block->startX = x;
        block->startY = y;
        auto [iter, success] = cache.emplace(std::move(block));
        if (! success) throw std::logic_error("Inserting block which already exists. " + std::to_string(x) + ", " + std::to_string(y));
        auto& target = **iter;

        int steps = 1; // including the current x, y.
        do {
            auto [nx, ny, nfacing] = successor(x, y, facing);
            x = nx;
            y = ny;
            facing = nfacing;
            ++steps;

            if (grid[y][x] == END_MARKER) { // we are done here, write the cost and exit this recursion.
                target.cost = steps;
                break;
            }

            // Intersection square logic.
            if (grid[y][x] != PATH_MARKER) {
                auto [interX, interY, _] = successor(x, y, facing); // we are now on the intersection
                ++steps;
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

                    // We have to add a successor to this block, but does it exist yet?
                    int _x = px; int _y = py; // openMP can't capture structured bindings. And it won't compile even though there is no OMP?
                    auto blockToMakeIter = std::find_if(cache.begin(), cache.end(), [_x, _y](auto& c){
                        return c->startX == _x && c->startY == _y;
                    });
                    if (blockToMakeIter != cache.end()) {
                        target.successors.emplace_back(*blockToMakeIter);
                    } else {
                        auto iteratorToNewBlock = calcBlock(px, py, possibleFacing, cache);
                        target.successors.emplace_back(*iteratorToNewBlock);
                    }
                }

                break; // we are done with this block now. We found the intersection square, appended blocks. No more walking fwd to do.
            }
        } while (grid[y][x] != END_MARKER);

        // According to documentation, no iterators are invalidated by calls to emplace of std::set so this should be fine.
        return iter;
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

    int numberOfPaths(int x, int y, std::array<std::pair<int,int>, 4>& paths) const {
        const std::array<std::pair<int,int>, 4> adjacent {{ {x-1,y}, {x+1,y}, {x,y+1}, {x,y-1} }};
        int c = 0;
        for (auto [nx, ny] : adjacent) {
            if (grid[ny][nx] != WALL_MARKER) {
                paths[c] = {nx, ny};
                c++;
            }
        }

        return c;
    }
};

} // namespace

#undef DAY