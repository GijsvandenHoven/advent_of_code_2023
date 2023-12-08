#pragma once

#include <iostream>
#include <cmath>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 6

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream& input) override {
        int c;
        while((c = input.get()) != EOF && c != ':')
            ;

        {
            int n;
            while (input >> n) {
                race_times.push_back(n);
            }
            input.clear();
        }

        while((c = input.get()) != EOF && c != ':')
            ;

        {
            int n;
            while (input >> n) {
                race_distances.push_back(n);
            }
            input.clear();
        }

        if (race_distances.size() != race_times.size()) throw std::logic_error("Race and Dist number vectors should have equal size");
    }

    void v1() const override {

        std::vector<int> wins_per_game;
        wins_per_game.reserve(race_distances.size());
        for (int i = 0; i < race_distances.size(); ++i) {
            auto time = static_cast<double>(race_times[i]);
            auto dist = static_cast<double>(race_distances[i]);

            auto [lower_bound, upper_bound] = find_integer_zeroes(time, dist + 0.000'001); // we want to find GREATER than distance.

            int ways_to_win = upper_bound - lower_bound + 1;
            wins_per_game.push_back(ways_to_win);
        }

        int64_t product = std::accumulate(wins_per_game.begin(), wins_per_game.end(), 1ll, [](int64_t s, int x) {
            return s * x;
        });
        reportSolution(product);
    }

    void v2() const override {

        auto kerning = [](auto& vec) {
            int64_t kerned = 0;
            for (auto v : vec) {
                int64_t mul = 1;
                auto w = v;
                while (w > 0) { mul *= 10; w /= 10; }

                kerned *= mul;
                kerned += v;
            }

            return kerned;
        };

        int64_t kerned_time = kerning(race_times);
        int64_t kerned_dist = kerning(race_distances);

        // 48 bits (!) on the kerned puzzle input, close to integer inaccuracy point of 53 bits on double.
        auto [low, hi] = find_integer_zeroes(static_cast<double>(kerned_time), static_cast<double>(kerned_dist) + 0.1); // bigger epsilon due to size.

        reportSolution(hi - low + 1);
    }

private:
    std::vector<int64_t> race_times;
    std::vector<int64_t> race_distances;

    static std::pair<int, int> find_integer_zeroes(double time, double dist) {
        // equation: dist = speed * (time-speed),  find zeroes to see tipping point of win/lose.
        // equation: 0 = speed * (time-speed) - dist, find zeroes.
        // equation: 0 = -speed^2 + speed * time - dist.    let speed=s, let time=t, let dist=d.
        // equation: 0 = -s^2 + ts - d.
        // equation: D = t^2 - 4 * -1 * -d     (b^2 - 4ac)
        // zeroes are at (sqrt(D) +/- -t) / 2*-1)

        double sqrtD = std::sqrt(static_cast<double>(time * time) - 4 * dist); // b^2 - (4ac but double - removed)

        double z1 = (sqrtD + static_cast<double>(-time)) / (-2);
        double z2 = (sqrtD - static_cast<double>(-time)) / (-2);

        int lower_bound = std::ceil(std::abs(z1));
        int upper_bound = std::floor(std::abs(z2));

        return std::make_pair(lower_bound, upper_bound);
    }

    void parseInput(std::ifstream& input) {
        int c;
        while((c = input.get()) != EOF && c != ':')
            ;

        {
            int n;
            while (input >> n) {
                race_times.push_back(n);
            }
            input.clear();
        }

        while((c = input.get()) != EOF && c != ':')
            ;

        {
            int n;
            while (input >> n) {
                race_distances.push_back(n);
            }
            input.clear();
        }

        if (race_distances.size() != race_times.size()) throw std::logic_error("Race and Dist number vectors should have equal size");
    }

    void parseBenchReset() override {
        race_times.clear();
        race_distances.clear();
    }
};

#undef DAY