#define STR(x) #x
#define FILE_PATH(num)  "day_" STR(num) "/day" STR(num) ".txt"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>

#define DAY 4
#define FILE FILE_PATH(DAY)

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

namespace v1 {
    int solve(std::ifstream& text);
    void parseIntoVector(std::ifstream& in, std::vector<ScratchCard>& out);
}

namespace v2 {
    int solve(std::ifstream& text);
}

void v1::parseIntoVector(std::ifstream &in, std::vector<ScratchCard> &out) {
    std::string line;
    while (std::getline(in, line)) {
        ScratchCard c(line);
        out.push_back(c);
    }
}

int v1::solve(std::ifstream& text) {
    std::vector<ScratchCard> cards;
    v1::parseIntoVector(text, cards);

    int totalScore = std::accumulate(cards.begin(), cards.end(), 0, [](int sum, auto& c){
        return sum + c.score();
    });

    return totalScore;
}

int v2::solve(std::ifstream& text) {
    std::vector<ScratchCard> cards;
    v1::parseIntoVector(text, cards);

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
    return sum;
}

int main () {

    {
        std::ifstream input(FILE);
        auto result = v1::solve(input);
        std::cout << "v1: " << result << "\n";
    }
    {
        std::ifstream input(FILE);
        auto result = v2::solve(input);
        std::cout << "v2: " << result << "\n";
    }

    return 0;
}