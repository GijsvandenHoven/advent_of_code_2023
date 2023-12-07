// created retroactively to test the new templates. Duplicate of day_4_better.cpp

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 4

/**
 * This is made because while 4 works, its obviously better to parse the input into structs first.
 * This solution is a proof of concept of this, it should show off less code, which is more readable and structured better too.
 */

struct ScratchCard {
    std::vector<int> winningNumbers;
    std::vector<int> yourNumbers;

    ScratchCard() = delete;
    explicit ScratchCard(const std::string& from) {
        auto start_of_winners = from.find(':');
        auto start_of_mine = from.find('|');

        // '+1' to offset for the token itself. This makes parsing into ints easier.
        auto winString = from.substr(start_of_winners + 1, start_of_mine - (start_of_winners + 1));
        auto myString = from.substr(start_of_mine + 1);

        auto tokenize = [](const std::string& from, std::vector<int>& into) {
            std::istringstream s(from);
            int x;
            while (s >> x) {
                into.push_back(x);
            }
        };

        tokenize(winString, winningNumbers);
        tokenize(myString, yourNumbers);
    }

    [[nodiscard]] int n_wins() const {
        int wins = 0;
        for (auto& number : yourNumbers) {
            for (auto& winner : winningNumbers) {
                if (number == winner) {
                    wins ++;
                    break;
                }
            }
        }
        return wins;
    }

    [[nodiscard]] int score() const {
        return (1 << n_wins()) / 2;
    }
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void v1(std::ifstream& input) override {
        std::vector<ScratchCard> cards;
        parseIntoVector(input, cards);

        int totalScore = std::accumulate(cards.begin(), cards.end(), 0, [](int sum, auto& c){
            return sum + c.score();
        });

        reportSolution(totalScore);
    }

    void v2(std::ifstream& input) override {
        std::vector<ScratchCard> cards;
        parseIntoVector(input, cards);

        std::vector<int> instancesOfCard;
        instancesOfCard.resize(cards.size(), 1);
        int index = 0;
        std::for_each(cards.begin(), cards.end(), [&instancesOfCard, &index](auto& c) {
            int wins = c.n_wins();
            for (int i = index+1; i < index+1 + wins && i < instancesOfCard.size(); ++i) {
                instancesOfCard[i] += instancesOfCard[index];
            }

            index++;
        });

        int sum = std::accumulate(instancesOfCard.begin(), instancesOfCard.end(), 0);
        reportSolution(sum);
    }

private:
    static void parseIntoVector(std::ifstream &in, std::vector<ScratchCard> &out) {
        std::string line;
        while (std::getline(in, line)) {
            ScratchCard c(line);
            out.push_back(c);
        }
    }
};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY