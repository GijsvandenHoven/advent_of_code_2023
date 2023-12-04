#ifndef ADVENT_OF_CODE_2023_BENCHSTATS_HPP
#define ADVENT_OF_CODE_2023_BENCHSTATS_HPP

#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cmath>
#include <iostream>

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

    friend std::ostream& operator<<(std::ostream& o, const BenchmarkStats& b);

private:
    std::vector<Time> all; // aligned 'temporally', i.e. earliest first, appended by measure();
    std::vector<Time> sorted; // only created if required by function calls. Transparently maintained. Do not use other than through get_sorted().

    [[nodiscard]] std::string format(const Time& value) const {

        auto converted = value / unit; // e.g. nanos to millis, this results in division by 1 million.
        auto extra = (value % unit).count(); // the stuff behind-the-comma of this division.
        auto behind_comma = std::to_string(extra * 1000).substr(0, 3);

        std::string unit_of_time;
        if (unit / std::chrono::seconds{1} != 0) {
            unit_of_time = "s";
        } else if (unit / std::chrono::milliseconds{1} != 0) {
            unit_of_time = "ms";
        } else if (unit / std::chrono::microseconds{1} != 0) {
            unit_of_time = "us"; // "μs"; // <<- encoding issue
        } else {
            unit_of_time = "ns";
        }

        return std::to_string(converted) + "." + behind_comma + " " + unit_of_time;
    }

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

public:
    // controls unit printed in operator<<. Change by assigning e.g. std::chrono::milliseconds{1}.
    Time unit{1};

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
};

std::ostream& operator<<(std::ostream& o, const BenchmarkStats& b) {
    o
    << "BenchStats {" << "\n"
    << "\tSample Size: " << b.n_samples() << "\n"
    << "\tMean (Median): " << b.format(b.mean()) << " (" << b.format(b.median()) << ")\n"
    << "\tStdDev: " << b.format(b.std_dev()) << "\n"
    << "\tlowest / highest: " << b.format(b.lowest()) << " / " << b.format(b.highest()) << "\n"
    << "\t5/95 %-ile: " << b.format(b.nth_ile(0.05)) << " / " << b.format(b.nth_ile(0.95)) << "\n"
    //<< "\tCI (95% 2-sided)" << "\n" // excessive.
    << "}";

    return o;
}

#endif //ADVENT_OF_CODE_2023_BENCHSTATS_HPP
