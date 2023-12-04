#define STR(x) #x
#define FILE_PATH(num)  "day_" STR(num) "/day" STR(num) ".txt"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#define DAY 4
#define FILE FILE_PATH(DAY)

enum class ReadingMode : uint8_t {
    JUNK = 0,
    WINNING_NUMBERS,
    YOUR_NUMBERS
};

namespace v1 {
    int solve(std::ifstream& text);
    void commitNumber(ReadingMode mode, int number, int& score, std::vector<int>& winningNumbers);
}

namespace v2 {
    int solve(std::ifstream& text);
    void commitNumber(ReadingMode mode, int number,int& score, std::vector<int>& winningNumbers);
    void updateCardCount(std::vector<int>& multiples_of_card, int score, int cardNumber);
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

int v1::solve(std::ifstream& text) {
    ReadingMode mode = ReadingMode::JUNK;
    std::vector<int> winningNumbers;
    winningNumbers.reserve(10);
    int c;
    int score = 0;
    int scoreTotal = 0;
    int parsedNumber = 0;
    do {
        c = text.get();
        switch (c) {
            case '\n':
            case EOF:
                // handle the final number
                commitNumber(mode, parsedNumber, score, winningNumbers);
                // round reset
                winningNumbers.clear();
                scoreTotal += score;
                score = 0;
                parsedNumber = 0;
                mode = ReadingMode::JUNK;
                break;
            case ':':
                mode = ReadingMode::WINNING_NUMBERS;
                parsedNumber = 0;
                break;
            case '|':
                mode = ReadingMode::YOUR_NUMBERS;
                parsedNumber = 0;
                break;
            case ' ':
                v1::commitNumber(mode, parsedNumber, score, winningNumbers);
                parsedNumber = 0;
                break;
            case '0' ... '9':
                parsedNumber *= 10;
                parsedNumber += c - '0';
                break;
            default:
                break;
        }
    } while (c != EOF);

    return scoreTotal;
}

void v1::commitNumber(ReadingMode mode, int number, int &score, std::vector<int> &winningNumbers) {
    // Fortunately 0 is not part of the inputs. This means double spaces or spaces after :, |, do not break anything.
    if (number == 0) {
        return;
    }

    switch (mode) {
        case ReadingMode::WINNING_NUMBERS:
            winningNumbers.push_back(number);
            break;
        case ReadingMode::YOUR_NUMBERS: {
            auto iter = std::find(winningNumbers.begin(), winningNumbers.end(), number);
            if (iter != winningNumbers.end()) {
                score = std::max(1, score * 2);
            }
            break;
        }
        case ReadingMode::JUNK:
        default:
            break;
    }
}


int v2::solve(std::ifstream& text) {
    ReadingMode mode = ReadingMode::JUNK;
    std::vector<int> round_winning_numbers;
    round_winning_numbers.reserve(10);
    std::vector<int> multiples_of_card;
    multiples_of_card.push_back(1); // first card is always 1x.

    int c;
    int score = 0;
    int cardNumber = 0;
    int parsedNumber = 0;

    auto ensure_this_card_exists = [&multiples_of_card](int cardNumber){
        if (cardNumber >= multiples_of_card.size()) {
            multiples_of_card.push_back(1);
        }
    };

    do {
        c = text.get();
        switch (c) {
            case '\n':
                // handle the final number
                v2::commitNumber(mode, parsedNumber, score, round_winning_numbers);
                ensure_this_card_exists(cardNumber + 1);
                // handle scoring
                v2::updateCardCount(multiples_of_card, score, cardNumber);
                // round reset
                round_winning_numbers.clear();
                cardNumber += 1;
                score = 0;
                parsedNumber = 0;
                mode = ReadingMode::JUNK;
                break;
            case EOF:
                // by definition of the problem, the last card is always a losing card.
                // "Will never win copies past the table".
                ensure_this_card_exists(cardNumber);
                break;
            case ':':
                mode = ReadingMode::WINNING_NUMBERS;
                parsedNumber = 0;
                break;
            case '|':
                mode = ReadingMode::YOUR_NUMBERS;
                parsedNumber = 0;
                break;
            case ' ':
                v2::commitNumber(mode, parsedNumber, score, round_winning_numbers);
                parsedNumber = 0;
                break;
            case '0' ... '9':
                parsedNumber *= 10;
                parsedNumber += c - '0';
                break;
            default:
                break;
        }
    } while (c != EOF);

    int sum = 0;
    for (int card : multiples_of_card) {
        sum += card;
    }

    return sum;
}

void v2::commitNumber(ReadingMode mode, int number, int& score, std::vector<int> &winningNumbers) {
    // Fortunately 0 is not part of the inputs. This means double spaces or spaces after :, |, do not break anything.
    if (number == 0) {
        return;
    }

    switch (mode) {
        case ReadingMode::WINNING_NUMBERS:
            winningNumbers.push_back(number);
            break;
        case ReadingMode::YOUR_NUMBERS: {
            auto iter = std::find(winningNumbers.begin(), winningNumbers.end(), number);
            if (iter != winningNumbers.end()) {
                score += 1;
            }
            break;
        }
        case ReadingMode::JUNK:
        default:
            break;
    }
}

void v2::updateCardCount(std::vector<int> &multiples_of_card, int score, int cardNumber) {
    size_t last_card_index = multiples_of_card.size() - 1;
    int instances_of_this_card = multiples_of_card[cardNumber];

    for (int index = cardNumber + 1; index < cardNumber + 1 + score; ++index) {
        if (index > last_card_index) {
            multiples_of_card.push_back(1); // this card is not yet represented, so add it and say it exists once.
        }

        multiples_of_card[index] += instances_of_this_card;
    }
}