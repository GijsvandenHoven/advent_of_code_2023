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

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) { // functors
            if (line.empty()) break;
            addFunctor(line);
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

    // todo: This is probably some substitution thing to get a spread on it all for starters.
    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        objs.clear();
        functions.clear();
    }

private:
    std::vector<ObjectWrapper> objs;
    std::map<std::string, Functor> functions;

    static constexpr std::string ACCEPT_LABEL = "A";
    static constexpr std::string REJECT_LABEL = "R";
    static constexpr std::string ENTRY_LABEL = "in";

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