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

    void offset(int64_t offset) {
        start += offset;
        end += offset;

        assert(start >= 0 && end >= 0, "Offset to negative");
    }
};

bool operator==(const Range& a, const Range& b) {
    return a.start == b.start && a.end == b.end;
}

std::ostream& operator<<(std::ostream& o, const Range& r) {
    o << '(' << r.start << ", " << r.end << ')';
    return o;
}

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

#define SMART_SOLUTION true

    void v2(std::ifstream& input) override {
#if SMART_SOLUTION
        parseAsProblem2(input);

        std::istringstream seed_reader(seed_string_numbers);

        std::vector<Range> seed_groups;
        {
            int64_t start_seed;
            int64_t seed_range;
            while (seed_reader >> start_seed >> seed_range) {
                Range r {start_seed, start_seed + seed_range - 1};
                seed_groups.emplace_back(r);
            }
        }

        // for every seed range, go 'down the layers' breaking up the range into possibly more ranges through intersect().
        // this DFS appraoch does not take int account overlaps inevitably created by the seed ranges.
        // To do that, a BFS approach must be done instead with a check for pruning duplicates/overlaps.
        int64_t global_min = std::numeric_limits<int64_t>::max();
        for (auto& seed_range : seed_groups) {
            std::vector<Range> output { seed_range };
            for (auto& remap_vec : mapping_ranges) { // foreach remapping layer
                std::vector<Range> new_output;
                /**
                 * Example run: (1) vector element, input range 0-100 gets processed.
                 * There exists remappings 0-25 and 50-100.
                 * First intersect call produces (offset) range 0-25 and pushes 26-100.
                 * Second intersect call produces (offset) range 50-100 and pushes 0-49.
                 *
                 * (next iteration of this loop)
                 * 26-100 gets processed.
                 * There exists remappings 0-25 and 50-100.
                 * First intersect call produces nothing.
                 * Second intersect call produces (offset) range 50-100 and pushes 26-49.
                 *
                 * (next iteration of this loop)
                 * 0-49 gets processed.
                 * There exists remappings 0-25 and 50-100.
                 * First intersect call produces 0-25 and pushes 26-49.
                 * Second intersect call produces nothing.
                 *
                 * (next iteration of this loop)
                 * 26-49 gets processed
                 * Both intersect calls produce nothing.
                 * 26-49 is added to output without offset. (1:1 mapping of this segment).
                 *
                 * (next iteration of this loop)
                 * 26-49 gets processed (it was pushed twice).
                 * It has the same result as last time.
                 */
                for (int i = 0; i < output.size(); ++i) { // convert a current output layer
                    auto input_range = output[i]; // must copy, not reference. output may be pushed to, re-allocating the vector, moving the memory.
                    bool was_remapped = false;
                    for (auto& remap : remap_vec) {
                        auto& [offset, range] = remap;
                        RangeMonad result = input_range.intersect(range);
                        if (result.first) {
                            was_remapped = true;
                            // push the remaining part(s) of the input range.
                            if (result.second.start > input_range.start) { // left side of intersection
                                output.emplace_back(input_range.start, result.second.start - 1);
                            }
                            if (result.second.end < input_range.end) { // right side of intersection
                                output.emplace_back(result.second.end + 1, input_range.end);
                            }
                            // push the intersected part after offsetting it accordingly.
                            result.second.offset(offset);
                            new_output.push_back(result.second);
                        }
                    }
                    if (! was_remapped) {
                        new_output.push_back(input_range);
                    }
                }
                auto printVec = [](auto& v){ for(auto& x : v)std::cout<<x<<", "; };
                // prune duplicate ranges produced by the inner procedure.
                output.clear();
                std::for_each(new_output.begin(), new_output.end(), [&output](const Range& r) {
                    if (output.end() == std::find(output.begin(), output.end(), r)) {
                        output.push_back(r);
                    }
                });
            }
            // done all the remapping layers for one of the seed ranges. The ranges in 'output' represent what it broke down to.
            int64_t local_min = std::numeric_limits<int64_t>::max();
            std::for_each(output.begin(), output.end(), [&local_min](auto& outputRange) {
                if (outputRange.start < local_min) {
                    local_min = outputRange.start;
                }
            });
            if (local_min < global_min) {
                global_min = local_min;
            }
        }

        reportSolution(global_min);
#else
        parseInput(input);

        std::istringstream seed_reader(seed_string_numbers);

        std::vector<std::pair<int64_t, int64_t>> seed_groups;

        int64_t start_seed;
        int64_t seed_range;
        while (seed_reader >> start_seed >> seed_range) {
            seed_groups.emplace_back(start_seed, seed_range);
        }

        std::vector<int64_t> results;

        int64_t global_min = std::numeric_limits<int64_t>::max();
        for (int i = 0; i < seed_groups.size(); ++i) {
            auto& [seed, range] = seed_groups[i];
            std::cout << "Alloc " << range << " Items. (" << (range * sizeof(int64_t)) / static_cast<double>(1 << 30) << " GiB)\n";
            results.resize(range);

            std::cout << "Crunching " << range << " Items\n";
            auto start = std::chrono::steady_clock::now();

#pragma omp parallel for schedule(static) shared(std::cout, seed, range, results) default(none)
            for (int64_t s = seed; s < seed + range; ++s) {
                uint64_t remap_result = remapper.remap(s);
                results[s-seed] = remap_result;
            }

            auto duration = std::chrono::steady_clock::now() - start;
            double sec = (duration.count() / 1'000'000'000.0);
            std::cout << "The crunch took " << sec << " seconds: " << (range / sec) << " items per second\n";
            std::cout << "Take local min of these items:\n";
            int64_t local_min = std::numeric_limits<int64_t>::max();

#pragma omp parallel for simd schedule(static) default(none) reduction(min:local_min) shared(results)
            for (int j = 0; j < results.size(); ++j) {
                if (results[j] < local_min) {
                    local_min = results[j];
                }
            }
            std::cout << "\tLocal min is: " << local_min << "\n";
            results.clear(); // technically redundant.

            if (local_min < global_min) {
                global_min = local_min;
            }
        }

        reportSolution(global_min);
#endif
    }

private:
    NumberMapper remapper; // problem 1 and old problem 2 solution

    std::vector<std::vector<std::pair<int64_t, Range>>> mapping_ranges; // poroblem 2 solution.

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

    void parseAsProblem2(std::ifstream& text) {
        std::string line;
        std::getline(text, line);

        seed_string_numbers = line.substr(7); // skip past "seeds: "

        auto read_three_int64 = [](std::basic_istream<char>& stream){
            int64_t first = -1;
            int64_t second = -1;
            int64_t third = -1;

            stream >> first >> second >> third;

            assert((! stream.bad()) && first >= 0 && second >= 0 && third >= 0, "Input reading fail, expected 3 ints.");

            stream.get(); // consume newline character (or gets EOF, but that's fine).

            return std::make_tuple(first, second, third);
        };

        while (std::getline(text, line)) {
            if (line.empty()) { // new map introduced, let's make room for it. Assumes no double newlines or file end w/ newline.
                mapping_ranges.emplace_back();
                std::getline(text, line); // eat the line describing the name of the map.
                continue;
            }

            // only works because the first getline in the loop is a blank line, UB otherwise. :).
            auto& last_range_container = mapping_ranges.back();
            std::istringstream s(line);
            auto [dest, src, len] = read_three_int64(s);

            last_range_container.emplace_back(std::make_pair<int64_t, Range>(dest - src, {src, src + len - 1}));
        }
    }

    void reset() override {
        remapper = NumberMapper{};
        mapping_ranges.clear();
        Day::reset();
    }

};

#undef CONCATENATE
#undef CLASS_DEF
#undef CTOR_DEF
#undef DAY