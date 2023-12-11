#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 11

NAMESPACE_DEF(DAY) {

struct Galaxy {
    int x;
    int y;

    Galaxy(int x, int y) : x(x), y(y) {}

    [[nodiscard]] int manhattanDistance(const Galaxy& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }
};

std::ostream& operator<<(std::ostream& os, const Galaxy& g) {
    os << "Galaxy at { " << g.x << ", " << g.y << "\n";
    return os;
}

constexpr int P2_GALAXY_EXPANSION_FACTOR = 1'000'000;

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    // Assumes a rectangular input; Every line should have the same amount of columns.
    void parse(std::ifstream &input) override {
        int columns = 0;
        while (input.get() != '\n') {
            columns++;
        }
        input.clear();
        input.seekg(0);

        std::vector<int> columnGalaxyCount(columns);

        int x = 0;
        int y_1 = 0;
        int y_2 = 0; // need to separately track the 1 million unit expansion of problem 2.

        int c;
        int rowCount = 0;
        while ((c = input.get()) != EOF) {
            switch(c) {
                case '\n':
                    y_1 += (rowCount == 0 ? 2 : 1); // rows count double if no galaxies.
                    y_2 += (rowCount == 0 ? P2_GALAXY_EXPANSION_FACTOR : 1);
                    x = 0;
                    rowCount = 0;
                    continue; // do not update column and x.
                case '#':
                    rowCount++;
                    columnGalaxyCount[x]++;
                    galaxies.emplace_back(x, y_1);
                    veryExpandedGalaxies.emplace_back(x, y_2);
                    break;
                default:
                    break;
            }
            x++;
        }

        { // problem 1 error correction
            // correct coordinates for columns
            int offset = 0;
            for (int i = 0; i < columnGalaxyCount.size(); ++i) {
                if (columnGalaxyCount[i] == 0) {
                    // every galaxy after this X should be offset by 1 on X for this expansion.
                    std::for_each(galaxies.begin(), galaxies.end(), [i, offset](auto &galaxy) {
                        if (galaxy.x > i + offset) {
                            galaxy.x++;
                        }
                    });
                    offset++; // so stretched galaxies do not get stretched again for columns that were originally past them.
                }
            }
        }
        { // problem 1 error correction
            // correct coordinates for columns
            int offset = 0;
            for (int i = 0; i < columnGalaxyCount.size(); ++i) {
                if (columnGalaxyCount[i] == 0) {
                    // every galaxy after this X should be offset by 1 on X for this expansion.
                    std::for_each(veryExpandedGalaxies.begin(), veryExpandedGalaxies.end(), [i, offset](auto &galaxy) {
                        if (galaxy.x > i + offset) {
                            galaxy.x += P2_GALAXY_EXPANSION_FACTOR - 1;
                        }
                    });
                    offset+= P2_GALAXY_EXPANSION_FACTOR - 1; // so stretched galaxies do not get stretched again for columns that were originally past them.
                }
            }
        }
    }

    void v1() const override {
        reportSolution(sumManhattanDistanceOfGalaxyPairs<int32_t>(galaxies));
    }

    void v2() const override {
        reportSolution(sumManhattanDistanceOfGalaxyPairs<int64_t>(veryExpandedGalaxies));
    }

    void parseBenchReset() override {
        galaxies.clear();
        veryExpandedGalaxies.clear();
    }

private:
    template<typename Integer>
    static inline Integer sumManhattanDistanceOfGalaxyPairs(const std::vector<Galaxy>& gs) {
        Integer sum = 0;
        for (int i = 0; i < gs.size(); ++i) {
            for (int j = i+1; j < gs.size(); ++j) {
                sum += gs[i].manhattanDistance(gs[j]);
            }
        }
        return sum;
    }

    std::vector<Galaxy> galaxies;
    std::vector<Galaxy> veryExpandedGalaxies; // problem 2. since we do not want to mutate the vector after parsing once, we should parse in the 2 variants separately.
};

} // namespace

#undef DAY