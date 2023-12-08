#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 4

/**
 * Retroactively added to this template, used to be a lone int main() file.
 * Before the 'new template' was made, there was also a second solution to improve on the original,
 * which is contained in the function bodies as seen here.
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

    void parse(std::ifstream& input) override {
        std::string line;
        while (std::getline(input, line)) {
            ScratchCard c(line);
            cards.push_back(c);
        }
    }

    void v1() const override {
        int totalScore = std::accumulate(cards.begin(), cards.end(), 0, [](int sum, auto& c){
            return sum + c.score();
        });

        reportSolution(totalScore);
    }

    void v2() const override {
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

    void parseBenchReset() override {
        cards.clear();
    }

private:
    std::vector<ScratchCard> cards;
};

#undef DAY