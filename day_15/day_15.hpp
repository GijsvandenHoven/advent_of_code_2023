#pragma once

#include <iostream>
#include <list>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 15

NAMESPACE_DEF(DAY) {

enum class Operation {
    ASSIGNMENT = 0,
    REMOVAL = 1,
};

struct Instruction {
    explicit Instruction(const std::string& from) {
        std::istringstream s(from);
        std::ostringstream o;
        int c;
        while (true) {
            c = s.get();
            switch (c) {
                case EOF:
                    goto loopEnd;
                case '=':
                    op = Operation::ASSIGNMENT;
                    goto loopEnd;
                case '-':
                    op = Operation::REMOVAL;
                    goto loopEnd;
                default:
                    o << static_cast<char>(c);
                    break;
            }
        }
        loopEnd:
        label = o.str();
    }

    virtual ~Instruction() = default;

    std::string label;
    Operation op;
};

struct AssignmentInstruction : Instruction {
    explicit AssignmentInstruction(const std::string& from) : Instruction(from) {
        char c = from.back();
        switch (c) { // NOLINT(*-multiway-paths-covered)
            case '0'...'9':
                focus = c - '0';
                break;
            default: throw std::logic_error("Impossible input for AssignmentInstruction. Expected digit at the end of the string: " + from);
        }

        if (op != Operation::ASSIGNMENT) {
            throw std::logic_error("Wrong label on instruction");
        }
    }

    int focus;
};

struct RemovalInstruction : Instruction {
    explicit RemovalInstruction(const std::string& from) : Instruction(from) {
        if (op != Operation::REMOVAL) {
            throw std::logic_error("Wrong label on instruction");
        }
    }
};

struct Lens {
    Lens() = delete;

    Lens(std::string lbl, int f) : label(std::move(lbl)), focus(f) {}

    std::string label;
    int focus;
};

struct Box : std::list<Lens> {
    std::pair<bool, iterator> seek(const std::string& label) {
        auto iter = std::find_if(this->begin(), this->end(), [&label](auto& lens) {
            return lens.label == label;
        });

        return std::make_pair(iter != this->end(), iter);
    }

    // sum of slot id * focus of lens in slot.
    [[nodiscard]] int focusingPower() const {
        int i = 1;
        int sum = 0;
        for (auto& l : *this) {
            sum += i * l.focus;
            ++i;
        }

        return sum;
    }
};

class HashMap : public std::array<Box, 256> {
public:
    // sum of box id * focus power of box
    [[nodiscard]] int focusingPower() const {
        int i = 1;
        int sum = 0;
        for (auto& b : *this) {
            sum += i * b.focusingPower();
            ++i;
        }

        return sum;
    }
};
//using HashMap = std::array<Box, 256>;

std::ostream& operator<<(std::ostream& os, const Lens& l) {
    return os << l.label << " " << l.focus;
}

std::ostream& operator<<(std::ostream& os, const Box& b) {
    for (auto& l : b) {
        os << "[" << l << "] ";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const HashMap& h) {
    os << "HASHMAP {\n";
    int i = 0;
    for (auto& b : h) {
        if (!b.empty()) {
            os << "\tBox " << i << ": " << b << "\n";
        }
        ++i;
    }
    os << "}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        std::getline(input, line);
        std::istringstream in(line);
        std::ostringstream read;

        int c;
        while (c != EOF) {
            c = in.get();
            if (c == ',' || c == EOF) { // commit this sequence to the vector, clear the read buffer.
                std::string x = read.str();

                if (x.end() == std::find(x.begin(), x.end(), '-')) {
                    instructions.emplace_back(std::make_unique<AssignmentInstruction>(x));
                } else {
                    instructions.emplace_back(std::make_unique<RemovalInstruction>(x));
                }

                sequence.emplace_back(std::move(x));
                read.clear();
                read.str(std::string());
            } else { // build the sequence more.
                read << static_cast<char>(c);
            }
        }
    }

    void v1() const override {
        int sum = 0;
        for (auto& s : sequence) {
            sum += hash(s);
        }

        reportSolution(sum);
    }

    void v2() const override {
        HashMap boxes;

        auto iter = instructions.begin();

        while (iter != instructions.end()) {
            executeInstruction(boxes, iter);
            ++iter;
        }

        reportSolution(boxes.focusingPower());
    }

    void parseBenchReset() override {
        sequence.clear();
        instructions.clear();
    }

private:
    std::vector<std::string> sequence;
    std::vector<std::unique_ptr<Instruction>> instructions;

    static uint8_t hash(char in, uint8_t seed) {
        return (seed + in) * 17; // 'remainder' is done automatically by virtue of uint8_t. The + operation casts to int, so there will be no early modulo truncating things.
    }

    static uint8_t hash(const std::string& s) {
        uint8_t seed = 0;
        for (auto& c : s) {
            seed = hash(c, seed);
        }
        return seed;
    }

    static void executeInstruction(HashMap& boxes, decltype(instructions)::const_iterator iterator) {
        auto& instruction = *iterator;
        uint8_t h = hash(instruction->label);
        Box& b = boxes[h];

        switch(instruction->op) {
            default: throw std::logic_error("Unknown op in executeInstruction");
            case Operation::REMOVAL:
                executeRemove(b, dynamic_cast<RemovalInstruction *>(instruction.get()));
                break;
            case Operation::ASSIGNMENT:
                executeAssign(b, dynamic_cast<AssignmentInstruction *>(instruction.get()));
                break;
        }
    }

    /**
     * Seek the Lens by label in this box. If it is not found, does nothing.
     * When it is found, it is removed from the box. All units after it are shifted over to fill the gap.
     */
    static void executeRemove(Box& b, const RemovalInstruction * r) {
        auto [found, location] = b.seek(r->label);

        if (found) {
            b.erase(location);
        }
    }

    /**
     * Seek the Lens by label in this box. If it is not found, add a new lens to the back with the value described by the instruction.
     * When it is found, overwrites the focus value of the Lens with that described by the instruction.
     */
    static void executeAssign(Box& b, const AssignmentInstruction * a) {
        auto [found, location] = b.seek(a->label);

        if (found) {
            location->focus = a->focus;
        } else {
            b.emplace_back(a->label, a->focus);
        }
    }
};

} // namespace

#undef DAY