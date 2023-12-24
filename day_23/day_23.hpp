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
        std::shared_ptr<Block> dontCare(nullptr);
        auto iterToStart = calcBlock(x, y + 1, Direction::SOUTH, cache, dontCare);
        int longest = calcLongestPath(**iterToStart);

        reportSolution(longest);
    }

    void v2() const override {
        auto [x, y] = start;

        // reduce the graph by collapsing the labyrinth sections into "blocks" with appropriate length.
        BlockCache cache;
        std::shared_ptr<Block> finalBlock(nullptr);
        auto iterToStart = calcBlock(x, y + 1, Direction::SOUTH, cache, finalBlock);

        // Extend connections to go backwards
        makeBidirectional(*iterToStart);
        std::vector<const Block *> visited;
        auto [endReached, solution] = calcLongestPathWithCircularDependencies(**iterToStart, *finalBlock, visited);
        if (! endReached) {
            throw std::logic_error("End could not be reached from start??");
        }
        std::cout << solution << "\n";

        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
    std::vector<std::string> grid;
    std::pair<int,int> start;

    static std::pair<bool, int> calcLongestPathWithCircularDependencies(const Block& here, const Block& end, std::vector<const Block*>& visited) {
//        std::cout << "1st has these cons: " << first.successors.size() << "\n";
//        for (auto& s : first.successors) {
//            std::cout << "\t" << s->startX << ", " << s->startY << " with further sucs \n";
//            for (auto& ss : s->successors) {
//                std::cout << "\t\t" << ss->startX << ", " << ss->startY << " with further further sucs \n";
//                for (auto& sss : ss->successors) {
//                    std::cout << "\t\t\t" << sss->startX << ", " << sss->startY << ".\n";
//                }
//            }
//        }

        // exhaustively depth-first search to find the max size path.
        std::cout << "Recursively considering " << here.startX << ", " << here.startY << "\n";

        if (&here != &end) {
            visited.push_back(&here);

            bool endIsInThisPath = false;
            int maxOfChoice = 0;
            for (auto& s : here.successors) {
                auto iterToVisitVec = std::find(visited.begin(), visited.end(), s.get());
                if (iterToVisitVec != visited.end()) {
                    continue; // We have already been here on this path, so we cannot use it again.
                }

                auto [endReaching, maxWithThisChoice] = calcLongestPathWithCircularDependencies(*s, end, visited);

                if (endReaching) {
                    endIsInThisPath = true;
                    maxOfChoice = std::max(maxOfChoice, maxWithThisChoice);
                }
            }

            visited.pop_back();
            return {endIsInThisPath, here.cost + maxOfChoice};
        } else {
            return {true, here.cost};
        }
    }

    // NP-hard :) that's why we collapsed to blocks.
    static int calcLongestPath(const Block& first) {
        int max = 0;
        for (auto& s : first.successors) {
            max = std::max(max, calcLongestPath(*s));
        }

        return first.cost + max;
    }

    static void makeBidirectional(const std::shared_ptr<Block>& b) {
        for (auto& suc : b->successors) {
            auto iter = std::find(suc->successors.begin(), suc->successors.end(), b);

            if (iter == suc->successors.end()) {
                suc->successors.emplace_back(b);

                // picture a->b->c, first a->b->(c,a), then recursing on b,
                // a->b->(c,a), for c -> b
                // 0-size loop exit for c, then a will be considered again (it is now a successor of b)
                // the std::find will find 'a' in its list already and not do anything.
                makeBidirectional(suc);
            }
        }
    }

    /**
     * Walk to the successor incrementing a count until...
     * ...a direction marker is reached, at which point,
     * Recursively push onto the successor list calls to this function.
     * Care is taken only "legal" directions are walked, i.e. not passing through one-ways.
     */
    BlockCache::iterator calcBlock(const int x, const int y, const Direction facing, BlockCache& cache, std::shared_ptr<Block>& endBlock) const {
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
            std::array<std::pair<int,int>, 4> options;
            int count = numberOfPaths(cx, cy, options);

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
                auto potentialFacing = whatIsFacing(nx, ny, cx, cy);

                if (next != PATH_MARKER) {
                    pathToWalk = false;

                    if (next == END_MARKER) {
                        target.cost = steps+1;
                        endBlock = *iter; // we have to remember for the calling funciton what the last block is.
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
                            auto newBlock = calcBlock(nx, ny, potentialFacing, cache, endBlock);
                            target.successors.emplace_back(*newBlock);
                        } else {
                            target.successors.emplace_back(*iterToCache);
                        }

                    } else { // intersection square
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
                            auto newBlock = calcBlock(nx, ny, potentialFacing, cache, endBlock);
                            target.successors.emplace_back(*newBlock);
                        } else {
                            target.successors.emplace_back(*iterToCache);
                        }
                    }
                } else {
                    if (count != 2) throw std::logic_error("Path marker & Option inconsistency.");
                    // just go to the next spot.
                    cfacing = potentialFacing;
                    cx = nx;
                    cy = ny;
                }
            }
        }

        return iter;
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