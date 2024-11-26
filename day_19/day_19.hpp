#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 19

NAMESPACE_DEF(DAY) {

struct Object {
    int x;
    int m;
    int a;
    int s;

    Object() = delete;
    Object(int x, int m, int a, int s) : x(x), m(m), a(a), s(s) {}

    [[nodiscard]] int getValue() const { return x + m + a + s; }
};

struct ObjectWrapper {
    enum class State : uint8_t { TENTATIVE, ACCEPTED, REJECTED } state;
    const Object o;

    ObjectWrapper() = delete;
    explicit ObjectWrapper(Object o) : o(o), state(State::TENTATIVE) {}
};

using Functor = std::function<std::pair<bool,std::string>(ObjectWrapper&)>;

// problem 2
struct Rule {
    std::string remap;
    int guard;
    char field;
    char comparator;

    Rule() = delete;
    explicit Rule(const std::string& from) : guard(0) {
        if (from.find(':') == std::string::npos) { // unconditional function.
            field = 'x';
            comparator = '>';
            remap = from;
            guard = 0; // x > 0 is always true.
        } else {
            std::istringstream s(from);
            field = static_cast<char>(s.get());
            char op = static_cast<char>(s.get());
            switch (op) {
                default:
                    throw std::logic_error("Unknown op");
                case '>':
                case '<':
                    comparator = op;
            }

            s >> guard;
            if (s.get() != ':') throw std::logic_error("Stream cannot extract label");

            s >> remap;
        }
    }
};

struct Range {
    struct R {
        int from;
        int to;
        [[nodiscard]] int64_t size() const {
            return to - from + 1;
        }
        [[nodiscard]] bool empty() const {
            return from >= to;
        }
    };

    R x{};
    R m{};
    R a{};
    R s{};

    Range() : x(1,4000), m(1,4000), a(1,4000), s(1,4000) {}

    Range(const Range& copying, const R& but, char field) {
        x = (field == 'x' ? but : copying.x);
        m = (field == 'm' ? but : copying.m);
        a = (field == 'a' ? but : copying.a);
        s = (field == 's' ? but : copying.s);
    }

    [[nodiscard]] bool empty () const {
        return x.empty() || m.empty() || a.empty() || s.empty();
    }

    [[nodiscard]] uint64_t combinations () const {
        return x.size() * m.size() * a.size() * s.size();
    }

    // Given a rule, split this range into at most 2 other ranges. Returns how many range objects were created (0 to 2)
    [[nodiscard]] int split(const Rule& rule, std::array<Range, 2>& newRanges) const {
        const R * operatingOn;
        switch (rule.field) {
            default: throw std::logic_error("Illegal field");
            case 'x': operatingOn = &x; break;
            case 'm': operatingOn = &m; break;
            case 'a': operatingOn = &a; break;
            case 's': operatingOn = &s; break;
        }

        bool completelyOver = rule.comparator == '>' && rule.guard >= operatingOn->to;
        bool completelyUnder = rule.comparator == '<' && rule.guard <= operatingOn->from;
        if (completelyOver || completelyUnder) {
            return 0;
        }

        bool completelyCoveredA = rule.comparator == '>' && rule.guard < operatingOn->from;
        bool completelyCoveredB = rule.comparator == '<' && rule.guard > operatingOn->to;
        if (completelyCoveredA || completelyCoveredB) {
            newRanges[0] = *this;
            return 1;
        }

        // There is partial coverage, and the range must be split in two.
        R lhs{};
        R rhs{};
        switch (rule.comparator) {
            default: throw std::logic_error("Unknown comparator");
            case '>':
                lhs.from = operatingOn->from;
                lhs.to = rule.guard;
                rhs.from = rule.guard + 1;
                rhs.to = operatingOn->to;
                break;
            case '<':
                lhs.from = operatingOn->from;
                lhs.to = rule.guard - 1;
                rhs.from = rule.guard;
                rhs.to = operatingOn->to;
                break;
        }

        newRanges[0] = Range(*this, lhs, rule.field);
        newRanges[1] = Range(*this, rhs, rule.field);

        return 2;
    }
};

std::ostream& operator<<(std::ostream& os, const Range& r) {
    os << "Range ("
    <<"x: " << r.x.from << " - " << r.x.to
    << ", m: " << r.m.from << " - " << r.m.to
    << ", a: " << r.a.from << " - " << r.a.to
    << ", s: " << r.s.from<< " - " << r.s.to << ")";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) { // functors
            if (line.empty()) break;
            addFunctor(line);
            addRule(line);
        }

        while (std::getline(input, line)) { // objects
            addObject(line);
        }

        functions.emplace(ACCEPT_LABEL, accept);
        functions.emplace(REJECT_LABEL, reject);
    }

    void v1() const override {
        auto copy = objs;
        inspectObjects(copy);
        int64_t accepted_xmas_sum = 0;
        std::for_each(copy.begin(), copy.end(), [&accepted_xmas_sum](auto& ow) {
            if (ow.state == ObjectWrapper::State::ACCEPTED) {
                accepted_xmas_sum += ow.o.getValue();
            }
        });
        reportSolution(accepted_xmas_sum);
    }

    void v2() const override {
        reportSolution(recursiveCombinatorialCountWithRange(Range(), ENTRY_LABEL));
    }

    void parseBenchReset() override {
        objs.clear();
        functions.clear();
    }

private:
    // problem 1
    std::vector<ObjectWrapper> objs;
    std::map<std::string, Functor> functions;
    // problem 2
    std::map<std::string, std::vector<Rule>> rules;

    static constexpr auto ACCEPT_LABEL = "A";
    static constexpr auto REJECT_LABEL = "R";
    static constexpr auto ENTRY_LABEL = "in";

    Functor accept = [](auto& ow){ ow.state = ObjectWrapper::State::ACCEPTED; return std::make_pair(false, ACCEPT_LABEL); };
    Functor reject = [](auto& ow){ ow.state = ObjectWrapper::State::REJECTED; return std::make_pair(false, REJECT_LABEL); };

    // Runs through the remapping functions, by taking the result label of a remap, and applying it again to that function in the map,
    // Until 'false' is returned from the remap, which is done exclusively by 'accept' and 'reject' functions,
    // which mutate the ObjectWrapper state to either 'accepted' or 'rejected'.
    void inspectObjects(std::vector<ObjectWrapper>& objects) const {
        for (auto& obj : objects) {
            inspectObject(obj);
        }
    }

    inline void inspectObject(ObjectWrapper& obj) const {
        auto * f = &getFunc(ENTRY_LABEL);

        while (true) {
            auto [remapped, to] = f->operator()(obj);

            if (! remapped) break;
            f = &getFunc(to);
        }
    }

    [[nodiscard]] const Functor& getFunc(const std::string& s) const {
        auto iter = functions.find(s);
        if (iter == functions.end()) throw std::logic_error("Unknown function label: " + s);

        return iter->second;
    }

    uint64_t recursiveCombinatorialCountWithRange(const Range& r, const std::string& lbl) const {
        // std::cout << "Eval " << r << " w/ " << lbl << "\n";

        // base cases.
        if (lbl == "A") {
            auto comb = r.combinations();
            // std::cout << "\tAccept w/ " << comb << "\n";
            return comb;
        }
        if (lbl == "R") {
            // std::cout << "\tReject (0)";
            return 0;
        }

        auto iter = rules.find(lbl);
        if (iter == rules.end()) throw std::logic_error("lbl " + lbl + " Does not exist");

        auto workingWith = r;
        int ruleIndex = 0;
        uint64_t combinationCount = 0;
        while (! workingWith.empty()) { // dangerous and unsafe. ruleIndex will increment past the vector if it weren't the case that the last rule in any vector is always encompassing the entire range.
            auto& rule = iter->second[ruleIndex];
            std::array<Range, 2> split;
            int newRanges = workingWith.split(rule, split);
            switch (newRanges) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
                default: throw std::logic_error("Unknown number of split ranges");
#pragma clang diagnostic pop
                case 0: // nothing satisfies the rule, try the next rule in this list.
                    ruleIndex ++;
                    break;
                case 1: // the whole range fits the rule. We can stop looking through this list.
                    // std::cout << "\trecurse entire range to " << rule.remap << "\n";
                    return combinationCount + recursiveCombinatorialCountWithRange(workingWith, rule.remap);
                case 2:
                    // Part goes to the next rule, part goes to a different list.
                    if (rule.comparator == '>') { // [0] does not satisfy the rule. [1] does.
                        combinationCount += recursiveCombinatorialCountWithRange(split[1], rule.remap);
                        workingWith = split[0];
                    } else { // '<'. [0] satisfies the rule, [1] does not.
                        combinationCount += recursiveCombinatorialCountWithRange(split[0], rule.remap);
                        workingWith = split[1];
                    }
                    ruleIndex++;
                    break;
            }
        }

        return combinationCount;
    }

    void addFunctor(const std::string& from) {
        std::istringstream s(from);
        std::ostringstream buf;

        int c;
        while ((c = s.get()) != '{') {
            buf << static_cast<char>(c);
        }
        std::string label = buf.str();

        buf.clear();
        buf.str(std::string());

        Functor f;
        bool canCompose = false;
        while ((c = s.get()) != EOF) {
            if (c == ',' || c == '}') {
                std::string function = buf.str();
                buf.clear();
                buf.str(std::string());

                f = parseFunction(function, canCompose, f);
                canCompose = true;
            } else {
                buf << static_cast<char>(c);
            }
        }

        functions.emplace(label, std::move(f));
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedLocalVariable" // stupid IDE thinks the value-captured variables are unused.
#pragma ide diagnostic ignored "UnusedValue"
    static Functor parseFunction(const std::string& from, bool compose, Functor& composeWith) {
        auto conditionPosition = from.find(':');

        Functor newF;
        if (conditionPosition == std::string::npos) { // unconditional function
            newF = [from](ObjectWrapper& ow){ return std::make_pair(true, from); };
        } else {
            std::string target = from.substr(conditionPosition + 1);
            std::string condition = from.substr(0, conditionPosition);

            auto ltPos = condition.find('<');
            auto gtPos = condition.find('>');

            char operand = condition[0];
            std::function<bool(int, int)> comparator;
            int guard;
            if (ltPos != std::string::npos) {
                guard = std::stoi(condition.substr(ltPos + 1));
                comparator = std::less<>();
            } else if (gtPos != std::string::npos) {
                guard = std::stoi(condition.substr(gtPos + 1));
                comparator = std::greater<>();
            } else {
                throw std::logic_error("Unknown condition " + condition);
            }

            newF = [operand, guard, target, comparator](ObjectWrapper& ow) {
                int val;
                switch (operand) {
                    default: throw std::logic_error("Unknown operand " + std::string(1, operand));
                    case 'x': val = ow.o.x; break;
                    case 'm': val = ow.o.m; break;
                    case 'a': val = ow.o.a; break;
                    case 's': val = ow.o.s; break;
                }

                bool guardPass = comparator(val, guard);
                return std::make_pair(guardPass, target);
            };
        }

        if (compose) {
            return [newF, composeWith](ObjectWrapper& ow){
                // first call the parent function.
                auto result = composeWith(ow);
                if (result.first) return result; // if the parent function did a remap, we do not consider children.

                return newF(ow); // no remap? call the newly added function.
            };
        } else {
            return newF;
        }
    }
#pragma clang diagnostic pop

    void addRule(const std::string& from) {
        std::istringstream s(from);
        std::ostringstream o;
        auto resetO = [&o](){ o.clear(); o.str(std::string()); };
        int c;
        while ((c = s.get()) != '{') {
            o << static_cast<char>(c);
        }
        std::string label = o.str();
        resetO();

        while ((c = s.get()) != EOF) {
            if (c == ',' || c == '}') {
                rules[label].emplace_back(o.str());
                resetO();
            } else {
                o << static_cast<char>(c);
            }
        }
    }

    void addObject(const std::string& from) {
        auto skip3 = [](std::istringstream& s) { s.get(); s.get(); s.get(); };
        std::istringstream scan(from);

        int x, m, a, s;

        skip3(scan); scan >> x;
        skip3(scan); scan >> m;
        skip3(scan); scan >> a;
        skip3(scan); scan >> s;

        objs.emplace_back(std::forward<Object>(Object(x,m,a,s)));
    }
};

} // namespace

#undef DAY