#pragma once

#include <iostream>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 2

/**
 * Retroactively added to this template, used to be a lone int main() file.
 * The original consumed the ifstream directly, which is hard to replicate with immutability.
 * To do this, we parse the input into one big string, and use a stringstream on it in the solvers.
 */

struct GameConstraints {
    int16_t red, green, blue;

    [[nodiscard]] int power() const { return red * green * blue; }
    void updateMin(const GameConstraints& other) {
        red = std::max(red, other.red);
        blue = std::max(blue, other.blue);
        green = std::max(green, other.green);
    }
};

bool operator>(const GameConstraints& a, const GameConstraints& b) {
    return a.red > b.red || a.green > b.green || a.blue > b.blue;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {
        std::ostringstream s;
        s << input.rdbuf();
        entire_input_string = s.str();
    }

    void v1() const override {
        std::istringstream text(entire_input_string);

        int game_id = 1;
        int game_id_sum = 0;

        std::string game;
        while (std::getline(text, game)) {
            bool legal_game = true;
            size_t game_start_index = game.find(':');
            while (game_start_index != std::string::npos) {
                size_t game_end_index = std::min(game.size(), game.find(';', game_start_index + 1));

                legal_game &= checkGameRound(game_start_index, game_end_index, game);

                game_start_index = game.find(';', game_start_index + 1);
            }
            game_id_sum += legal_game * game_id;
            game_id++;
        }

        reportSolution(game_id_sum);
    }

    void v2() const override {
        std::istringstream text(entire_input_string);
        std::string game;
        int powerSum = 0;
        while (std::getline(text, game)) {
            size_t game_start_index = game.find(':');
            GameConstraints minima = {0};
            while (game_start_index != std::string::npos) {
                size_t game_end_index = std::min(game.size(), game.find(';', game_start_index + 1));

                updateMinima(game_start_index, game_end_index, game, minima);

                game_start_index = game.find(';', game_start_index + 1);
            }

            powerSum += minima.power();
        }

        reportSolution(powerSum);
    }

    void parseBenchReset() override {
        entire_input_string.clear(); // should be redundant since the string is assigned to in parse.
    }

private:
    std::string entire_input_string;
    GameConstraints CONSTRAINTS = GameConstraints { 12, 13, 14 };

    static void updateMinima(size_t start, size_t end, const std::string &game, GameConstraints &minima) {
        GameConstraints observed {0};
        int16_t accumulator = 0;
        for (size_t i = start; i < end; ++i) {
            char read = game[i];
            switch (read) {
                // r, g, b was read: Store accumulator in the right constraint of this round.
                // The accumulator should also be reset, for the next number read.
                // but also because 'GReen' would add non-zero to red if you don't immediately.
                case 'r':
                    observed.red = static_cast<int16_t>(observed.red + accumulator);
                    accumulator = 0;
                    break;
                case 'b':
                    observed.blue = static_cast<int16_t>(observed.blue + accumulator);
                    accumulator = 0;
                    break;
                case 'g':
                    observed.green = static_cast<int16_t>(observed.green + accumulator);
                    accumulator = 0;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    accumulator *= 10; // shift existing digits
                    accumulator = static_cast<int16_t>(accumulator + (read - '0')); // add new one.
                    break;
                default: break;
            }
        }

        // update minima:
        minima.updateMin(observed);
    }

    /**
     * Given a start and end index, seeks the game string to read red & green & blue values.
     * It accumulates these values and compares it against the global constraints.
     * @param start start index of search space
     * @param end end index of search spage
     * @param game string to seek.
     * @return whether the game was possible according to the global constraints variable.
     */
    bool checkGameRound(size_t start, size_t end, const std::string& game) const {
        GameConstraints g = {0};

        int16_t accumulator = 0;
        for (size_t i = start; i < end; ++i) {
            char read = game[i];
            switch (read) {
                // r, g, b was read: Store accumulator in the right constraint of this round.
                // The accumulator should also be reset, for the next number read.
                // but also because 'GReen' would add non-zero to red if you don't immediately.
                case 'r':
                    g.red = static_cast<int16_t>(g.red + accumulator);
                    accumulator = 0;
                    break;
                case 'b':
                    g.blue = static_cast<int16_t>(g.blue + accumulator);
                    accumulator = 0;
                    break;
                case 'g':
                    g.green = static_cast<int16_t>(g.green + accumulator);
                    accumulator = 0;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    accumulator *= 10; // shift existing digits
                    accumulator = static_cast<int16_t>(accumulator + (read - '0')); // add new one.
                    break;
                default: break;
            }
        }

        return ! (g > CONSTRAINTS);
    }
};

#undef CONCATENATE
#undef CLASS_DEF
#undef DEFAULT_CTOR_DEF
#undef DAY