#pragma once

#include <iostream>
#include <memory>
#include <utility>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 8

class NetworkNode;
using NetworkNodePtr = std::shared_ptr<NetworkNode>;

enum class Direction : bool {
    LEFT,
    RIGHT
};

class Instructions {
    std::istringstream instructionScanner;
public:

    int pos() {
        return static_cast<int>(instructionScanner.tellg());
    }

    Direction getDirection() {
        int c = instructionScanner.get();

        if (c == EOF) { // It's rewind time!
            instructionScanner.clear();
            instructionScanner.seekg(0);
            c = instructionScanner.get();
        }

        switch(c) {
            case 'L': return Direction::LEFT;
            case 'R': return Direction::RIGHT;
            default:
                throw std::logic_error(std::string("Illegal char in directions ") + static_cast<char>(c));
        }
    }

    explicit Instructions(const std::string& from) {
        instructionScanner.str(from);
    }

    explicit Instructions() = delete;
};

class NetworkNode {
    NetworkNodePtr left_;
    NetworkNodePtr right_;

public:
    const std::string label;

    NetworkNode() = delete;
    explicit NetworkNode(std::string lbl) : label(std::move(lbl)) {}

    void validate () const {
        if (left_ == nullptr || right_ == nullptr) {
            throw std::logic_error("Invalid nodePtr in " + label);
        }
    }

    [[nodiscard]] const NetworkNode * left() const {
        return left_.get();
    }

    [[nodiscard]] const NetworkNode * right() const {
        return right_.get();
    }

    void set_left(NetworkNodePtr l) {
        left_ = std::move(l);
    }
    void set_right(NetworkNodePtr r) {
        right_ = std::move(r);
    }
};

class Network {
    std::map<std::string, NetworkNodePtr> network;

    friend std::ostream& operator<<(std::ostream& os, const Network& n);

public:
    void add_node(NetworkNodePtr&& node) {
        network.emplace(node->label, node);
    }

    [[nodiscard]] const NetworkNode * get_node(const std::string& lbl) const {
        auto iter = std::find_if(network.begin(), network.end(), [&lbl](auto& kvp) {
            return kvp.first == lbl;
        });

        if (iter == network.end()) {
            throw std::logic_error("Label " + lbl + " Was requested, but not present in the Network.");
        }

        return iter->second.get();
    }

    void get_nodes_ending_with(char c, std::vector<const NetworkNode *>& out) const {
        std::for_each(network.begin(), network.end(), [&out, c](auto& pair) {
            if (pair.first.back() == c) {
                out.push_back(pair.second.get());
            }
        });
    }
};

std::ostream& operator<<(std::ostream& os, const Network& n) {
    os  << "Network with (" << n.network.size() << ") nodes: {\n";
    for (auto& item : n.network) {
        os << "\t'" << item.first << "': '" << item.second->left()->label << "', '" << item.second->right()->label << "'\n";
    }
    os << "}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {
        std::string line;
        std::getline(input, line);
        instructions_string = line;

        std::getline(input, line); // blank line

        std::vector<std::pair<NetworkNodePtr, std::pair<std::string, std::string>>> nodelets;
        while (std::getline(input, line)) {
            // format: X = (Y, Z)
            auto a = line.substr(0, 3);
            auto b = line.substr(7, 3);
            auto c = line.substr(12, 3);

            nodelets.push_back({ std::make_shared<NetworkNode>(a), {b, c} });
        }

        /**
         * Curried Predicate function for finding a node matching the 'seeking' parameter.
         * The result of calling the (outer) function should be used with std::find_if or as a similar predicate functor.
         */
        auto search_predicate = [](const std::string& seeking) {
            return [&seeking](auto& nodelet_item) {
                return nodelet_item.first->label == seeking;
            };
        };

        for (const auto& [node, label_pair] : nodelets) {
            auto a = std::find_if(nodelets.begin(), nodelets.end(), search_predicate(label_pair.first));
            auto b = std::find_if(nodelets.begin(), nodelets.end(), search_predicate(label_pair.second));

            if (nodelets.end() == a || nodelets.end() == b) {
                throw std::logic_error("Node labeled " + node->label +  " Has illegal labels: " + label_pair.first + " or " + label_pair.second);
            }

            node->set_left(a->first);
            node->set_right(b->first);
        }

        std::for_each(nodelets.begin(), nodelets.end(), [this](auto& nodelet) {
            nodelet.first->validate();
            network.add_node(std::move(nodelet.first));
        });
    }

    void v1() const override {
        Instructions instructions(instructions_string);

        int stepCount = 0;
        auto* current = network.get_node("AAA");
        while (current->label != "ZZZ") {
            stepCount ++; // step taken, where do we end up?
            auto direction = instructions.getDirection();
            switch(direction) {
                case Direction::LEFT:
                    current = current->left();
                    break;
                case Direction::RIGHT:
                    current = current->right();
                    break;
                default:
                    throw std::logic_error("Literally how, this enum is a bool.");
            }
        }

        reportSolution(stepCount);
    }

    void v2() const override {

        std::vector<const NetworkNode *> starts;
        std::vector<const NetworkNode *> ends;
        network.get_nodes_ending_with('A', starts);
        network.get_nodes_ending_with('Z', ends);

        std::vector<std::pair<int64_t, int64_t>> cycleValues; // first and last reported cycle.
        for (auto start : starts) {
            int steps_taken = 0;
            std::vector<std::tuple<int, std::string, int>> z_node_hits;
            Instructions instructions(instructions_string); // create fresh each time, mutating this between nodes is a bad idea.
            auto node = start;

            bool cycleFound = false;
            while (! cycleFound) {
                node = findNextZ(instructions, node, steps_taken);
                int instruction_pos = instructions.pos();

                auto predicate = [&instruction_pos, &node](auto& item) {
                    return instruction_pos == std::get<0>(item) && node->label == std::get<1>(item);
                };

                // always add it, even if it's a 'duplicate' as per the predicate,
                // we want at least 2 in the vec for cycle calculation with an 'offset' (although if there is an offset, we cannot LCM)
                z_node_hits.emplace_back(instructions.pos(), node->label, steps_taken);

                if (z_node_hits.end() != std::find_if(z_node_hits.begin(), z_node_hits.end(), predicate)) {
                    cycleFound = true;
                }
            }

            // Check for funny business, not part of the puzzle input but...
            if (z_node_hits.size() != 2) {
                int diff = std::get<2>(z_node_hits[1]) - std::get<2>(z_node_hits[0]);
                int prev = 0;
                for (auto& [a, b, c] : z_node_hits) {
                    if (prev != 0 && c - prev != diff) {
                        throw std::logic_error("Can't solve this: Only works for predictable cycles such that some form of LCM can be used.");
                    }
                    prev = c;
                }
            }

            // znodehits has at least 2 members due to how the cycle seeker works.
            cycleValues.emplace_back( std::get<2>(z_node_hits[0]), std::get<2>(z_node_hits.back()) );
        }

        bool hasOffset = false;
        for (auto& [first, last] : cycleValues) {
            auto x = first / last;
            auto y = static_cast<double>(first) / static_cast<double>(last);
            if (static_cast<double>(x) != y) {
                hasOffset = true;
            }
        }

        int64_t result;
        if (hasOffset) {
            result = stupid_lcm(cycleValues);
        } else {
            bool _1 = true;
            for (auto& [first, _] : cycleValues) {
                if (_1) {
                    result = first;
                    _1 = false;
                } else {
                    result = std::lcm(result, first);
                }
            }
        }

        reportSolution(result);
    }

    void parseBenchReset() override {
        instructions_string.clear();
        network = Network();
    }

private:
    std::string instructions_string;
    Network network;

    // problem 2 helper func
    static const NetworkNode * findNextZ(Instructions& i, const NetworkNode * from, int& stepCount) {
        auto current = from;
        do {
            stepCount++;
            auto dir = i.getDirection();
            if (dir == Direction::LEFT) {
                current = current->left();
            } else {
                current = current->right();
            }
            // std::cout << "\t\tstep " << stepCount << " go " << (dir == Direction::LEFT ? 'L' : 'R') << " to " << current->label << "\n";
        } while (current->label.back() != 'Z');

        return current;
    }

    static int64_t stupid_lcm(const std::vector<std::pair<int64_t, int64_t>>& cycleValues) {
        std::cout << "Warn: LCM will be computed in linear time, presumably due to presence of offsets.\n";
        std::vector<int64_t> counts;
        std::vector<int64_t> sizes;
        std::cout << "WE ARE WORKING WITH THIS:\n";
        for (auto& [start, end] : cycleValues) {
            std::cout << "S: " << start << ", E: " << end << ", thus C: " << (end - start) << "\n";
            counts.push_back(start);
            sizes.push_back(end-start);
        }

        int at = 0;
        uint32_t iter = 0; // intentionally 32 byte, we want to report every power of 2 and this overflowing keeps reporting a bit more frequent.
        while ( counts.back() != counts[0] ) {
            iter++;
            auto& a = counts[at];
            auto& b = counts[at+1];

            if (a == b) {
                // std::cout << "@" << at << ": " << a << " == " << b << "\n";
                at++;
            } else if (a > b) {
                b += sizes[at+1];
                at = 0; // invalidated, check again
            } else { // a < b
                a += sizes[at];
                at = 0;
            }

            if ((iter & (iter - 1)) == 0) {
                std::cout << iter << " - ";
                for (auto v : counts) std::cout << v << ", ";
                std::cout << "\n";
            }
        }

        return counts[0];
    }
};

#undef DAY