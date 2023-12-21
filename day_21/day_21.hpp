#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 21

NAMESPACE_DEF(DAY) {

struct Tile {
    bool occupied;
};

constexpr int UNVISITED_DIST = std::numeric_limits<int>::min();
struct PathfindingTile : public Tile {
    int dist = UNVISITED_DIST;
};

template <typename T>
requires std::is_base_of_v<Tile, T>
class Garden : private std::vector<std::vector<T>> {
    template <typename V>
    friend std::ostream& operator<<(std::ostream& os, const Garden<V>& g);
public:
    int startX = -1;
    int startY = -1;

    [[nodiscard]] T& at(int x, int y) { return this->operator[](y)[x]; }

    [[nodiscard]] std::tuple<bool, int, int> up(int x, int y) {
        int newY = y - 1;
        if (newY < 0) return {false, 0, 0};

        return {true, x, newY};
    }

    [[nodiscard]] std::tuple<bool, int, int> down(int x, int y) {
        int newY = y + 1;
        if (newY >= this->size()) return {false, 0, 0};

        return {true, x, newY};
    }

    [[nodiscard]] std::tuple<bool, int, int> left(int x, int y) {
        int newX = x - 1;
        if (newX < 0) return {false, 0, 0};

        return {true, newX, y};
    }

    [[nodiscard]] std::tuple<bool, int, int> right(int x, int y) {
        int newX = x + 1;
        if (this->size() == 0 || newX > this->back().size()) return {false, 0, 0};

        return {true, newX, y};
    }

    void addRow(const std::string& row) {
        std::istringstream s(row);
        this->emplace_back(); // empty vec to the back, to be filled
        this->back().reserve(row.size());

        int c;
        int i = 0;
        while ((c = s.get()) != EOF) {
            this->back().emplace_back(c == '#');

            if (c == 'S') {
                startX = i;
                startY = static_cast<int>(this->size() - 1);
            }

            i++;
        }
    }

    void addRow(std::vector<T>&& r) {
        this->emplace_back(r);
    }

    void mutableCopy(Garden<PathfindingTile>& g) const {
        g.startX = startX;
        g.startY = startY;
        for (auto& r : *this) {
            std::vector<PathfindingTile> copy;
            for (auto& t : r) {
                copy.emplace_back(PathfindingTile(t, false));
            }
            g.addRow(std::move(copy));
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Tile& t) {
    return t.occupied ? os << '#' : os << '.';
}

std::ostream& operator<<(std::ostream& os, const PathfindingTile& t) {
    return t.occupied ? os << " # " : ( t.dist == UNVISITED_DIST ? os << " . " : os << t.dist);
}

template <typename V>
std::ostream& operator<<(std::ostream& os, const Garden<V>& g) {
    int y = 0;
    for (auto& row : g) {
        int x = 0;
        for (auto& t : row) {
            if (y == g.startY && x == g.startX) {
                os << 'S';
            } else {
                os << t;
            }
            x++;
        }
        y++;
        os << '\n';
    }
    return os;
}

using BlueprintGarden = Garden<Tile>;
using MutableGarden = Garden<PathfindingTile>;

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            grid.addRow(line);
        }

        if (grid.startX < 0 || grid.startY < 0) throw std::logic_error("Start position was not set.");
    }

    void v1() const override {
        MutableGarden copy;
        grid.mutableCopy(copy);
        BFS(copy);
        std::cout << copy << "\n";
        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
    BlueprintGarden grid;

    static void BFS(MutableGarden& subject) {

        std::queue<std::tuple<int,int,int>> bfs; // x,y,dist.
        bfs.emplace(subject.startX, subject.startY, 0);

        while (! bfs.empty()) {
            auto [x, y, d] = bfs.front();
            bfs.pop();

            subject.at(x, y).dist = d;

            std::array<std::tuple<bool,int,int>,4> next = { subject.up(x,y), subject.down(x,y), subject.left(x,y), subject.right(x,y) };

            for (auto [ok, nx, ny] : next) {
                if (! ok) continue;

                auto& maybe = subject.at(nx, ny);
                if (! maybe.occupied && maybe.dist == UNVISITED_DIST) {
                    bfs.emplace(nx, ny, d+1);
                }
            }
        }
    }
};

} // namespace

#undef DAY