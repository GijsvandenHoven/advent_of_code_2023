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
        std::vector<char> recordMemory;
        recordMemory.reserve(data.size());

        //std::cout << "Recursive backtracking on '" << data << "'...\n";

        int64_t result = recursiveCount(recordIndex, numbersIndex, recordMemory);
        //std::cout << "FOUND " << result << " FOR " << data << "\n";

        return result;
    }

    static void strmem(const std::vector<char>& m) {
        std::cout << "mem: "; for (auto&& c : m) std::cout << c; std::cout << "\n";
    }

    [[nodiscard]] int64_t recursiveCount(
            int recordIndex,
            int numbersIndex,
            std::vector<char>& recordMemory
    ) const {
        if (recordIndex >= data.size()) {
            strmem(recordMemory);
            bool ok = sequenceIsLegal(recordMemory); // recursion base case. This sequence is maybe possible.

//            if (ok) { // caching results for later.
//
//            }

            return ok; // 0 or 1 is intentional here. it's either possible, or not...
        }

        char c = data[recordIndex];
        switch(c) {
            case '.': {
                auto [cont, retVal] = handleDot(numbersIndex, recordMemory);
                if (!cont) return retVal;
                break;
            }
            case '#': {
                auto [cont, retVal] = handlePound(numbersIndex, recordIndex, recordMemory);
                if (!cont) return retVal;
                break;
            }
            case '?': {
                return handleQuestion(numbersIndex, recordIndex, recordMemory);
            }
            default: throw std::logic_error("Impossible char.");
        }

        // so far so good. let's try the next character.
        return recursiveCount(recordIndex + 1, numbersIndex, recordMemory);
    }

    [[nodiscard]] bool sequenceIsLegal(const std::vector<char>& partialSequence) const {

        if (partialSequence.size() > data.size()) return false;

        int numbersIndex = 0;
        int currentSequenceCount = 0;
        for (int i = 0; i < partialSequence.size(); ++i) {
            switch(partialSequence[i]) {
                case '#':
                    if (numbersIndex >= numbers.size()) {
                        return false;
                    }
                    currentSequenceCount++;
                    break;
                case '.':
                    if (i > 0 && partialSequence[i-1] == '#') { // sequence ended.
                        if (currentSequenceCount != numbers[numbersIndex]) { // and it's not a match.
                            return false;
                        }
                        currentSequenceCount = 0;
                        numbersIndex++;
                    }
                    break;
                default: throw std::logic_error("Impossible char in partial sequence.");
            }
        }

        if (partialSequence.size() == data.size()) {
            if (partialSequence.back() == '#') {
                return numbersIndex == numbers.size() - 1 && currentSequenceCount == numbers[numbersIndex];
            } else {
                return numbersIndex == numbers.size();
            }
        } else {
            if (partialSequence.back() == '#') {
                return numbersIndex <= numbers.size() - 1 && currentSequenceCount <= numbers[numbersIndex]; // there might be more # after.
            } else {
                return true; // looks good so far.
            }
        }
    }

    [[nodiscard]] std::pair<bool, int64_t> handleDot(int& numbersIndex, std::vector<char>& recordMemory) const {
        if (recordMemory.back() == '#') { // sequence of numbers end.
            // was this sequence as expected?
            int expected = numbers[numbersIndex];
            int count = 0;
            for (auto it = recordMemory.rbegin(); it != recordMemory.rend() && (*it == '#'); ++it) {
                ++count;
            }
            if (count != expected) {
                return std::make_pair(false, 0);
            }

            numbersIndex++; // go to the next number since this sequence is completed.
        }
        recordMemory.emplace_back('.');
        return std::make_pair(true, 0);
    }

    [[nodiscard]] std::pair<bool, int64_t> handlePound(int& numbersIndex, int& recordIndex, std::vector<char>& recordMemory) const {
        if (numbersIndex >= numbers.size()) { // found another sequence, but we are out of numbers representing them. Impossible.
            return std::make_pair(false, 0);
        }
        recordMemory.emplace_back('#');
        // check legality of this by comparing the current cluster to the recent number.
        int seqEnd = recordIndex + 1;
        while (seqEnd < data.size() && data[seqEnd] == '#') { // check the data if the future is still possible.
            seqEnd++;
        }
        int seqStart = recordIndex - 1;
        while (seqStart >= 0 && recordMemory[seqStart] == '#') { // check the memory, for decisions we made in the past.
            seqStart--;
        }
        int minlen = (seqEnd - 1) - seqStart;
        while (seqEnd < data.size() && (data[seqEnd] == '#' || data[seqEnd] == '?')) { // check more if it's possible.
            seqEnd++;
        }
        int maxlen = (seqEnd - 1) - seqStart;

        int recentNumber = numbers[numbersIndex];
        if (recentNumber < minlen || recentNumber > maxlen) {
            return std::make_pair(false, 0); // We are on a path of creating an impossible sequence.
        }

        return std::make_pair(true, 0);
    }

    [[nodiscard]] int64_t handleQuestion(int& numbersIndex, int& recordIndex, std::vector<char>& recordMemory) const {
        int64_t totalPossible = 0;
        // let's try '#', how much is possible?
        {
            recordMemory.emplace_back('#');
            bool legal = sequenceIsLegal(recordMemory);
            // std::cout << "\t\tRecurse on ? with '#', is it legal? " << legal << "\n";
            if (legal) {
                int64_t options = recursiveCount(
                        recordIndex + 1,
                        numbersIndex,
                        recordMemory
                );

                totalPossible += options;
            }
            // clean up after recursion (or just illegal adding of a '.'), recordMemory was mutated.
            // We want a memory from 0 up to but not incl. recordIndex.
            recordMemory.erase(recordMemory.begin() + recordIndex, recordMemory.end());
        }
        // let's try '.', how much is possible?
        {
            bool sequenceEnder = recordMemory.back() == '#';
            bool legal = true;
            recordMemory.emplace_back('.');
            // check legality of this move.
            if (sequenceEnder) {
                legal = sequenceIsLegal(recordMemory);
            }

            if (legal) { // filled in a ., continue ahead to count possibilities.

                int64_t options = recursiveCount(
                        recordIndex + 1,
                        numbersIndex + sequenceEnder,
                        recordMemory
                );

                totalPossible += options;
            }
            // clean up after recursion (or just illegal adding of a '.'), recordMemory was mutated.
            // We want a memory from 0 up to but not incl. recordIndex.
            recordMemory.erase(recordMemory.begin() + recordIndex, recordMemory.end());
        }

        return totalPossible;
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