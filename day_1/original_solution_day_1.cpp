#include <iostream>
#include <fstream>
#include <array>
#include <stdexcept>
#include <memory>
#include <functional>
#include <chrono>
#include <utility>

int v1(std::ifstream& text);

int v2(std::ifstream& text);
int get_char_of_line_fwd(const std::string& line);
int get_char_of_line_bwd(const std::string& line);

// SFINAE spaghetti to restrict the template type of check_string_iterator_for_sequence.
template <typename I>
struct is_string_iterator : std::false_type {};

template <>
struct is_string_iterator<std::string::const_iterator> : std::true_type {};

template <>
struct is_string_iterator<std::string::const_reverse_iterator> : std::true_type {};

template<size_t N, typename StringIterator>
std::enable_if_t<is_string_iterator<StringIterator>::value, std::pair<bool, int>>
check_string_iterator_for_sequence(
    StringIterator,
    const StringIterator&,
    const std::array<std::string, N>&
);

int v2_single_pass(std::ifstream& text);

int main()
{

    const int BENCH_ITER = 100'000;
    const int BENCH_SLOW_ITER = 10'000;

    {
        std::ifstream text("day_1/day1.txt");

        auto start = std::chrono::steady_clock::now();
        for (int x = 0; x < BENCH_ITER; ++x) {
            v1(text);
            text.clear();
            text.seekg(0);
        }
        auto end = std::chrono::steady_clock::now();

        auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "(" << BENCH_ITER << " iterations) v1 ns " << dur << ", avg = " << (static_cast<double>(dur) / BENCH_ITER) / 1'000'000.0 << "ms \n";
    }
    {
        std::ifstream text("day_1/day1.txt");
        auto start = std::chrono::steady_clock::now();
        for (int x = 0; x < BENCH_ITER; ++x) {
            v2(text);
            text.clear();
            text.seekg(0);
        }
        auto end = std::chrono::steady_clock::now();

        auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout <<  "(" << BENCH_ITER << " iterations) v2 ns " << dur << " avg = " << (static_cast<double>(dur) / BENCH_ITER) / 1'000'000.0 << "ms \n";
    }
    {
        std::ifstream text("day_1/day1.txt");
        auto start = std::chrono::steady_clock::now();
        for (int x = 0; x < BENCH_SLOW_ITER; ++x) {
            v2_single_pass(text);
            text.clear();
            text.seekg(0);
        }
        auto end = std::chrono::steady_clock::now();

        auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout <<  "(" << BENCH_SLOW_ITER << " iterations) v2_sp ns " << dur << " avg = " << (static_cast<double>(dur) / BENCH_SLOW_ITER) / 1'000'000.0 << "ms \n";
    }

    return 0;
}

struct AutomatonConfig {
    const char * language;
    int input_accepted_value;
};

class AutomatonInterface {
protected:
    explicit AutomatonInterface(std::function<void(int)> cb) : input_accept_callback(std::move(cb)) {}
    AutomatonInterface() : AutomatonInterface([](int){}) {}

public:
    virtual void feed(char c) = 0;
    virtual ~AutomatonInterface() = default;

    std::function<void(int)> input_accept_callback;
};

class Automaton : public AutomatonInterface {

    const char * language;
    const int input_accept_value;

    int language_index = 0;

public:
    Automaton(AutomatonConfig config, std::function<void(int)> callback)
    : AutomatonInterface(std::move(callback))
    , language(config.language)
    , input_accept_value(config.input_accepted_value)
    { }

    explicit Automaton(AutomatonConfig&& config) : Automaton(config, [](int){}) {}

    void feed(char c) override {
        bool matching = c == language[language_index];

        if (! matching) {
            language_index = 0;
            // Don't give up yet, we fall back to index 0, but maybe index 0 is a match?
            // If you do not do this, things like "eeight" for "eight" will be missed.
            language_index += (c == language[0]);
        } else {
            language_index += 1;
            if ('\0' == language[language_index]) {
                language_index = 0;
                input_accept_callback(input_accept_value);
            }
        }
    }
};

class MultiAutomaton : public AutomatonInterface {
    std::vector<std::unique_ptr<AutomatonInterface>> automata;

public:
    explicit MultiAutomaton(std::vector<AutomatonConfig> && blueprints) {
        for (auto&& b : blueprints) {

            auto a = std::make_unique<Automaton>(std::forward<AutomatonConfig>(b), [this](int x) {
                this->input_accept_callback(x);
            });
            automata.emplace_back(std::move(a));
        }
    }

    MultiAutomaton(MultiAutomaton && other) noexcept
    : AutomatonInterface(other.input_accept_callback)
    , automata(std::move(other.automata))
    {
        // Bad news, the bound [this] on the assigned callbacks is undefined behavior to use after the move.
        // And I don't think I can just tell the function closure to "update the fucking pointer to something else",
        // So... I'm just throwing away whatever callbacks were in there, and making new ones.
        // This kills composition of nested MultiAutomatons.
        for (auto& a : automata) {
            a->input_accept_callback = [this](int x) {
                this->input_accept_callback(x);
            };
        }
    }

    MultiAutomaton() = default;

    void add(std::unique_ptr<AutomatonInterface> other) {

        other->input_accept_callback = [original_cb = other->input_accept_callback, this](int x){
            original_cb(x);
            this->input_accept_callback(x);
        };
        automata.emplace_back(std::move(other));
    }

    void feed(char c) override {
        for (auto& a : automata) {
            a->feed(c);
        }
    }
};

int v2_single_pass(std::ifstream& text) {
    const int NEWLINE_VALUE = -1;
    MultiAutomaton digits({
        {"1", 1}, {"2", 2}, {"3", 3},
        {"4", 4}, {"5", 5}, {"6", 6},
        {"7", 7}, {"8", 8}, {"9", 9}
    });
    MultiAutomaton sequences({
        {"one", 1},     {"two", 2},     {"three", 3},
        {"four", 4},    {"five", 5},    {"six", 6},
        {"seven", 7},   {"eight", 8},   {"nine", 9}
    });
    Automaton newline({"\n", NEWLINE_VALUE});

    MultiAutomaton nfa;
    nfa.add(std::move(std::make_unique<MultiAutomaton>(std::move(digits))));
    nfa.add(std::move(std::make_unique<MultiAutomaton>(std::move(sequences))));
    nfa.add(std::move(std::make_unique<Automaton>(std::move(newline))));

    int line_first = 0;
    int line_last = 0;
    int sum = 0;
    nfa.input_accept_callback = [&](int x) {
        if (x == NEWLINE_VALUE) {
            sum += line_first * 10 + line_last;
            line_first = 0;
            line_last = 0;
        } else {
            line_last = x;
            line_first = line_first == 0 ? x : line_first;
        }
    };

    int c;
    while ((c = text.get()) != EOF) {
        nfa.feed(static_cast<char>(c));
    }
    nfa.feed('\n'); // let's emulate EOF as a newline char, this flushes the last line to sum.

    return sum;
}

int v2(std::ifstream& text) {

    std::string line;
    int sum = 0;

    while (getline(text, line)) {
        // get the first digit or word, search from the left
        int first_value = get_char_of_line_fwd(line);

        // get the last digit or word, search from the right
        int last_value = get_char_of_line_bwd(line);

        int line_value = 10 * first_value + last_value;
        sum += line_value;
    }

    return sum;
}

int get_char_of_line_fwd(const std::string& line) {

#define S std::string
    static std::array numbers{S("one"), S("two"), S("three"), S("four"), S("five"), S("six"), S("seven"), S("eight"), S("nine")};
#undef S

    for (std::string::const_iterator it = line.begin(); it != line.end(); ++it) {
        char c = *it;
        switch (c) {
            default:
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
                return c - '0';

    // "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
    // start_chars_fwd = "otfsen";
    // start_chars_bwd = "eorxnt";
            case 'o':
            case 't':
            case 'f':
            case 's':
            case 'e':
            case 'n':
                auto [found, index] = check_string_iterator_for_sequence(it, line.end(), numbers);
                if (found) {
                    return index + 1; // the relation of sequence index to number is N+1.
                }
        }
    }

    std::cout << "!!  " << line << "  !!\n";
    throw std::invalid_argument( "line without detectable number." );
}

int get_char_of_line_bwd(const std::string& line) {
#define S(x) ([](const char * s) { auto str = std::string(s); std::reverse(str.begin(), str.end()); return str; })(x)
    static std::array numbers_backward{S("one"), S("two"), S("three"), S("four"), S("five"), S("six"), S("seven"), S("eight"), S("nine")};
#undef S

    for (std::string::const_reverse_iterator it = line.rbegin(); it != line.rend(); ++it) {
        char c = *it;
        switch (c) {
            default:
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
                return c - '0';

    // "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
    // start_chars_fwd = "otfsen";
    // start_chars_bwd = "eorxnt";
            case 'e':
            case 'o':
            case 'r':
            case 'x':
            case 'n':
            case 't':
                auto [found, index] = check_string_iterator_for_sequence(it, line.rend(), numbers_backward);
                if (found) {
                    return index + 1; // the relation of sequence index to number is N+1.
                }
        }
    }

    std::cout << "!!  " << line << "  !!\n";
    throw std::invalid_argument( "line without detectable number." );
}


/**
 * @returns If there is no match: false in the first pair, with an undefined second value.
 *          If there is a match: true in the first of the pair,
 *                               with the index in the sequence array in the second value of the pair.
 */
template<size_t N, typename StringIterator>
std::enable_if_t<is_string_iterator<StringIterator>::value, std::pair<bool, int>>
check_string_iterator_for_sequence(
    StringIterator it, // the iterator is intentionally copied by value, it will be modified in the function.
    const StringIterator& end,
    const std::array<std::string, N>& sequences
) {
    if (N > 63) {
        throw std::invalid_argument( "Nope." );
    }

    // start with assuming every index matches...
    uint64_t match_set = (1 << N) - 1;

    int match_index = -1;
    while (match_set != 0 && it != end) {
        char c;
        {
            c = *it;
            ++it;
            ++match_index;
        }

        for (int x = 0; x < N; ++x) {
            if ((match_set >> x) & 1) {
                const std::string& may_match = sequences[x];

                // A sequence remains matching if:
                // may_match[match_index] == c;
                char compare = may_match[match_index];
                if (compare == c) {
                    // The sequence is a complete match if:
                    // this was the last char in may_match.
                    if (match_index + 1 >= may_match.size()) {
                        return std::make_pair(true, x);
                    }
                } else {
                    // Drop the sequence from the match_set.
                    match_set &= ~(1 << x);
                }
            }
        }
    }

    return std::make_pair(false, -1);
}

int v1(std::ifstream& text) {
    int sum = 0;
    int first_in_line = 0;
    int last_in_line = 0;
    int read_value;

    int c = 0;
    while (c != EOF) {
        c = text.get(); // EOF should still go through the case labels to sum the final line.
        switch(c) {
            default:
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
                read_value = c - '0';
                last_in_line = read_value;
                first_in_line = first_in_line + (first_in_line == 0) * read_value;
                break;

            case EOF:
            case '\n':
                sum += (10 * first_in_line) + last_in_line;
                first_in_line = 0;
                last_in_line = 0;
                break;
        }
    }

    return sum;
}