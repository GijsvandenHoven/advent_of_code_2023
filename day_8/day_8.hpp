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
    std::vector<Direction> directions;
    size_t currentDirection = 0;
public:

    Direction getDirection() {
        auto direction = directions[currentDirection];
        currentDirection = (currentDirection + 1) % directions.size();
        return direction;
    }

    explicit Instructions(const std::string& from) {
        for (auto c : from) {
            directions.push_back(c == 'L' ? Direction::LEFT : Direction::RIGHT);
        }
    }

    explicit Instructions() = default;
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

    [[nodiscard]] NetworkNodePtr left() const {
        return left_;
    }

    [[nodiscard]] NetworkNodePtr right() const {
        return right_;
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
        instructions = Instructions(line);

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

        std::cout << "PARSE COMPLETE, RESULT: \n";
        std::cout << network << "\n";
    }

    void v1() const override {
        reportSolution(0);
    }

    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        instructions = Instructions();
        network = Network();
    }

private:
    Instructions instructions;
    Network network;
};

#undef DAY