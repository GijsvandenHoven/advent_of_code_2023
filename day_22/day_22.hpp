#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 22

NAMESPACE_DEF(DAY) {

struct Point;
std::istream& operator>>(std::istream& s, Point& p);

struct Point { int x; int y; int z; };

struct Cube {

    Point begin{};
    Point end{};
    int id;

    explicit Cube(const std::string& from, int id) : id(id) {
        std::istringstream s(from);

        s >> begin;
        s.ignore(1);
        s >> end;

        if (begin.x > end.x || begin.y > end.y || begin.z > end.z) { // this warning is bogus.
            throw std::logic_error("Cube: begin Point should be less or equal to end Point.");
        }
    }
};

std::istream& operator>>(std::istream& s, Point& p) {
    s >> p.x; s.ignore(1);
    s >> p.y; s.ignore(1);
    s >> p.z;

    return s;
}

std::ostream& operator<<(std::ostream& os, const Cube& c) {
    os << "Cube ("<< c.id <<") {\n";
    os << "\tbegin: { " << c.begin.x << ", " << c.begin.y << ", " << c.begin.z << " }\n";
    os << "\tend: { " << c.end.x << ", " << c.end.y << ", " << c.end.z << " }\n";
    os << "}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        int i = 0;
        while(std::getline(input, line)) {
            cubes.emplace_back(line, i++);
        }

        // sort cubes by (bottm) Z ascending.
        std::sort(cubes.begin(), cubes.end(), [](auto& l, auto& r){
            return l.begin.z < r.begin.z;
        });

        int minX = std::numeric_limits<int>::max();
        int minY = std::numeric_limits<int>::max();

        int maxX = std::numeric_limits<int>::min();
        int maxY = std::numeric_limits<int>::min();

        for (auto& c : cubes) {
            auto [maybeMinX, maybeMinY, _z1] = c.begin; // by definition c.end >= c.begin, so c.begin values are lowest
            auto [maybeMaxX, maybeMaxY, _z2] = c.end;

            if (maybeMinX < minX) { minX = maybeMinX; }
            if (maybeMinY < minY) { minY = maybeMinY; }

            if (maybeMaxX > maxX) { maxX = maybeMaxX; }
            if (maybeMaxY > maxY) { maxY = maybeMaxY; }
        }

        if (minX < 0 || minY < 0) {
            throw std::logic_error("No, i'm not going to deal with that :)");
        }

        dimensions = { minX, minY, maxX, maxY };
    }

    void v1() const override {
        auto [minX, minY, maxX, maxY] = dimensions;

        int xDomain = (maxX + 1); // do not subtract from min, we are 0-based indexing vectors with this.
        int yDomain = (maxY + 1);
        // For every x,y how high (z) the floor is.
        // Starts as all 0s (no cubes), as cubes fall, floorHeights for their locations change.
        std::vector<std::vector<int>> floorHeights;
        for (int y = 0; y < yDomain; ++y) {
            floorHeights.emplace_back(xDomain, 0);
        }

        std::cout << "My floor height map is " << floorHeights.size() << " by " << floorHeights.back().size() << "\n";

        auto printHeightMap = [&floorHeights](){
            for (auto& row : floorHeights) {
                for (auto& i : row) {
                    std::cout << i;
                }
                std::cout << "\n";
            }
        };

        // References for each x,y the topmost cube occupying that space.
        // Not the cubes at the highest slice, e.g. 0,0 could have a 10 tall cube and 1,1 has a 1 tall cube.
        // Use floorHeights for this.
        std::map<std::pair<int, int>, const Cube *> occupancy;
        std::map<const Cube *, std::set<const Cube *>> connections;
        for (auto& cube : cubes) { // put an empty list on each to get started.
            connections.emplace(&cube, std::set<const Cube *>());
        }

        for (auto& c : cubes) {
            std::cout << "Make fall: " << c << "\n";

            int cubeHeight = c.end.z - c.begin.z + 1;
            // This cubes height shall be the maximum of the xy surface it is above.
            int maxHeight = std::numeric_limits<int>::min();
            for (int j = c.begin.y; j <= c.end.y; ++j) {
                for (int i = c.begin.x; i <= c.end.x; ++i) {
                    auto& heightHere = floorHeights[j][i];
                    if (heightHere > maxHeight) {
                        maxHeight = heightHere;
                    }
                }
            }

            for (int j = c.begin.y; j <= c.end.y; ++j) {
                for (int i = c.begin.x; i <= c.end.x; ++i) {
                    // test touch detection.
                    if (floorHeights[j][i] == maxHeight) {
                         // we are going to rest upon whatever cube is here.
                         auto iter = occupancy.find({i,j});
                         if (iter != occupancy.end()) {
                             // std::cout << "\tcube shall rest on id " << iter->second->id << "\n";
                             connections[iter->second].emplace(&c);
                         }
                    }

                    floorHeights[j][i] = maxHeight + cubeHeight; // this floor tile is cubeHeight higher now.
                    occupancy.insert_or_assign(std::make_pair(i, j), &c); // and at this coord the top cube shall be this.
                }
            }

            std::cout << "After fall this is the height map\n";
            printHeightMap();
        }

        // bucket connection counts to speed up the next part.
        std::vector<int> supportCount(cubes.size(), 0);

        std::cout << "CONNECTIONS MAP\n";
        for (auto& [ptr, list] : connections) {
            std::cout << "\t" << ptr->id << " Supports:\n";
            for (auto p : list) {
                 std::cout << "\t\t" << p->id << "\n";
                supportCount[p->id]++;
            }
        }

        std::cout << "SUPPORT COUNT\n";
        for (int i = 0; i < supportCount.size(); ++i) {
            std::cout << "\tcube " << i << " is supported " << supportCount[i] << " times.\n";
        }

        // A cube 'c' can be safely removed if:
        //      For every connected cube 'd', there exists a cube 'e', 'c' != 'e', such that 'd' is connected to 'e'.
        //      This is given by the supportCount bucket, if it is greater than 1, then we know that 'e' exists (But not who 'e' is)
        int count = 0;
        for (auto& [ptr, list] : connections) {
            bool safe = true;
            for (auto p : list) {
                if (supportCount[p->id] < 2) {
                    safe = false;
                    break;
                }
            }

            if (safe) {
                std::cout << "cube " << ptr->id << " is safe\n";
                ++count;
            }
        }

        reportSolution(count);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        cubes.clear();
    }

private:
    std::vector<Cube> cubes;
    std::tuple<int,int,int,int> dimensions; // domain of the cubes in X,Y space, from "top left" to "bottom" coordinate pair.
};

} // namespace

#undef DAY