#pragma once

#include <iostream>
#include <bitset>
#include <bit>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 13

NAMESPACE_DEF(DAY) {

static const unsigned char BitReverseTable256[] =
{
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

struct MirrorableBitfield { // rows and cols represent the same 2d matrix of bits, but the bits are grouped differently.
    std::vector<uint64_t> rows;
    std::vector<uint64_t> cols;

    explicit MirrorableBitfield(std::vector<std::string>& matrix) {
        if (matrix.size() >= 64 || matrix.back().size() >= 64) {
            throw std::logic_error(">= 64 values in rows or columns, cannot do bit field stuff.");
        }

        // go over the rows...
        for (auto& row : matrix) {
            uint64_t r = 0;
            for (int i = 0; i < row.size(); ++i) {
                r |= ((row[i] == '#') << (row.size() - 1 - i));
            }
            rows.emplace_back(r);
        }
        // and the columns...
        for (int col = 0; col < matrix.back().size(); ++col) {
            uint64_t c = 0;
            for (int i = 0; i < matrix.size(); ++i) {
                c |= ((matrix[i][col] == '#') << (matrix.size() - 1 - i));
            }
            cols.emplace_back(c);
        }
    }

    // As per puzzle definition, number of columns left of the mirror point returned if vertical mirroring.
    // 100 times number of rows above mirror point returned if horizontal mirroring.
    // Error is thrown if neither.
    [[nodiscard]] int searchMirror(int tolerance) const {
        {
            auto [found, verticalMirrorPoint] = tryMirror(rows, static_cast<int>(cols.size()), tolerance);
            if (found) {
                return verticalMirrorPoint;
            }
        }
        {
            auto [found, horizontalMirrorPoint] = tryMirror(cols, static_cast<int>(rows.size()), tolerance);
            if (found) {
                return horizontalMirrorPoint * 100;
            }
        }

        throw std::logic_error("No vertical or horizontal mirror point.");
    }

    // Representing either rows or columns (passed as 'axis' vector) and number of items (bits) in the axis.
    // Example use: tryMirror(rows, cols.size()), tryMirror(cols, rows.size())
    [[nodiscard]] static std::pair<bool, int> tryMirror(const std::vector<uint64_t>& axis, const int n, const int tolerance) {
        for (int splitPoint = 1; splitPoint < n; ++splitPoint) {
            int mirroringScore = tolerance;
            for (auto& item : axis) {
                auto leftXORright = checkMirror(item, splitPoint, n);
                int mismatchedBits = std::popcount(leftXORright);
                mirroringScore -= mismatchedBits;
                // could early exit if mirroringScore goes negative but whatever. It might even be slower :)
            }
            if (mirroringScore == 0) { // only if the score is exactly 0, that is, the exact number of 'bit corrections / mismatches' were needed to satisfy this mirroring.
                return std::make_pair(true, splitPoint);
            }
        }

        return std::make_pair(false, -1);
    }

    [[nodiscard]] static uint64_t checkMirror(uint64_t val, int splitPoint, int size) {
        static constexpr auto all_ones = ~0ULL;
        // bits on the left of the splitpoint:
        auto left_mask = (all_ones >> (64 - splitPoint)) << (size - splitPoint);
        auto left = (val & left_mask) >> (size - splitPoint);
        // bits on the right of the splitpoint:
        auto right_mask = ~left_mask;
        auto right = (val & right_mask) << (64 - (size - splitPoint));

        //std::cout << "left:\t" << std::bitset<64>(left) << "\nright:\t" << std::bitset<64>(right) << "\n";

        // left and right are now correctly occupying their position in a 64 bit int. Let's mirror 'right' and see if it equals 'left'.
        int64_t mirrored_right = 0;
        for (int i = 0; i < 8; ++i) {
            auto x = (right >> (i * 8)) & 0xFF;
            auto rev = BitReverseTable256[x];
            mirrored_right |= rev << (56 - (i*8));
        }

        //std::cout << "mirr:\t" << std::bitset<64>(mirrored_right) << "\n";

        // This is the section that should be equal with mirroring. we need to mask it on both values.
        // One of the two should be unaffected by this, the other truncated to the mirroring section.
        int domain = std::min(size - splitPoint, splitPoint);
        auto domainMask = all_ones >> (64 - domain);
        //std::cout << "dom: " << domain << "\nmask:\t" << std::bitset<64>(domainMask) << "\n";

        // give the matching grade of the left compared to the right mirror. 0 is a perfect mirror. The more bits are on, the more is off.
        return (left & domainMask) ^ (mirrored_right & domainMask);
    }
};

std::ostream& operator<<(std::ostream& os, const MirrorableBitfield& mbf) {
    os << "MirrorableBitField {\n";
    os << "\trows:\n";
    for (auto& row : mbf.rows) {
        // os << "\t\t row has value " << row << "\n";
        os << "\t\t";
        for (int i = 0; i < mbf.cols.size(); ++i) {
            os << (row & (1 << (mbf.cols.size() - 1 - i)) ? '#' : '.');
        }
        os << "\n";
    }
    os << "\tcols:\n";
    for (auto& col : mbf.cols) {
        // os << "\t\t col has value " << col << "\n";
        os << "\t\t";
        for (int i = 0; i < mbf.rows.size(); ++i) {
            os << (col & (1 << (mbf.rows.size() - 1 - i)) ? '#' : '.');
        }
        os << "\n";
    }
    os << "}";

    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        std::vector<std::string> matrix;
        matrix.reserve(64);
        while (std::getline(input, line)) {

            if (line.empty()) {
                MirrorableBitfield field(matrix);
                fields.emplace_back(std::move(field));
                matrix.clear();
            } else {
                matrix.emplace_back(std::move(line));
            }
        }
        fields.emplace_back(matrix); // EOF would discard the last matrix otherwise.
    }

    void v1() const override {
        int64_t sum = 0;
        for (auto& field : fields) {
            sum += field.searchMirror(0);
        }
        reportSolution(sum);
    }

    void v2() const override {
        int64_t sum = 0;
        for (auto& field : fields) {
            sum += field.searchMirror(1);
        }
        reportSolution(sum);
    }

    void parseBenchReset() override {
        fields.clear();
    }

private:
    std::vector<MirrorableBitfield> fields;
};

} // namespace

#undef DAY


// testing
//        MirrorableBitfield::checkMirror(0b101, 2, 3); // 2 and (1 << 63)
//        MirrorableBitfield::checkMirror(0b101, 1, 3); // 1 and (1 << 62)
//        MirrorableBitfield::checkMirror(0b111, 2, 3); // 3 and (1 << 63)
//        MirrorableBitfield::checkMirror(0b111, 1, 3); // 1 and (1 << 63) | (1 << 62)
//        MirrorableBitfield::checkMirror(0b11101, 3, 5); // 7 and (1 << 62)
//        MirrorableBitfield::checkMirror(0b111001, 3, 6); // 7 and (1 << 61)
//        MirrorableBitfield::checkMirror(0b1110001, 3, 7); // 7 and (1 << 60)

//constexpr auto ref = (1ull << 63);
//std::cout << "\nreference sheet: 1 << 63 = " << ref << ", 1 << 62 = " << (ref >> 1) << ", 1 << 61 = " << (ref >> 2) << ", 1 << 60 = " << (ref >> 3) << "\n";

// reportSolution(0);

//##.####.######.##
//bool mirrors = MirrorableBitfield::checkMirror(0b11011110111111011ULL, 11, 17); // failing.
// bool mirrors = MirrorableBitfield::checkMirror(0b1100110011ULL, 5, 10); // passing.
// bool mirrors = MirrorableBitfield::checkMirror(0b10101111ULL, 7, 8); // passing.
//std::cout << "mirrors ? " << mirrors << "\n";

/*
.##......
###.####.
##.##...#
..###..##
...##..##
#..#.##.#
..#......
.##..##..
.##..##..
 */