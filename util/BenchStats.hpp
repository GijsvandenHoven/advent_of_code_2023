#ifndef ADVENT_OF_CODE_2023_BENCHSTATS_HPP
#define ADVENT_OF_CODE_2023_BENCHSTATS_HPP

#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cmath>
#include <iostream>
// todo: cannot #include format, need g++ 13 or higher. currently on 11.

using Time = std::chrono::steady_clock::duration;

/**
 * Structure for storing stats of a "benchmark".
 *
 * This structure is append-only by way of measurement(), but can be re-used by clear().
 *
 * It implements operator<< for printing an overview of stats.
 * More specific stats can be accessed yourself through function calls such as mean() and stddev().
 * All returned values are in std::chrono::steady_clock::duration.
 * To get readable strings in other units, set 'unit' to the desired unit (e.g. std::chrono::milliseconds{1}), and call format().
 * To get a casted duration value yourself, divide the result by std::chrono::your_unit{1}.
 *
 * Maintains data temporally as well as ordinally.
 * The ordinal data is generated on-demand. That is, there is no sort unless asked for.
 * Data is sorted only once, unless more is added after a demand for sorting through measurement().
 */
class BenchmarkStats {
public:
    BenchmarkStats() : unit(1) { }
    explicit BenchmarkStats(const Time& representation_unit) : unit(representation_unit) { }

    void measurement(Time t) {
        all.push_back(t);
    }

    [[nodiscard]] size_t n_samples () const {
        return all.size();
    }

    void reset () {
        all.clear();
        sorted.clear();
    }

    void reserve(int n) { all.reserve(n); }

    [[nodiscard]] Time lowest() const {
        return * std::min_element(all.begin(), all.end());
    }

    [[nodiscard]] Time highest() const {
        return * std::max_element(all.begin(), all.end());
    }

    // assumes size > 0
    [[nodiscard]] Time mean() const {
        auto sum = std::accumulate(all.begin(), all.end(), Time{});
        return sum / all.size();
    }

    // assumes size > 0
    [[nodiscard]] Time median() const {
        if (all.size() % 2 == 1) {
            return all[all.size() / 2];
        } else {
            return (all[n_samples() / 2] + all[n_samples() / 2 - 1]) / 2;
        }
    }

    [[nodiscard]] Time std_dev() const {
        if (all.size() <= 1) { return Time{0}; } // 0 divided by 0 otherwise, it's a bad time.

        auto x = mean();
        Time::rep squaredSum = 0;
        for (auto& s : all) {
            squaredSum += (s - x).count() * (s - x).count();
        }
        auto result = std::sqrt(squaredSum / (n_samples() - 1));
        return Time {static_cast<decltype(squaredSum)>(result) };
    }

    // assumes 0 < ile < 1
    [[nodiscard]] Time nth_ile(double ile) const {
        return get_sorted()[static_cast<int>(n_samples() * ile)]; // NOLINT(cppcoreguidelines-narrowing-conversions) -- if you have 2^53 measurements you have bigger problems.
    }

private:
    friend std::ostream& operator<<(std::ostream& o, const BenchmarkStats& b);

    Time unit; // controls unit printed in operator<<. Change by assigning e.g. std::chrono::milliseconds{1}.
    std::vector<Time> all; // aligned 'temporally', i.e. earliest first, appended by measure();
    std::vector<Time> sorted; // only created if required by function calls. Transparently maintained. Do not use other than through get_sorted().

    [[nodiscard]] const std::vector<Time>& get_sorted() const {
        /** Bad To the Bone Riff */
        auto sorted_ptr = const_cast<std::vector<Time>*>(&sorted);
        /** It's rule-breaking time */
        if (all.size() > sorted.size()) { // because the class is append-only, this is the only way a sort could be invalidated.
            *sorted_ptr = all;
            std::sort(sorted_ptr->begin(), sorted_ptr->end());
        }

        return sorted;
    }

    friend int benchEverything();
    // absolute mess of code, it keeps breaking I hate this.
    [[nodiscard]] std::string format(const Time& value) const {
        if (value.count() == 0) { // 0 will result in infinite loops when upgrading/downgrading displayed time unit. Might as well exit early and just say it's zero.
            return "0 ns";
        }

        double time_units = static_cast<double>(value.count()) / static_cast<double>(unit.count());

        std::string unit_of_time;
        const char* time_suffixes[4] = { "s", "ms", "us", "ns" }; // "μs"; // <<- encoding issue
        int unit_index;
        if (unit / std::chrono::seconds{1} != 0) {
            unit_index = 0;
        } else if (unit / std::chrono::milliseconds{1} != 0) {
            unit_index = 1;
        } else if (unit / std::chrono::microseconds{1} != 0) {
            unit_index = 2;
        } else {
            unit_index = 3;
        }

        if (time_units < 1.0) {
            // handles using smaller units if the time was smaller than the expected unit.
            while (time_units < 1.0) {
                unit_index = std::min(3, unit_index + 1);
                time_units *= 1000;
            }
        } else if (time_units >= 1000.0) {
            // handles using bigger units if the time was larger than the expected unit.
            while (time_units > 1000.0) {
                unit_index = std::max(0, unit_index - 1);
                time_units /= 1000;
            }
        }

        unit_of_time = time_suffixes[unit_index];

        if (unit_of_time == "s" && time_units >= 1000) {
            std::string formatted = std::to_string(time_units);
            auto dot_index = formatted.find('.');
            return formatted.substr(0, dot_index) + " " + unit_of_time;
        }

        // no std::format on this compiler, so we must suffer.
        std::string formatted = std::to_string(time_units).substr(0, 4);
        if (formatted[3] == '.') formatted = formatted.substr(0, 3);
        return formatted + " " + unit_of_time;
    }
};

inline std::ostream& operator<<(std::ostream& o, const BenchmarkStats& b) {
    o
    << "BenchStats {" << "\n"
    << "\tSample Size: " << b.n_samples() << "\n"
    << "\tMean (Median): " << b.format(b.mean()) << " (" << b.format(b.median()) << ")\n"
    << "\tStdDev: " << b.format(b.std_dev()) << "\n"
    << "\tlowest / highest: " << b.format(b.lowest()) << " / " << b.format(b.highest()) << "\n"
    << "\t5/95 %-ile: " << b.format(b.nth_ile(0.05)) << " / " << b.format(b.nth_ile(0.95)) << "\n" // might have mixed up the definition of %-ile, maybe the labels should be swapped. oh well.
    //<< "\tCI (95% 2-sided)" << "\n" // excessive.
    << "}";

    return o;
}

#endif //ADVENT_OF_CODE_2023_BENCHSTATS_HPP
