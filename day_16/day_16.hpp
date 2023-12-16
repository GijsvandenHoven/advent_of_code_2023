#pragma once

#include <iostream>
#include <omp.h>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 16

NAMESPACE_DEF(DAY) {

enum class Direction : uint8_t {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

enum class Gizmo : uint8_t {
    VERTICAL_SPLITTER = 0,
    HORIZONTAL_SPLITTER,
    TL_BR_REFLECTOR,
    BL_TR_REFLECTOR
};

class DirectionMap : public std::map<std::pair<int, int>, std::set<Direction>> {
    int maxX;
    int maxY;

public:
    DirectionMap(int sizeX, int sizeY) : maxX(sizeX), maxY(sizeY) {  }

    // returns whether a new item was added.
    bool add(int x, int y, Direction d) {
        if (x < 0 || x >= maxX) return false;
        if (y < 0 || y >= maxY) return false;

        auto p = std::make_pair(x, y);
        auto iter = this->find(p);
        if (iter == this->end()) {
            std::set<Direction> dset;
            dset.emplace(d);
            this->emplace(p, std::move(dset));
            return true;
        } else { // set already exists at this xy.
            auto [i, b] = iter->second.emplace(d);
            return b;
        }
    }
};

class GizmoMap : public std::map<std::pair<int,int>, Gizmo> {
public:
    [[nodiscard]] std::pair<bool, const_iterator> seek(int x, int y) const {
        auto iter = this->find(std::make_pair(x,y));
        return std::make_pair(iter != this->end(), iter);
    }
};

std::ostream& operator<<(std::ostream& os, const Direction& d) {
    switch(d) { case Direction::DOWN: os << "DOWN"; break; case Direction::UP: os << "UP"; break; case Direction::LEFT: os << "LEFT"; break; case Direction::RIGHT: os << "RIGHT"; break; default: os << "?"; break; }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Gizmo& g) {
    switch(g) { case Gizmo::VERTICAL_SPLITTER: os << '|'; break; case Gizmo::HORIZONTAL_SPLITTER: os << '-'; break; case Gizmo::TL_BR_REFLECTOR: os << '\\'; break; case Gizmo::BL_TR_REFLECTOR: os << '/'; break; default: os << '?'; break; }
    return os;
}

std::ostream& operator<<(std::ostream& os, const GizmoMap& g) {
    os << "GizmoMap {\n";
    for (auto& [xy, giz] : g) {
        os << "\t" << xy.first << ", " << xy.second << ": " << giz << "\n";
    }
    os << "}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        int y = 0;
        int x = 0;
        int c = 0;
        while (c != EOF) {
            c = input.get();
            switch(c) {
                default: break;
                case EOF:
                    XYSIZE.first = x;
                    XYSIZE.second = y + 1;
                    continue;
                case '\n':
                    y++;
                    x = 0;
                    continue;
                case '-':
                    map.emplace(std::make_pair(x, y), Gizmo::HORIZONTAL_SPLITTER);
                    break;
                case '|':
                    map.emplace(std::make_pair(x, y), Gizmo::VERTICAL_SPLITTER);
                    break;
                case '\\':
                    map.emplace(std::make_pair(x, y), Gizmo::TL_BR_REFLECTOR);
                    break;
                case '/':
                    map.emplace(std::make_pair(x, y), Gizmo::BL_TR_REFLECTOR);
                    break;
            }
            x++;
        }

    }

    void v1() const override {
        auto [sizX, sizY] = XYSIZE;
        DirectionMap coverage(sizX, sizY);
        buildCoverage(coverage, 0, 0, Direction::RIGHT);
        reportSolution(coverage.size());
    }

    void v2() const override {
        int max = 0;

        auto updateMax = [&max, this](int x, int y, Direction d) {
            auto [sizX, sizY] = XYSIZE;
            DirectionMap coverage(sizX, sizY);
            buildCoverage(coverage, x, y, d);

#pragma omp critical
            {
                max = std::max(max, static_cast<int>(coverage.size()));
            }
        };

#pragma omp parallel for schedule(static) default(none) shared(updateMax)
        for (int i = 0; i < XYSIZE.first; ++i) {
            updateMax(i, 0, Direction::DOWN);
            updateMax(i, XYSIZE.second - 1, Direction::UP);
        }

        std::cout << "i have " << omp_get_max_threads() << " threads\n";
#pragma omp parallel for schedule(static) default(none) shared(updateMax, std::cout)
        for (int i = 0; i < XYSIZE.second; ++i) {
            std::cout << "thread " << omp_get_thread_num() << "\n";
            updateMax(0, i, Direction::RIGHT);
            updateMax(XYSIZE.first - 1, i, Direction::LEFT);
        }

        reportSolution(max);
    }

    void parseBenchReset() override {
        map.clear();
    }

private:
    GizmoMap map;
    std::pair<int, int> XYSIZE;

    void buildCoverage(DirectionMap& coverage, int x, int y, Direction d) const {
        // try to mark this location & direction as visited in coverage.
        auto newCoverage = coverage.add(x, y, d);
        if (! newCoverage) { // recursion base case: if we are going OOB or this coverage is nothing new.
            return;
        }

        auto [exists, item] = map.seek(x, y);
        if (exists) {
            handleGizmo(coverage, item->second, x, y, d);
        } else {
            auto [nx, ny] = calculateNext(x, y, d);
            buildCoverage(coverage, nx, ny, d);
        }
    }

    // This probably could have been a constexpr std::map<Gizmo, std::map<Direction, std::pair<int,int>>>;
    void handleGizmo(DirectionMap& coverage, const Gizmo& g, int x, int y, Direction d) const {
        switch (g) {
            default: throw std::logic_error("Unknown Gizmo in handleGizmo");
            case Gizmo::TL_BR_REFLECTOR:
                switch (d) {
                    default: throw std::logic_error("Unknown Direction in handleGizmo");
                    case Direction::UP:     return buildCoverage(coverage, x-1, y, Direction::LEFT);
                    case Direction::DOWN:   return buildCoverage(coverage, x+1, y, Direction::RIGHT);
                    case Direction::LEFT:   return buildCoverage(coverage, x,y-1, Direction::UP);
                    case Direction::RIGHT:  return buildCoverage(coverage, x,y+1, Direction::DOWN);
                }
            case Gizmo::BL_TR_REFLECTOR:
                switch (d) {
                    default: throw std::logic_error("Unknown Direction in handleGizmo");
                    case Direction::UP:     return buildCoverage(coverage, x+1, y, Direction::RIGHT);
                    case Direction::DOWN:   return buildCoverage(coverage, x-1, y, Direction::LEFT);
                    case Direction::LEFT:   return buildCoverage(coverage, x,y+1, Direction::DOWN);
                    case Direction::RIGHT:  return buildCoverage(coverage, x,y-1, Direction::UP);
                }
            case Gizmo::HORIZONTAL_SPLITTER:
                switch (d) {
                    default: throw std::logic_error("Unknown Direction in handleGizmo");
                    case Direction::LEFT:   return buildCoverage(coverage, x-1,y, d); // 'just keep going left'
                    case Direction::RIGHT:  return buildCoverage(coverage, x+1,y, d); // 'just keep going right'

                    case Direction::DOWN: [[fallthrough]];
                    case Direction::UP:
                        buildCoverage(coverage, x-1, y, Direction::LEFT);
                        buildCoverage(coverage, x+1, y, Direction::RIGHT);
                        return;
                }
            case Gizmo::VERTICAL_SPLITTER:
                switch (d) {
                    default: throw std::logic_error("Unknown Direction in handleGizmo");
                    case Direction::UP: return buildCoverage(coverage, x,y-1, d); // 'just keep going up'
                    case Direction::DOWN: return buildCoverage(coverage, x,y+1, d); // 'just keep going 'down'

                    case Direction::LEFT: [[fallthrough]];
                    case Direction::RIGHT:
                        buildCoverage(coverage, x, y-1, Direction::UP);
                        buildCoverage(coverage, x, y+1, Direction::DOWN);
                        return;
                }
        }
    }

    static std::pair<int, int> calculateNext(int x, int y, Direction d) {
        switch (d) {
            default: throw std::logic_error("Unknown direction");
            case Direction::UP:     return std::make_pair(x, y-1);
            case Direction::DOWN:   return std::make_pair(x, y+1);
            case Direction::LEFT:   return std::make_pair(x-1, y);
            case Direction::RIGHT:  return std::make_pair(x+1, y);
        }
    }
};

} // namespace

#undef DAY
#undef PART_2_USE_OPENMP