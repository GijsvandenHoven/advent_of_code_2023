#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 21

NAMESPACE_DEF(DAY) {

struct Tile {
    bool occupied;
};

struct PathfindingTile : public Tile {
    bool visited;
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
    return t.occupied ? os << '#' : ( t.visited ? os << 'O' : os << '.');
}

template <typename V>
std::ostream& operator<<(std::ostream& os, const Garden<V>& g) {
    auto [sx, sy] = g.getStart();
    int y = 0;
    for (auto& row : g) {
        int x = 0;
        for (auto& t : row) {
            if (y == sy && x == sx) {
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
        int result = leapFrogDFS(copy, copy.startX, copy.startY, 2);
        reportSolution(result);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
    BlueprintGarden grid;

    static void BFS(MutableGarden& subject) {

    }

    [[nodiscard]] static int leapFrogDFS(MutableGarden& subject, int x, int y, int length = 64) {
        if (length % 2 != 0) throw std::logic_error("Can't be bothered to support this.");

        subject.at(x, y).visited = true;

        if (length < 2) return 1; // cannot do any more leaps, but we did visit here.

        bool canGoUp = (! subject.at(x, y-1).occupied) && (! subject.at(x, y-2).occupied) && (! subject.at(x,y-2).visited);
        bool canGoDo = (! subject.at(x, y+1).occupied) && (! subject.at(x, y+2).occupied) && (! subject.at(x,y+2).visited);
        bool canGoLe = (! subject.at(x-1, y).occupied) && (! subject.at(x-2, y).occupied) && (! subject.at(x-2,y).visited);
        bool canGoRi = (! subject.at(x+1, y).occupied) && (! subject.at(x+2, y).occupied) && (! subject.at(x+2,y).visited);

        std::cout << "up " << canGoUp << " do " << canGoDo << " le " << canGoLe << " ri " << canGoRi << "\n";

        int total = 1; // we can visit ourselves from here.

        if (canGoUp) { // and if we can go up we can visit these spots
            total += leapFrogDFS(subject, x, y-2, length - 2);
        }
        if (canGoDo) { // and these if we can go down.
            total += leapFrogDFS(subject, x, y+2, length - 2);
        }
        if (canGoLe) {
            total += leapFrogDFS(subject, x-2, y, length - 2);
        }
        if (canGoRi) {
            total += leapFrogDFS(subject, x+2, y, length - 2);
        }

        return total;
    }
};

} // namespace

#undef DAY