#pragma once

#include <iostream>
#include <queue>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 10

namespace Day10 {

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
    PipeSegment() { ; } // Intentionally uninitialized memory. NOLINT(*-pro-type-member-init)

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

template<typename T>
class Maze : public std::vector<std::vector<T>> {
public:
    [[nodiscard]] const T &at(int x, int y) const { return this->operator[](y)[x]; }
    [[nodiscard]] T &at(int x, int y) { return this->operator[](y)[x]; }

    [[nodiscard]] std::pair<bool, T*> safeAt(int x, int y) {
        if (y < 0 || y >= this->size()) {
            return std::make_pair(false, nullptr);
        }
        if (x < 0 || x >= this->operator[](y).size()) {
            return std::make_pair(false, nullptr);
        }

        return std::make_pair(true, &at(x, y));
    }
};

struct SearchablePipeSegment : public PipeSegment {
    bool visited = false;

    explicit SearchablePipeSegment(PipeType &&p) : PipeSegment(std::forward<PipeType>(p)) {}
    SearchablePipeSegment() : PipeSegment() {}
};

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

std::ostream& operator<<(std::ostream& os, const SearchablePipeSegment& p) {
    switch (p.type()) {
        case PipeType::NONE:            os << (p.visited ? 'X' : '.'); break;
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

template<typename T>
std::ostream& operator<<(std::ostream& os, const Maze<T>& m) {
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

        auto &S = maze.at(SX, SY);
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
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
                default: throw std::logic_error("Impossible travel direction given by tryTravel().");
#pragma clang diagnostic pop
            }
            steps++;
        } while (! (x == startX && y == startY));

        reportSolution(steps / 2);
    }

    void v2() const override {
        Maze<PipeSegment> explodedMaze;
        explodeMaze(explodedMaze);

        Maze<SearchablePipeSegment> BFSWorksheet; // fully blank maze, to be painted by the loop detection process.

        BFSWorksheet.reserve(explodedMaze.size());
        for (auto& row : explodedMaze) {
            BFSWorksheet.emplace_back();
            for (int i = 0; i < row.size(); ++i) {
                BFSWorksheet.back().emplace_back(PipeType::NONE);
            }
        }

        auto BFSWorksheetOuter = BFSWorksheet; // copy
        auto BFSWorksheetInner = std::move(BFSWorksheet); // and move.

        doDoubleLoopCheckOnExplodedMaze(explodedMaze, BFSWorksheetOuter, BFSWorksheetInner);
        BFSReachableTiles(BFSWorksheetOuter);
        BFSReachableTiles(BFSWorksheetInner);

        // for every 'virtual' tile, the 2x2 block representing one tile in the exploded maze.
        // Count it as explored if _either_ of the worksheets could reach it.
        // Conversely, if neither worksheet could reach it, it is unreachable.
        Maze<SearchablePipeSegment> shrunkDown;
        for (int i = 0; i < explodedMaze.size(); i+=2) {
            shrunkDown.emplace_back();
            for (int j = 0; j < explodedMaze[i].size(); j+=2) {
                auto& t1 = BFSWorksheetOuter[i][j];
                auto& t2 = BFSWorksheetInner[i][j];
                bool reachable = (t1.visited || t2.visited) && t1.type() == PipeType::NONE && t2.type() == PipeType::NONE;

                // what was this tile?
                std::array<SearchablePipeSegment, 4> virtualTile {{
                    { BFSWorksheetOuter[i][j] },
                    { BFSWorksheetOuter[i][j+1] },
                    { BFSWorksheetOuter[i+1][j] },
                    { BFSWorksheetOuter[i+1][j+1] },
                }};

                int remarkability = 0;
                PipeType mostRemarkableItem = PipeType::NONE;
                for (auto& v : virtualTile) {
                    switch(v.type()) {
                        case PipeType::NONE:
                            break;
                        case PipeType::UPDOWN:
                        case PipeType::LEFTRIGHT:
                            if (remarkability < 1) {
                                mostRemarkableItem = v.type();
                                remarkability = 1;
                            }
                            break;
                        case PipeType::LEFTUP:
                        case PipeType::UPRIGHT:
                        case PipeType::RIGHTDOWN:
                        case PipeType::DOWNLEFT:
                            mostRemarkableItem = v.type();
                            remarkability = 2;
                            break;
                        default: throw std::logic_error("Unknown pipe type in maze reconstruction");
                    }
                }

                shrunkDown.back().emplace_back(std::move(mostRemarkableItem)); // NOLINT(*-move-const-arg) - compile error if not rvalue ref.
                shrunkDown.back().back().visited = reachable;
            }
        }

        int enclosedTiles = 0;
        for (int i = 1; i < shrunkDown.size() - 1; ++i) {
            for (int j = 1; j < shrunkDown[i].size() - 1; ++j) {
                auto& item = shrunkDown[i][j];
                if (item.type() == PipeType::NONE && ! item.visited) {
                    enclosedTiles ++;
                }
            }
        }

        reportSolution(enclosedTiles);
    }

    void parseBenchReset() override {
        startX = 0;
        startY = 0;
        maze.clear();
    }

private:
    Maze<PipeSegment> maze;
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
    static void BFSReachableTiles(Maze<SearchablePipeSegment>& worksheet, int startX = 0, int startY = 0) {
#pragma clang diagnostic pop
        std::queue<std::pair<int, int>> coordinateQueue;
        coordinateQueue.emplace(startX, startY);

        // Start by marking the starting node as visited.
        worksheet.at(startX, startY).visited = true;
        // BFS algo, instead of "visted set" we use the mutable worksheet to do this job.
        while (! coordinateQueue.empty()) {
            auto [x, y] = coordinateQueue.front();
            coordinateQueue.pop();

            const std::array<std::pair<int,int>, 4> neighbours {{ {x-1,y}, {x+1,y}, {x,y-1}, {x,y+1} }};
            for (auto [nx, ny] : neighbours) {
                auto [safe, ptr] = worksheet.safeAt(nx, ny);

                if (safe && !ptr->visited && ptr->type() == PipeType::NONE) {
                    ptr->visited = true;
                    coordinateQueue.emplace(nx, ny);
                }
            }
        }
    }

    // 'blow up' a maze to 2x its size. PipeSegments are extended appropriately, for example:
    // 'F'     'J'    '|'
    // becomes:
    //  'F-'   'J|'   '||'
    //  '|F'   '-J'   '||'
    void explodeMaze(Maze<PipeSegment>& worksheet) const {
        worksheet.resize(maze.size() * 2);
        for (int i = 0; i < worksheet.size(); ++i) {
            auto& row = worksheet[i];
            row.resize((maze[i/2].size()) * 2);
        }

        for (int i = 0; i < maze.size(); ++i) {
            for (int j = 0; j < maze[i].size(); ++j) {
                int p = i * 2;
                int q = j * 2;
                auto& toExplode = maze[i][j];

                switch(toExplode.type()) {
                    default:
                        throw std::logic_error("Cannot explode PipeSegment of type " + std::to_string(static_cast<PipeTypeEnumType>(toExplode.type())));
                    case PipeType::NONE:
                        worksheet[p][q] = PipeSegment(PipeType::NONE);
                        worksheet[p][q+1] = PipeSegment(PipeType::NONE);
                        worksheet[p+1][q] = PipeSegment(PipeType::NONE);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::NONE);
                        break;
                    case PipeType::LEFTRIGHT:
                        worksheet[p][q] = PipeSegment(PipeType::LEFTRIGHT);
                        worksheet[p][q+1] = PipeSegment(PipeType::LEFTRIGHT);
                        worksheet[p+1][q] = PipeSegment(PipeType::LEFTRIGHT);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::LEFTRIGHT);
                        break;
                    case PipeType::UPDOWN:
                        worksheet[p][q] = PipeSegment(PipeType::UPDOWN);
                        worksheet[p][q+1] = PipeSegment(PipeType::UPDOWN);
                        worksheet[p+1][q] = PipeSegment(PipeType::UPDOWN);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::UPDOWN);
                        break;
                    case PipeType::LEFTUP:
                        worksheet[p][q] = PipeSegment(PipeType::LEFTUP);
                        worksheet[p][q+1] = PipeSegment(PipeType::UPDOWN);
                        worksheet[p+1][q] = PipeSegment(PipeType::LEFTRIGHT);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::LEFTUP);
                        break;
                    case PipeType::UPRIGHT:
                        worksheet[p][q] = PipeSegment(PipeType::UPDOWN);
                        worksheet[p][q+1] = PipeSegment(PipeType::UPRIGHT);
                        worksheet[p+1][q] = PipeSegment(PipeType::UPRIGHT);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::LEFTRIGHT);
                        break;
                    case PipeType::RIGHTDOWN:
                        worksheet[p][q] = PipeSegment(PipeType::RIGHTDOWN);
                        worksheet[p][q+1] = PipeSegment(PipeType::LEFTRIGHT);
                        worksheet[p+1][q] = PipeSegment(PipeType::UPDOWN);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::RIGHTDOWN);
                        break;
                    case PipeType::DOWNLEFT:
                        worksheet[p][q] = PipeSegment(PipeType::LEFTRIGHT);
                        worksheet[p][q+1] = PipeSegment(PipeType::DOWNLEFT);
                        worksheet[p+1][q] = PipeSegment(PipeType::DOWNLEFT);
                        worksheet[p+1][q+1] = PipeSegment(PipeType::UPDOWN);
                        break;
                }
            }
        }
    }

    void doDoubleLoopCheckOnExplodedMaze(const Maze<PipeSegment>& explodedMaze, Maze<SearchablePipeSegment>& outer, Maze<SearchablePipeSegment>& inner) const {
        int outerX;
        int innerX;
        int outerY;
        int innerY;
        { // find which of the pieces in a 4 block segment is not connected to this.
            outerX = startX * 2;
            outerY = startY * 2;
            auto& tile = explodedMaze.at(outerX, outerY);
            switch(tile.type()) {
                case PipeType::UPDOWN:
                    innerX = outerX + 1;
                    innerY = outerY;
                    break;
                case PipeType::LEFTRIGHT:
                    innerX = outerX;
                    innerY = outerY + 1;
                    break;
                case PipeType::DOWNLEFT:
                    throw std::logic_error("7 in top left of exploded maze");
                case PipeType::RIGHTDOWN:
                case PipeType::LEFTUP:
                    innerX = outerX + 1;
                    innerY = outerY + 1; // really only need to inc one of these but let's start on the same symbol just cause.
                    break;
                case PipeType::UPRIGHT:
                    throw std::logic_error("L in top left of exploded maze");
                default:
                    throw std::logic_error("Unknown PipeType for start tile.");
            }
        }

        {
            int x = outerX;
            int y = outerY;
            auto cameFrom = Direction::NONE;
            do {
                auto& place = explodedMaze.at(x, y);
                outer.at(x, y) = SearchablePipeSegment{place.type()};
                cameFrom = tryTravel(place, cameFrom);
                switch (cameFrom) {
                    case Direction::UP:     y--; cameFrom = Direction::DOWN;    break;
                    case Direction::DOWN:   y++; cameFrom = Direction::UP;      break;
                    case Direction::LEFT:   x--; cameFrom = Direction::RIGHT;   break;
                    case Direction::RIGHT:  x++; cameFrom = Direction::LEFT;    break;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
                    default: throw std::logic_error("Impossible travel direction given by tryTravel().");
#pragma clang diagnostic pop
                }
            } while (! (x == outerX && y == outerY));
        }
        {
            int x = innerX;
            int y = innerY;
            auto cameFrom = Direction::NONE;
            do {
                auto& place = explodedMaze.at(x, y);
                inner.at(x, y) = SearchablePipeSegment{place.type()};
                cameFrom = tryTravel(place, cameFrom);
                switch (cameFrom) {
                    case Direction::UP:     y--; cameFrom = Direction::DOWN;    break;
                    case Direction::DOWN:   y++; cameFrom = Direction::UP;      break;
                    case Direction::LEFT:   x--; cameFrom = Direction::RIGHT;   break;
                    case Direction::RIGHT:  x++; cameFrom = Direction::LEFT;    break;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
                    default: throw std::logic_error("Impossible travel direction given by tryTravel().");
#pragma clang diagnostic pop
                }
            } while (! (x == innerX && y == innerY));
        }
    }
};

} // namespace

#undef DAY