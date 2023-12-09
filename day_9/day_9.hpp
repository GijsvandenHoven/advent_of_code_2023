#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 9

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

    [[nodiscard]] int checkAccess(int row, int item) const {
        int index = item + (((row+1) * row) / 2); // 0 based indexing for both item and row.
        if (index >= this->size()) {
            throw std::logic_error("Out of bounds access on Pyramid: " + std::to_string(index) + " Requested on pyramid of dimension " + std::to_string(DIMENSION));
        }
        return index;
    }

    [[nodiscard]] size_t getIndex(int row, int item) const {
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
        int64_t result_c = 0;
        int64_t result_lazy_1 = 0;
        int64_t result_lazy_2 = 0;
        for (const auto& vec : data) {
            result_c += interpolateVec_correct(vec);
            result_lazy_1 += interpolateVec_1(vec);
            result_lazy_2 += interpolateVec_2(vec);

//            if (result_lazy_1 != result_c || result_lazy_2 != result_c) {
//                std::for_each(vec.begin(), vec.end(), [](auto& v) {
//                    std::cout << v << "\n";
//                });
//                exit(-1);
//            }
        }

        std::cout << result_c << " vs " << result_lazy_1 << " vs " << result_lazy_2 << "\n";

        reportSolution(result_c);
    }

    void v2() const override {
        int64_t result_c = 0;
        int64_t result_lazy_1 = 0;
        int64_t result_lazy_2 = 0;


    }

    void parseBenchReset() override {
        data.clear();
    }

private:
    std::vector<std::vector<int>> data;

    template<typename T>
    static int interpolateVec_correct(const std::vector<T>& input) {
        Pyramid<T> pyramid(static_cast<T>(input.size() - 1));
        calculatePyramidValue(pyramid, input, 0, 0);
//        std::cout << "what the fuck?\n";
//        std::cout << pyramid << "\n";

        int row = 0;
        int item = 0;
        T interp{};
        while (row != pyramid.DIMENSION) {
            interp += pyramid.atIndex(row, item);
            row++;
            item++;
        }
        int x = interp + input.back();
        std::cout << "interp'd " << x << "\n";
        return x;
    }

    template<typename T>
    static int interpolateVec_1(const std::vector<T> & input) {
        Pyramid<T> pyramid(static_cast<T>(input.size() - 1));
        int size = input.size();
        // start by pointing to the bottom right of the pyramid. -1 due to shrinkage by being "on top" of the vector, -1 due to 0 based index.
        auto firstZeroCoordinate = lazyFillInterpolationPyramid_1(pyramid, input, size - 2, size - 2);
        // now walk down the right-edge, starting at firstZeroCoordinate, to find the value that is interpolated to be next in the vector.
        auto [row, item] = firstZeroCoordinate;
        T interp{};
        while (row != pyramid.DIMENSION) {
            interp += pyramid.atIndex(row, item);

            row++;
            item++;
        }

        auto x = interp + input.back();
        std::cout << "interp'd " << x << "\n";
        return x;
    }

    template<typename T>
    static int interpolateVec_2(const std::vector<T> & input) {
        Pyramid<T> pyramid(static_cast<T>(input.size() - 1));
        int size = input.size();
        // start by pointing to the bottom right of the pyramid. -1 due to shrinkage by being "on top" of the vector, -1 due to 0 based index.
        auto firstZeroCoordinate = lazyFillInterpolationPyramid_2(pyramid, input, size - 2, size - 2);
        // now walk down the right-edge, starting at firstZeroCoordinate, to find the value that is interpolated to be next in the vector.
        auto [row, item] = firstZeroCoordinate;
        T interp{};
        while (row != pyramid.DIMENSION) {
            interp += pyramid.atIndex(row, item);

            row++;
            item++;
        }

        auto x = interp + input.back();
        std::cout << "interp'd " << x << "\n";
        return x;
    }

/** Not that helpful due to problem 2 requiring left side, not even correct since we stop at the first zero which apparently is not valid. */

    template<typename T>
    static std::pair<int, int> lazyFillInterpolationPyramid_1(Pyramid<T>& pyramid, const std::vector<T> & input, int row, int item) {
        if (row < 0 || item < 0) {
            std::cout << pyramid << "\n";
            throw std::logic_error("No interpolation possible, differences pyramid ends in non-zero.");
        }
        T& pyramidItem = pyramid.atIndex(row, item);

        // This function exits early if there is no work to be done (i.e. the value is already assigned)
        calculatePyramidValue(pyramid,  input, row, item);

        if (pyramidItem == 0 && pyramid.atIndex(row + 1, item) == 0 && pyramid.atIndex(row + 1, item + 1) == 0) { // we are done.
            return std::make_pair(row, item);
        } else { // We do not have enough information. Go up one step along the right edge of the pyramid.
            return lazyFillInterpolationPyramid_1(pyramid, input, row - 1, item - 1);
        }
    }

    template<typename T>
    static std::pair<int, int> lazyFillInterpolationPyramid_2(Pyramid<T>& pyramid, const std::vector<T> & input, int row, int item) {
        if (row < 0 || item < 0) {
            std::cout << pyramid << "\n";
            throw std::logic_error("No interpolation possible, differences pyramid ends in non-zero.");
        }
        T& pyramidItem = pyramid.atIndex(row, item);

        // This function exits early if there is no work to be done (i.e. the value is already assigned)
        calculatePyramidValue(pyramid,  input, row, item);

        if (pyramidItem == 0) { // we are done.
            return std::make_pair(row, item);
        } else { // We do not have enough information. Go up one step along the right edge of the pyramid.
            return lazyFillInterpolationPyramid_2(pyramid, input, row - 1, item - 1);
        }
    }

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