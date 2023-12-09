#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 9

#define DO_LAZY_PYRAMID_INTERPOLATION true
#define DO_ERROR_CHECKS false

// Forward declaring the template class and the insertion operator of it.
// This is a way to prevent declaring every template instantiotion a friend of every other operator<<.
// e.g. Pyramid<int> and operator<<(Pyramid<double>) are NOT friends.
// https://stackoverflow.com/questions/4660123/overloading-friend-operator-for-class-template

template <typename T> class Pyramid;
template <typename T> std::ostream& operator<< ( std::ostream&, const Pyramid<T>& );

template<typename T>
class Pyramid : public std::vector<T> {

    // Declaring as friend only this version of operator<<.
    friend std::ostream& operator<< <T>(std::ostream&, const Pyramid<T>& p);

    [[nodiscard]] inline int checkAccess(int row, int item) const {
        int index = item + (((row+1) * row) / 2); // 0 based indexing for both item and row.
#if DO_ERROR_CHECKS
        if (index >= this->size()) {
            throw std::logic_error("Out of bounds access on Pyramid: " + std::to_string(index) + " Requested on pyramid of dimension " + std::to_string(DIMENSION));
        }
#endif
        return index;
    }

    [[nodiscard]] inline size_t getIndex(int row, int item) const {
        return checkAccess(row, item);
    }

public:
    explicit Pyramid(int base) : std::vector<T>(0), DIMENSION(base) {
        auto siz = static_cast<size_t>((base+1) * (base / 2.0));
        this->resize(siz, UNINITIALIZED_VALUE);
    }

    // Get a reference to the item in the pyramid, referenced by 0-based row and column. Row 0 is the tip of the pyramid.
    [[nodiscard]] T& atIndex(int row, int item) {
        return this->operator[](getIndex(row, item));
    }
    [[nodiscard]] const T& atIndex(int row, int item) const {
        return this->operator[](getIndex(row, item));
    }

    const T UNINITIALIZED_VALUE = std::numeric_limits<T>::min();

    const int DIMENSION;
};

template<typename T>
std::ostream &operator<<(std::ostream & os, const Pyramid<T> &p) {
    os << "Pyramid {\n";
    int width = 1;
    for (int i = 0; i < p.DIMENSION; ++i) {
        for (int j = 0; j < width; ++j) {
            os << p.atIndex(i, j) << " ";
        }
        width++;
        os << "\n";
    }
    os << "}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {
        std::string line;
        while(std::getline(input, line)) {
            std::istringstream s(line);
            data.emplace_back();
            int y;
            while (s >> y) {
                data.back().push_back(y);
            }
        }
    }

    void v1() const override {
        int64_t result = 0;
        for (const auto& vec : data) {
#if DO_LAZY_PYRAMID_INTERPOLATION == true
            result += lazyInterpolateVec<false>(vec);
#else
            result += interPolateVecFull<false>(vec);
#endif
        }

        reportSolution(result);
    }

    void v2() const override {
        int64_t result = 0;
        for (const auto& vec : data) {
#if DO_LAZY_PYRAMID_INTERPOLATION == true
            result += lazyInterpolateVec<true>(vec);
#else
            result += interPolateVecFull<true>(vec);
#endif
        }

        reportSolution(result);
    }

    void parseBenchReset() override {
        data.clear();
    }

private:
    std::vector<std::vector<int>> data;

#if DO_LAZY_PYRAMID_INTERPOLATION == true
    template<bool LeftEdge, typename T>
    static int lazyInterpolateVec(const std::vector<T> & input) {
        Pyramid<T> pyramid(static_cast<T>(input.size() - 1));
        int size = input.size();
        int itemIndex;

        // compile-time evaluates whether to start "left" or "right" of the pyramid.
        if constexpr (LeftEdge) {
            itemIndex = 0;
        } else {
            itemIndex = size - 2;
        }

        // start by pointing to the bottom right of the pyramid. -1 due to shrinkage by being "on top" of the vector, -1 due to 0 based index.
        auto row = lazyFillInterpolationPyramidFromEdge<LeftEdge>(pyramid, input, size - 2, itemIndex);

        // similarly decide where in the row this zero is depending on the edge we inspect.
        int item;
        if constexpr (LeftEdge) { item = 0; } else { item = row; }
        // now walk down the right-edge, starting at firstZeroCoordinate, to find the value that is interpolated to be next in the vector.
        T interp{};
        while (row != pyramid.DIMENSION) {
            auto pyramidEdgeValue = pyramid.atIndex(row, item);

            if constexpr (LeftEdge) {
                interp = pyramidEdgeValue - interp;
                row++;
                // item = 0; each time, so let's just do nothing
            } else {
                interp = pyramidEdgeValue + interp;
                row++;
                item++;
            }
        }

        if constexpr (LeftEdge) {
            return input[0] - interp;
        } else {
            return input.back() + interp;
        }
    }

    template<bool LeftEdge, typename T>
    static int lazyFillInterpolationPyramidFromEdge(Pyramid<T>& pyramid, const std::vector<T> & input, int row, int item) {
#if DO_ERROR_CHECKS
        if (row < 0 || item < 0) {
            std::cout << pyramid << "\n";
            throw std::logic_error("No interpolation possible, differences pyramid ends in non-zero.");
        }
#endif
        T& pyramidItem = pyramid.atIndex(row, item);

        // This function exits early if there is no work to be done (i.e. the value is already assigned)
        calculatePyramidValue(pyramid,  input, row, item);

        // This is the condition under which we are 'done'. Just checking 0 in the current cell does not suffice:
        // consider a pyramid layer [3 2 1 0], it would count itself done,
        // while actually [-1 -1 -1] then [0 0] should be above it still.
        if (
                pyramidItem == 0 && // this item is zero
                row + 1 != pyramid.DIMENSION && // safeguard for checking children. We could be done after just one row, but in such cases the cost of doing 1 more layer is worth not complicating the logic to me.
                pyramid.atIndex(row + 1, item) == 0 && // left of this item is 0
                pyramid.atIndex(row + 1, item + 1) == 0 // right of this item is 0
        ) {
            return row;
        } else { // We do not have enough information. Go up one step along the edge of the pyramid.

            // stick to 0 on the left edge, correct for there being 1 less item on a row on the right edge.
            int itemOffset;
            if constexpr (LeftEdge) { itemOffset = 0; } else { itemOffset = item - 1; }

            return lazyFillInterpolationPyramidFromEdge<LeftEdge>(pyramid, input, row - 1, itemOffset);
        }
    }
#else
    // non-lazy variant, calculates the entire pyramid. Slower compared to lazies, the larger the input is. O(N^2) slower.
    template<bool LeftEdge, typename T>
    static int interPolateVecFull(const std::vector<T>& input) {
        Pyramid<T> pyramid(static_cast<T>(input.size() - 1));
        calculatePyramidValue(pyramid, input, 0, 0);

        int row = 0;
        int item;
        if constexpr (LeftEdge) { item = 0; } else { item = row; }

        T interp{};
        while (row != pyramid.DIMENSION) {
            auto pyramidEdgeValue = pyramid.atIndex(row, item);

            if constexpr (LeftEdge) {
                interp = pyramidEdgeValue - interp;
                row++;
                // item = 0; each time, so let's just do nothing
            } else {
                interp = pyramidEdgeValue + interp;
                row++;
                item++;
            }
        }

        if constexpr (LeftEdge) {
            return input[0] - interp;
        } else {
            return input.back() + interp;
        }
    }
#endif
    template<typename T>
    static const T& calculatePyramidValue(Pyramid<T>& pyramid, const std::vector<T>& base, int row, int item) {
        T& pyramidItem = pyramid.atIndex(row, item);
        if (pyramidItem == pyramid.UNINITIALIZED_VALUE) {
            // calculate this value in the pyramid by taking the difference of the items 'left' and 'right' below it.

            int left = item;
            int right = item + 1;

            if (row + 1 == pyramid.DIMENSION) { // base of pyramid, get from input vector instead.
                pyramidItem = base[right] - base[left];
            } else {
                const T& l = calculatePyramidValue(pyramid, base, row + 1, left);
                const T& r = calculatePyramidValue(pyramid, base, row + 1, right);

                pyramidItem = r - l;
            }
        } // otherwise we already have the value...

        return pyramidItem;
    }
};

#undef DAY
#undef DO_LAZY_PYRAMID_INTERPOLATION
#undef DO_ERROR_CHECKS