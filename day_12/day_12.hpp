#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 12

NAMESPACE_DEF(DAY) {

struct SpringRecord {
    using StateMap = std::map<std::tuple<int, int, int>, int64_t>;

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

        StateMap cache;
        auto res = recursiveCount(recordIndex, numbersIndex, currentBlockSize, cache);
        return res;
    }

    [[nodiscard]] int64_t recursiveCount(int recordIndex, int numbersIndex, int currentBlockSize, StateMap& cache) const {
        { // search the cache first, any early exit possible?
            auto state = std::make_tuple(recordIndex, numbersIndex, currentBlockSize);
            auto iter = cache.find(state);
            if (iter != cache.end()) {
                return iter->second;
            }
        }

        if (recordIndex == data.size()) {
            bool blockTerminatedLegal = numbersIndex == numbers.size() - 1 && currentBlockSize == numbers.back(); // 1 block left but it's finished, actually.
            bool dotTerminatedLegal = numbersIndex == numbers.size() && currentBlockSize == 0; // out of blocks and no ongoing one.

            bool legal = blockTerminatedLegal || dotTerminatedLegal;

            cache.emplace(std::make_tuple(recordIndex, numbersIndex, currentBlockSize), legal);

            return legal;
        }

        char c = data[recordIndex];

        switch (c) {
            case '#': {
                int64_t ret;
                if (numbersIndex == numbers.size()) { // out of blocks, yet trying to add to it? impossible.
                    ret = 0;
                } else if (currentBlockSize == numbers[numbersIndex]) { // adding to the block would overflow here.
                    ret = 0;
                } else { // expand block (or start one)
                    ret = recursiveCount(recordIndex + 1, numbersIndex, currentBlockSize + 1, cache);
                }
                cache.emplace(std::make_tuple(recordIndex, numbersIndex, currentBlockSize), ret);
                return ret;
            }
            case '.': {
                int64_t ret;
                if (currentBlockSize != 0 && currentBlockSize != numbers[numbersIndex]) { // end of existing block but not the right size.
                    ret = 0;
                } else if (currentBlockSize == 0) { // no ongoing block.
                    ret = recursiveCount(recordIndex + 1, numbersIndex, 0, cache);
                } else if (numbersIndex == numbers.size()) { // nonzero blocksize, but out of numbers? illegal.
                    ret = 0;
                } else { // nonzero blocksize, so this '.' finishes the block.
                    ret = recursiveCount(recordIndex + 1, numbersIndex + 1, 0, cache);
                }
                cache.emplace(std::make_tuple(recordIndex, numbersIndex, currentBlockSize), ret);
                return ret;
            }
            case '?': {
                int64_t ret;
                if (numbersIndex == numbers.size()) { // out of spring blocks, this can only be a .
                    ret = recursiveCount(recordIndex + 1, numbersIndex, 0, cache);
                } else if (currentBlockSize == numbers[numbersIndex]) { // must be a ., or we violate the block size. Also start a new block.
                    ret = recursiveCount(recordIndex + 1, numbersIndex + 1, 0, cache);
                } else if (currentBlockSize > 0 && currentBlockSize < numbers[numbersIndex]) { // ongoing block must be continued.
                    ret = recursiveCount(recordIndex + 1, numbersIndex, currentBlockSize + 1, cache);
                } else { // no ongoing block, could be both. So try both.
                    if (currentBlockSize != 0) throw std::logic_error("nonzero block size?");

                    auto a = recursiveCount(recordIndex + 1, numbersIndex, 1, cache);
                    auto b = recursiveCount(recordIndex + 1, numbersIndex, 0, cache);

                    ret = a + b;
                }
                cache.emplace(std::make_tuple(recordIndex, numbersIndex, currentBlockSize), ret);
                return ret;
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
        reportSolution(0);
    }

    void v2() const override {
        int64_t sum = std::accumulate(unfoldedRecords.begin(), unfoldedRecords.end(), 0ll, [](int64_t s, auto& item){
            return s + item.countPossibleRecords();
        });
        reportSolution(sum);
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