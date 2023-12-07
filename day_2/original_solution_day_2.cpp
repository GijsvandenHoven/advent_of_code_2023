#include <iostream>
#include <fstream>

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

namespace v1 {
    int solve(std::ifstream &input);
    [[nodiscard]] bool checkGameRound(size_t start, size_t end, const std::string &game);
    auto CONSTRAINTS = GameConstraints { 12, 13, 14 };
}

namespace v2 {
    int solve(std::ifstream &input);
    void updateMinima(size_t start, size_t end, const std::string& game, GameConstraints& minima);
}

int main() {

    {
        std::ifstream text("day_2/day2.txt");
        int solution = v1::solve(text);
        std::cout << "v1: " << solution << "\n";
    }
    {
        std::ifstream text("day_2/day2.txt");
        int solution = v2::solve(text);
        std::cout << "v2: " << solution << "\n";
    }

    return 0;
}

int v2::solve(std::ifstream& input) {
    std::string game;
    int powerSum = 0;
    while (std::getline(input, game)) {
        size_t game_start_index = game.find(':');
        GameConstraints minima = {0};
        while (game_start_index != std::string::npos) {
            size_t game_end_index = std::min(game.size(), game.find(';', game_start_index + 1));

            v2::updateMinima(game_start_index, game_end_index, game, minima);

            game_start_index = game.find(';', game_start_index + 1);
        }

        powerSum += minima.power();
    }

    return powerSum;
}

void v2::updateMinima(size_t start, size_t end, const std::string &game, GameConstraints &minima) {
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

int v1::solve(std::ifstream& input) {
    int game_id = 1;
    int game_id_sum = 0;

    std::string game;
    while (std::getline(input, game)) {
        bool legal_game = true;
        size_t game_start_index = game.find(':');
        while (game_start_index != std::string::npos) {
            size_t game_end_index = std::min(game.size(), game.find(';', game_start_index + 1));

            legal_game &= v1::checkGameRound(game_start_index, game_end_index, game);

            game_start_index = game.find(';', game_start_index + 1);
        }
        game_id_sum += legal_game * game_id;
        game_id++;
    }

    return game_id_sum;
}

/**
 * Given a start and end index, seeks the game string to read red & green & blue values.
 * It accumulates these values and compares it against the global constraints.
 * @param start start index of search space
 * @param end end index of search spage
 * @param game string to seek.
 * @return whether the game was possible according to the global constraints variable.
 */
bool v1::checkGameRound(size_t start, size_t end, const std::string& game) {
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

    return ! (g > v1::CONSTRAINTS);
}