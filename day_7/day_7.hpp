#pragma once

#include <iostream>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 7

// ordered from weakest to strongest, for use in sorting with operator<.
enum class HandType : int8_t {
    SADNESS = 00,
    ONE_PAIR = 10,
    TWO_PAIR = 20,
    THREE_KIND = 30,
    FULL_HOUSE = 40,
    FOUR_KIND = 50,
    FIVE_KIND = 60
};

constexpr int JOKER_VALUE = 1;

struct Hand {
    int wager = 0;
    std::array<int8_t, 5> cards {};
    HandType power;

    Hand() = delete;
    explicit Hand(const std::string& input, bool use_jokers) { // will hilariously break if a non-single-digit or TJQKA card exists.
        std::istringstream scanner(input);

        auto char_to_card_value = [use_jokers](int c) -> int8_t {
            switch(c) {
                case 'T': return 10;
                case 'J': return use_jokers ? JOKER_VALUE : 11;
                case 'Q': return 12;
                case 'K': return 13;
                case 'A': return 14;
                default: return static_cast<int8_t>(c - '0');
            }
        };

        // used for calculations of card type, later. Counts # of occurences of a card in the hand.
        // Index maps to card number directly.
        // Undefined value in index 0. (There are no cards with value 0)
        std::array<int, 15> buckets {};
        // assign cards
        for (auto& card : cards) {
            card = char_to_card_value(scanner.get());

            if (card == JOKER_VALUE) { // a joker can be anything, so increment every card...
                for (auto& bucket : buckets) { bucket++; }
            } else {
                buckets[card]++;
            }
        }
        // assign wager, do this after card reading as the scanner needs to be past the card description!
        scanner >> wager;

        // calculate card type. Count occurence numbers. e.g. 2x2 -> two_pair. 3 & 2 -> full_house. Need to count 0 to simplify the mapping.
        std::array<int, 6> counts {};
        std::for_each(buckets.begin(), buckets.end(), [&counts](auto occurences) {
            counts[occurences]++;
        });

        if (counts[5] != 0) {
            power = HandType::FIVE_KIND;
        } else if (counts[4] != 0) {
            power = HandType::FOUR_KIND;
        } else if (counts[3] != 0) {
            power = Hand::MaybeFullHouse(counts, buckets); // full house, or just 3-of-a-kind? Also considers jokers, if any.
        } else if (counts[2] != 0) {
            power = Hand::MaybeTwoPair(counts, buckets); // two pair, or just one pair? Also considers jokers, if any.
        } else {
            power = HandType::SADNESS;
        }
    }

private:
    static HandType MaybeFullHouse(const std::array<int, 6> &counts, const std::array<int, 15> &buckets) {
        // If there is no 2 pair or multiple 3 pair (jokers can cause this), it's just 3 of a kind.
        if (counts[2] == 0 && counts[3] < 2) {
            return HandType::THREE_KIND;
        }

        // if there are jokers, a 2-pair was counted from a joker.
        // The joker must already be counted in the 3 pair, so this is not a full house.
        if (buckets[JOKER_VALUE] != 0) {
            // unless there are two 'three-pairs'. That would imply JXXYY -> J matches X or Y, leaving a real 2-pair.
            return counts[3] == 2 ? HandType::FULL_HOUSE : HandType::THREE_KIND;
        } else {
            return HandType::FULL_HOUSE;
        }
    }

    static HandType MaybeTwoPair(const std::array<int, 6> &counts, const std::array<int, 15> &buckets) {
        if (counts[2] < 2) { // if there's no 2 pairs even with jokers, it's just not two pairs.
            return HandType::ONE_PAIR;
        }

        // If there are jokers, and there are no triplets or higher (hence we are in this function),
        // Then a pair was counted while including a joker.
        // But, every pair is then counted as a joker. E.G. J2345 -> 5 pairs.
        // It is not possible to have a legit 2-pair while a joker is present in this function, because 3-pair and higher is already ruled out.
        // e.g. a legit 2-pair 33JXY (33 and JX) would be a 3-pair instead. (33J)
        if (buckets[JOKER_VALUE] != 0) {
            return HandType::ONE_PAIR;
        } else {
            return HandType::TWO_PAIR;
        }
    }
};

bool operator<(const Hand& a, const Hand& b) {
    if (a.power != b.power) {
        return a.power < b.power;
    } else {
        int i = 0;
        while (a.cards[i] == b.cards[i]) {
            ++i;
        }

        // i == size implies hands are exactly equal.
        return i != a.cards.size() && a.cards[i] < b.cards[i];
    }
}

std::ostream& operator<<(std::ostream& os, const Hand& h) {
    os  << "Hand {\n\t"
        << "cards: "
            << static_cast<int>(h.cards[0]) << ", "
            << static_cast<int>(h.cards[1]) << ", "
            << static_cast<int>(h.cards[2]) << ", "
            << static_cast<int>(h.cards[3]) << ", "
            << static_cast<int>(h.cards[4]) << "\n\t" // ¯\_(ツ)_/¯
        << "wager: " << h.wager << "\n\t"
        << "type: " << static_cast<int>(h.power) << "\n}";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    // If we sort the hands based on a custom operator< we can get ther rank of each.
    void v1(std::ifstream& input) override {
        parseInput(input, false);

        solveProblem();
    }

    void v2(std::ifstream& input) override {
        parseInput(input, true);

        solveProblem();
    }

    void solveProblem() {
        std::sort(hands.begin(), hands.end());

        // std::for_each(hands.begin(), hands.end(), [](auto& c) {std::cout <<c << "\n\n";});

        int rank_counter = 1;
        auto accumulator = [&rank_counter](int64_t s, const Hand& c) -> int64_t {
            return s + (rank_counter++) * c.wager;
        };
        int64_t rank_times_wager = std::accumulate(hands.begin(), hands.end(), 0LL, accumulator);

        reportSolution(rank_times_wager);
    }

private:
    std::vector<Hand> hands;

    void parseInput(std::ifstream& input, bool use_jokers) {
        std::string line;
        while (std::getline(input, line)) {
            Hand h(line, use_jokers);
            hands.emplace_back(h);
        }
    }

    void reset() override {
        hands.clear();
        Day::reset();
    }
};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY