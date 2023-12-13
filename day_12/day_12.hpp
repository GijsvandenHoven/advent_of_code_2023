#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 12

NAMESPACE_DEF(DAY) {

struct SpringRecord {
    std::string data;
    std::vector<int> numbers;

    explicit SpringRecord(const std::string& str) {
        std::istringstream s(str);
        s >> data;

        int x;
        while (! s.eof()) {
            s >> x;
            numbers.push_back(x);
            s.get(); // comma (or eof)
        }
    }

    [[nodiscard]] int64_t countPossibleRecords() const {
        int recordIndex = 0;
        int numbersIndex = 0;
        int currentBlockSize = 0;

        auto res = recursiveCount(recordIndex, numbersIndex, currentBlockSize);
        std::cout << res << "\n";
        return res;
    }

    [[nodiscard]] int64_t recursiveCount(int recordIndex, int numbersIndex, int currentBlockSize) const {
        if (recordIndex == data.size()) {
            bool blockTerminatedLegal = numbersIndex == numbers.size() - 1 && currentBlockSize == numbers.back(); // 1 block left but it's finished, actually.
            bool dotTerminatedLegal = numbersIndex == numbers.size() && currentBlockSize == 0; // out of blocks and no ongoing one.

            return blockTerminatedLegal || dotTerminatedLegal;
        }

        char c = data[recordIndex];

        switch (c) {
            case '#':
                if (numbersIndex == numbers.size()) { // out of blocks, yet trying to add to it? impossible.
                    return 0;
                } else if (currentBlockSize == numbers[numbersIndex]) { // adding to the block would overflow here.
                    return 0;
                } else { // expand block (or start one)
                    return recursiveCount(recordIndex + 1, numbersIndex, currentBlockSize + 1);
                }
            case '.':
                if (currentBlockSize != 0 && currentBlockSize != numbers[numbersIndex]) { // end of existing block but not the right size.
                    return 0;
                } else if (currentBlockSize == 0) { // no ongoing block.
                    return recursiveCount(recordIndex + 1, numbersIndex, 0);
                } else if (numbersIndex == numbers.size()) { // nonzero blocksize, but out of numbers? illegal.
                    return 0;
                } else { // nonzero blocksize, so this '.' finishes the block.
                    return recursiveCount(recordIndex + 1, numbersIndex + 1, 0);
                }
            case '?':
                if (numbersIndex == numbers.size()) { // out of spring blocks, this can only be a .
                    return recursiveCount(recordIndex + 1, numbersIndex, 0);
                } else if (currentBlockSize == numbers[numbersIndex]) { // must be a ., or we violate the block size. Also start a new block.
                    return recursiveCount(recordIndex + 1, numbersIndex + 1, 0);
                } else if (currentBlockSize > 0 && currentBlockSize < numbers[numbersIndex]) { // ongoing block must be continued.
                    return recursiveCount(recordIndex + 1, numbersIndex, currentBlockSize + 1);
                } else { // no ongoing block, could be both. So try both.
                    if (currentBlockSize != 0) throw std::logic_error("nonzero block size?");

                    auto a = recursiveCount(recordIndex + 1, numbersIndex, 1);
                    auto b = recursiveCount(recordIndex + 1, numbersIndex, 0);

                    return a + b;
                }
            default: throw std::logic_error("Unexpected char in recursive count.");
        }
    }
};

struct UnfoldedSpringRecord : public SpringRecord {
    explicit UnfoldedSpringRecord(const std::string& toUnfold) : SpringRecord(unfold(toUnfold)) {}

    static std::string unfold(const std::string& folded) {
        std::istringstream s(folded);
        std::string data;
        std::string nums;

        s >> data;
        s >> nums;

        data = data + '?' + data + '?' + data + '?' + data + '?' + data;
        nums = nums + ',' + nums + ',' + nums + ',' + nums + ',' + nums;

        return data + ' ' + nums;
    }
};

std::ostream& operator<<(std::ostream& os, const SpringRecord& s) {
    os << "SpringRecord {\n\t";
    os << s.data << "\n\t";
    for (auto&& v : s.numbers) os << v << ", ";
    os << "\n}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            records.emplace_back(line);
            unfoldedRecords.emplace_back(line);
        }
    }

    void v1() const override {
        int sum = std::accumulate(records.begin(), records.end(), 0, [](int s, auto& item){
            return s + item.countPossibleRecords();
        });
        reportSolution(sum);
    }

    void v2() const override {
//        int sum = std::accumulate(unfoldedRecords.begin(), unfoldedRecords.end(), 0, [](int s, auto& item){
//            return s + item.countPossibleRecords();
//        });
//        reportSolution(sum);
        reportSolution(0);
    }

    void parseBenchReset() override {
        records.clear();
        unfoldedRecords.clear();
    }

private:
    std::vector<SpringRecord> records;
    std::vector<UnfoldedSpringRecord> unfoldedRecords;
};

} // namespace

#undef DAY