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
        network.get_nodes_ending_with('A', starts);

        // This solution assumes that nodes immediately start on their cycle,
        // And that a cycle is as simple as possible: just one Z node per cycle.
        // This enables us to compute the LCM of all cycles.
        // A more general solution exists (LCM computation in linear time),
        // For when there is a 'tail' of nodes before a cycle is entered.
        // At commit 4c3c25a027f1759cabaeb6ac372d042bdcfbc4bf.
        // Even more general solutions (Multiple Z nodes per cycle) were not made.
        int64_t cycles_lcm;
        {
            int first_node_cycle_count = 0;
            Instructions i(instructions_string);
            findNextZ(i, starts[0], first_node_cycle_count);
            cycles_lcm = first_node_cycle_count;
        }

        for (auto iter = starts.begin() + 1; iter != starts.end(); ++iter) {
            int steps_taken = 0;
            Instructions i(instructions_string);
            findNextZ(i, *iter, steps_taken);
            cycles_lcm = std::lcm(cycles_lcm, steps_taken);
        }

        reportSolution(cycles_lcm);
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
};

#undef DAY