#pragma once

#include <iostream>
#include <omp.h>
#include <syncstream>

#include "../util/Day.hpp"

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) CONCATENATE(Day, D) () : Day(D) {}

#define DAY 5

struct Range;

// std::pair<bool, T> is the poor man's monad.
using Monad = std::pair<bool, int64_t>;
using RangeMonad = std::pair<bool, Range>;

Monad Ok(int64_t v) { return std::make_pair(true, v); }
Monad Fail() { return std::make_pair(false, 0xDEAD'BEEF'DEAD'BEEF); }

using IntRemapper = std::function<Monad(int64_t)>;

void assert(bool _, const std::string& why = "unspecified") { if (!_) throw std::logic_error(why); }

class NumberMapper {
    std::vector<IntRemapper> remapping_sequence;
public:
    NumberMapper() = default;

    int64_t remap(int64_t input) {
        int64_t current = input;
        std::for_each(remapping_sequence.begin(), remapping_sequence.end(), [&](auto& remapper) {
            auto remap = remapper(current);
            if (remap.first) {
                current = remap.second;
            }
        });

        return current;
    }

    void add_new_remapper(IntRemapper && f) {
        remapping_sequence.emplace_back(f);
    }

    /**
     * given a function f, pops the function 'g' from the vector, and pushes a function h, such that h = f(g()).
     */
    void extend_last_remapper(const IntRemapper& f) {
        assert(! remapping_sequence.empty(), "extending empty remapper.");

        IntRemapper& current = remapping_sequence.back();

        // not so sure about ownership and lifetimes of these function objects, so I think it's safest to copy them.
        IntRemapper composed = [f, current](int64_t val) -> Monad {
            auto optionA = current(val);
            auto optionB = f(val);

            if (optionA.first) return optionA;
            if (optionB.first) return optionB;

            // Neither mapping covers this value, so it is simply not remapped.
            return Fail();
        };

        remapping_sequence.pop_back();
        remapping_sequence.emplace_back(std::move(composed));
    }
};

struct Range { // ranges are inclusive on both ends.
    int64_t start;
    int64_t end;

    Range() = delete;
    Range(int64_t s, int64_t e) {
        assert(s <= e, "Range must have end not greater than start. - " + std::to_string(s) + ", " + std::to_string(e));
        start = s;
        end = e;
    }

    [[nodiscard]] RangeMonad intersect(const Range& other) const {
        // no overlap
        if (other.end < start || other.start > end) return std::make_pair<bool, Range>(false, { 0, 0 });

        // full overlap
        if (other.start <= start && other.end >= end) return std::make_pair<bool, Range>(true, { start, end });
        if (other.start >= start && other.end <= end) return std::make_pair<bool, Range>(true, { other.start, other.end });

        // partial overlap.
        if (other.start <= start) return std::make_pair<bool, Range>(true, { start, other.end });
        if (other.end >= end) return std::make_pair<bool, Range>(true, { other.start, end });

        throw std::logic_error( " Impossible case reached " );
    }
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void v1(std::ifstream& input) override {
        parseInput(input);

        std::istringstream seed_reader(seed_string_numbers);

        int64_t seed;
        int64_t lowest = std::numeric_limits<int64_t>::max();
        while (seed_reader >> seed) {
            int64_t result = remapper.remap(seed);

            if (result < lowest) {
                lowest = result;
            }
        }

        reportSolution(lowest);
    }

    void v2(std::ifstream& input) override {
        parseInput(input);

        std::istringstream seed_reader(seed_string_numbers);

        std::vector<std::pair<int64_t, int64_t>> seed_groups;

        int64_t start_seed;
        int64_t seed_range;
        while (seed_reader >> start_seed >> seed_range) {
            seed_groups.emplace_back(start_seed, seed_range);
        }

        // big block of memory for each of the groups.
        std::vector<std::vector<int64_t>> results;
        std::for_each(seed_groups.begin(), seed_groups.end(), [&results](auto pair) {
            results.emplace_back();
            results.back().reserve(pair.second);
        });
        std::cout << "alloc over\n";
// todo: static schedule on inner loop instead.
#pragma omp parallel for schedule(dynamic) shared(seed_groups, results, std::cout) default(none)
        for (int i = 0; i < seed_groups.size(); ++i) {
            auto& result_group = results[i];
            auto& [seed, range] = seed_groups[i];

            std::osyncstream(std::cout) << "Thread id " << omp_get_thread_num() << " Shall cover index " << i << " Which has " << range << " Units of work.\n";

            int64_t iter_count = 0;
            for (int64_t s = seed; s < seed + range; ++s) {
                result_group.emplace_back(remapper.remap(s));

                if (iter_count % 1'000'000 == 0) {
                    std::osyncstream(std::cout) << "Thread id " << omp_get_thread_num() << " reports progress " << iter_count << " / " << range << " (" << static_cast<double>(iter_count) / range << ")\n";
                }
                iter_count++;
            }
        }

        auto total = std::accumulate(results.begin(), results.end(), 0, [](auto s, auto& v) { return s + v.size(); });
        std::cout << "At the end of it all\n";
        std::cout << "There are " << results.size() << " result vectors, representing " << total << " items \n";

        int64_t lowest = std::numeric_limits<int64_t>::max();
#pragma omp parallel for schedule(dynamic) shared(results, lowest, std::cout) default(none)
        for (int i = 0; i < results.size(); ++i) {
            auto& group_results = results[i];
            std::osyncstream(std::cout) << "Thread id " << omp_get_thread_num() << " will min_element index " << i << ".\n";
            auto lowest_of_this = std::min_element(group_results.begin(), group_results.end());

            std::osyncstream(std::cout) << "Thread id " << omp_get_thread_num() << " reports lowest " << *lowest_of_this << "\n";

#pragma omp critical(assign_min)
            {
                if (*lowest_of_this < lowest) {
                    lowest = *lowest_of_this;
                }
            }
        }

        reportSolution(lowest);
    }

private:
    NumberMapper remapper;

    std::string seed_string_numbers;

    void parseInput(std::ifstream& input) {
        std::string line;
        std::getline(input, line);

        seed_string_numbers = line.substr(7); // skip past "seeds: "

        // lambda to de-duplicate the reading from std::istringstream and std::ifstream.
        auto read_three_int64 = [](std::basic_istream<char>& stream){
            int64_t first = -1;
            int64_t second = -1;
            int64_t third = -1;

            stream >> first >> second >> third;

            assert((! stream.bad()) && first >= 0 && second >= 0 && third >= 0, "Input reading fail, expected 3 ints.");

            stream.get(); // consume newline character (or gets EOF, but that's fine).

            return std::make_tuple(first, second, third);
        };

        while(std::getline(input, line)) {
            int64_t src_start, src_end, remap;

            // either add_new or extend_last must be called on remapper.
            // So we let the if/else branches assign to this lambda and call after the functor is constructed.
            // This enables us to avoid duplication in the creation of the functor; otherwise we would have to do it in both branches of the if/else.
            std::function<void(IntRemapper && functor)> add_or_compose;

            if (line.empty()) {
                // a blank line begins definition of a new mapping. The end of the file does not have a blank line.
                std::getline(input, line); // consume the 'header' specifying a name that we do not care about. Input remappings are linear.

                auto [dest, src, len] = read_three_int64(input);
                src_start = src;
                src_end = src + len - 1;
                remap = dest - src;

                add_or_compose = [this](auto && functor) {
                    remapper.add_new_remapper(std::forward<decltype(functor)>(functor));
                };

            } else { // the line that was just extracted should contain 3 numbers.
                std::istringstream line_reader(line);

                auto [dest, src, len] = read_three_int64(line_reader);
                src_start = src;
                src_end = src + len - 1;
                remap = dest - src;

                add_or_compose = [this](auto && functor) {
                    remapper.extend_last_remapper(functor);
                };
            }

            auto map_function = [src_start, src_end, remap](int64_t val) -> Monad {
                if (val >= src_start && val <= src_end) return Ok(val + remap);

                return Fail();
            };

            add_or_compose(map_function);
        }
    }

    void reset() override {
        remapper = NumberMapper{};
        Day::reset();
    }
};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY