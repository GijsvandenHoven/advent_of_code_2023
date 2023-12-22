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
        std::map<const Cube *, std::set<const Cube *>> cons;
        makeFallenBrickConnections(cons);

        // bucket connection counts to speed up the next part.
        std::vector<int> supportCount(cubes.size(), 0);

        for (auto& [ptr, list] : cons) {
            for (auto p : list) {
                supportCount[p->id]++;
            }
        }
        // A cube 'c' can be safely removed if:
        //      For every connected cube 'd', there exists a cube 'e', 'c' != 'e', such that 'd' is connected to 'e'.
        //      This is given by the supportCount bucket, if it is greater than 1, then we know that 'e' exists (But not who 'e' is)
        int count = 0;
        for (auto& [ptr, list] : cons) {
            bool safe = true;
            for (auto p : list) {
                if (supportCount[p->id] < 2) {
                    safe = false;
                    break;
                }
            }

            if (safe) {
                ++count;
            }
        }

        reportSolution(count);
    }

    void v2() const override {
        std::map<const Cube *, std::set<const Cube *>> cons;
        makeFallenBrickConnections(cons);

        // Where cons is "cube x supports this set of cubes",
        // This is "cube x is supported by this set of cubes".
        std::map<const Cube *, std::set<const Cube*>> inverseConnections;
        for (auto& [supporter, supported] : cons) {
            // by definition of emplace, only inserted if nothing exists here yet.
            // This is done to not have unsupported (= on the floor) cubes not present in inverse.
            // Although not having them doesn't have to matter because operator[] would insert an empty set for us.
            // So it is mostly about maintaining invariants.
            inverseConnections.emplace(supporter, std::set<const Cube *>());

            for (auto& c : supported) {
                inverseConnections[c].emplace(supporter);
            }
        }

        // Let's turn that O(log(n)) into O(1) by using cube IDs.
        auto makeLookupTable = [](auto& table, auto& connectionMap){
            table.clear();
            table.resize(connectionMap.size(), nullptr);
            for (auto& [cube, connections] : connectionMap) {
                if (cube->id < 0 || cube->id >= connectionMap.size()) {
                    throw std::logic_error("Indices should be contiguous from 0 to map size.");
                }

                // because there are no duplicate keys by defintion of map, and we throw exceptions for all but [size] IDs,
                // This vector should be exactly filled on every value at the end of the loop. (no double writes or 0 writes for any index)
                table[cube->id] = &connections;
            }
            if (table.end() != std::find(table.begin(), table.end(), nullptr)) {
                throw std::logic_error("This should be impossible.");
            }
        };

        // make a lookup table for both connections and inverse connections.
        std::vector<std::set<const Cube*> *> connectionLookup;
        std::vector<std::set<const Cube*> *> inverseConnectionLookup;
        makeLookupTable(connectionLookup, cons);
        makeLookupTable(inverseConnectionLookup, inverseConnections);

        int64_t fallSum = 0;
        for (auto& [cube, connections] : cons) {
            std::set<const Cube *> unstable;
            unstable.emplace(cube);

            std::queue<const Cube *> work;
            work.emplace(cube);

            while (! work.empty()) {
                auto * handle = work.front();
                work.pop();

                for (auto& destabilizing : *connectionLookup[handle->id]) {
                    if (unstable.contains(destabilizing)) {
                        continue;
                    }

                    // This cube becomes unstable, if none of its supporting nodes are stable.
                    // this bool default true would be wonky if a 0-supported cube (floor cube) is here,
                    // but it's from a connection lookup already so those types of cubes will never be considered.
                    bool willBecomeUnstable = true;
                    for (auto& relyingOn : *inverseConnectionLookup[destabilizing->id]) {
                        if (! unstable.contains(relyingOn)) {
                            willBecomeUnstable = false;
                            break;
                        }
                    }

                    if (willBecomeUnstable) {
                        unstable.emplace(destabilizing);
                        work.emplace(destabilizing);
                    }
                }
            }

            int unstables = static_cast<int>(unstable.size() - 1);
            fallSum += unstables;
        }

        reportSolution(fallSum);
    }

    void parseBenchReset() override {
        cubes.clear();
    }

private:
    std::vector<Cube> cubes;
    std::tuple<int,int,int,int> dimensions; // domain of the cubes in X,Y space, from "top left" to "bottom" coordinate pair.

    void makeFallenBrickConnections(std::map<const Cube *, std::set<const Cube *>>& connections) const {
        auto [minX, minY, maxX, maxY] = dimensions;

        int xDomain = (maxX + 1); // do not subtract from min, we are 0-based indexing vectors with this.
        int yDomain = (maxY + 1);
        // For every x,y how high (z) the floor is.
        // Starts as all 0s (no cubes), as cubes fall, floorHeights for their locations change.
        std::vector<std::vector<int>> floorHeights;
        for (int y = 0; y < yDomain; ++y) {
            floorHeights.emplace_back(xDomain, 0);
        }

        // References for each x,y the topmost cube occupying that space.
        // Not the cubes at the highest slice, e.g. 0,0 could have a 10 tall cube and 1,1 has a 1 tall cube.
        // Use floorHeights for this.
        std::map<std::pair<int, int>, const Cube *> occupancy;
        for (auto& cube : cubes) { // put an empty list on each to get started.
            connections.emplace(&cube, std::set<const Cube *>());
        }

        for (auto& c : cubes) {

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
                            connections[iter->second].emplace(&c); // updates the connections (!)
                        }
                    }

                    floorHeights[j][i] = maxHeight + cubeHeight; // this floor tile is cubeHeight higher now.
                    occupancy.insert_or_assign(std::make_pair(i, j), &c); // and at this coord the top cube shall be this.
                }
            }
        }
    }
};

} // namespace

#undef DAY