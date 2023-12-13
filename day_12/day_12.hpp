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

    [[nodiscard]] int countPossibleRecords() const {
        int recordIndex = 0;
        int numbersIndex = 0;
        std::vector<char> recordMemory;
        recordMemory.reserve(data.size());

        std::cout << "Recursive backtracking on '" << data << "'...\n";

        int result = recursiveCount(recordIndex, numbersIndex, recordMemory);
        std::cout << "FOUND " << result << " FOR " << data << "\n";
        return result;
    }

    [[nodiscard]] int recursiveCount(int recordIndex, int numbersIndex, std::vector<char>& recordMemory) const {
        if (recordIndex >= data.size()) {
            //std::cout << "\t\t SEQ END, check legal: ";
            bool ok = sequenceIsLegal(recordMemory); // recursion base case. This sequence is maybe possible.
            //std::cout << (ok ? "yes" : "no") << "\n";
            return ok;
        }

        char c = data[recordIndex];
        //std::cout << "\tnext op '" << c << "' ...\n";
        switch(c) {
            case '.':
                if (recordMemory.back() == '#') { // sequence of numbers end.
                    // was this sequence as expected?
                    int expected = numbers[numbersIndex];
                    int count = 0;
                    for (auto it = recordMemory.rbegin(); it != recordMemory.rend() && (*it == '#'); ++it) {
                        ++count;
                    }
                    if (count != expected) {
                        //std::cout << "\t\tAfter a '.' that came after '#', the sequence was illegal. Have: " << count << ", expected: " << expected << "\n";
                        return 0;
                    }

                    numbersIndex++; // go to the next number since this sequence is completed.
                }
                recordMemory.emplace_back(c);
                break;
            case '#': {
                if (numbersIndex >= numbers.size()) { // found another sequence, but we are out of numbers representing them. Impossible.
                    return 0;
                }
                recordMemory.emplace_back(c);
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
                    //std::cout << "\t\tAfter a normal '#', this sequence was found illegal.\n";
                    //std::cout << "\t\tExpected " << recentNumber << ", got : [" << minlen << ", " << maxlen << "]\n";
                    return 0; // We are on a path of creating an impossible sequence.
                }
                break;
            }
            case '?': {
                int totalPossible = 0;
                // let's try '.', how much is possible?
                {
                    bool sequenceEnder = recordMemory.back() == '#';
                    bool legal = true;
                    recordMemory.emplace_back('.');
                    // check legality of this move.
                    if (sequenceEnder) {
                        legal = sequenceIsLegal(recordMemory);
                    }

                    //std::cout << "\t\tRecurse on ? with '.', is it legal? " << legal << "\n";
                    if (legal) { // filled in a ., continue ahead to count possibilities.
                        totalPossible += recursiveCount(recordIndex + 1, numbersIndex + sequenceEnder, recordMemory);
                        //std::cout << "\t\tnew count after recurse on ? with . -> " << totalPossible << "\n";
                    }
                    // clean up after recursion (or just illegal adding of a '.'), recordMemory was mutated.
                    // We want a memory from 0 up to but not incl. recordIndex.
                    recordMemory.erase(recordMemory.begin() + recordIndex, recordMemory.end());
                }
                // let's try '#', how much is possible?
                {
                    recordMemory.emplace_back('#');
                    bool legal = sequenceIsLegal(recordMemory);
                    //std::cout << "\t\tRecurse on ? with '#', is it legal? " << legal << "\n";
                    if (legal) {
                        totalPossible += recursiveCount(recordIndex + 1, numbersIndex, recordMemory);
                        //std::cout << "\t\tnew count after recurse on ? with # -> " << totalPossible << "\n";
                    }
                    // clean up after recursion (or just illegal adding of a '.'), recordMemory was mutated.
                    // We want a memory from 0 up to but not incl. recordIndex.
                    recordMemory.erase(recordMemory.begin() + recordIndex, recordMemory.end());
                }

                return totalPossible;
            }
            default: throw std::logic_error("Impossible char.");
        }

        // so far so good. let's try the next character.
        //std::cout << "\t\tgo deeper...\n";
        return recursiveCount(recordIndex + 1, numbersIndex, recordMemory);
    }

    [[nodiscard]] bool sequenceIsLegal(const std::vector<char>& partialSequence) const {
        //std::cout << "\t\t\tchck legal: ";
        //for (auto&& c : partialSequence) std::cout << c;
        //std::cout << " w.r.t. ";
        //for (auto&& v  : numbers) std::cout << v << ",";
        //std::cout << "\n";

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
        }
    }

    void v1() const override {
        int sum = std::accumulate(records.begin(), records.end(), 0, [](int s, auto& item){
            return s + item.countPossibleRecords();
        });
        reportSolution(sum);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {

    }

private:
    std::vector<SpringRecord> records;
};

} // namespace

#undef DAY