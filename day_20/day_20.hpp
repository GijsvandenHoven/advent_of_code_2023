#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 20

NAMESPACE_DEF(DAY) {

enum class Signal : uint8_t {
    LOW = 0,
    HIGH = 1,
    NONE = 0xFF
};

enum class ModuleType : uint8_t { // used by factory for constructing the right polymorphic type from a blueprint.
    OUTPUT,
    TRANSMIT,
    FLIP,
    CONJUNCT
};

struct ModuleBlueprint {
    std::string name;
    std::vector<std::string> outputConnections;
    ModuleType t;

    ModuleBlueprint() = delete;
    ModuleBlueprint(std::string s, std::vector<std::string>&& o, ModuleType _t) : name(std::move(s)), outputConnections(o), t(_t) {}
};

struct Module {
    std::string name;
    std::vector<Module *> outputConnections;

    // Returns whether a signal was output, and if so, what signal.
    virtual Signal incomingSignal(const Module * from, Signal s) = 0;

    // Notify the Module it may receive input from (param). Normally unused, derived classes may make use of it.
    virtual void connectedTo(const Module * m) {}

    // Notify the Module its output is connected to (param)
    virtual void addConnection(Module * connectingTo) {
        outputConnections.push_back(connectingTo);
    }

    Module() = delete;
    explicit Module(std::string n) : name(std::move(n)) {}
    virtual ~Module() = default;
};

struct TransmitModule : public Module {
    using Module::Module;

    Signal incomingSignal(const Module * from, Signal s) override {
        return s; // it just forwards the signal.
    }
};

struct OutputModule : public Module {
    using Module::Module;

    Signal state = Signal::NONE;

    void addConnection(Module * connectingTo) override {
        throw std::logic_error("OutputModule should not be connected to anything.");
    }

    Signal incomingSignal(const Module * from, Signal s) override {
        state = s;
        return Signal::NONE; // nothing should happen. it's just an output module.
    }
};

struct FlipModule : public Module {
    bool on = false;

    using Module::Module;

    Signal incomingSignal(const Module * from, Signal s) override {
        if (s == Signal::LOW) {
            on = ! on;
            return (on ? Signal::HIGH : Signal::LOW);
        } // high signals are ignored.

        return Signal::NONE;
    }
};

struct ConjunctModule : public Module {
    std::map<const Module *, Signal> inputConnections;

    using Module::Module;

    void connectedTo(const Module * m) override {
        inputConnections.emplace(m, Signal::LOW);
    }

    Signal incomingSignal(const Module * from, Signal s) override {
        // only if every remembered pulse is high, sends low. Otherwise, high.
        // First updates the memory of the received signal before doing this check.
        auto iter = inputConnections.find(from);
        // not taking any chances with inserting through operator[].
        if (iter == inputConnections.end()) {
            throw std::logic_error("Conjunct Module received signal from a recipient not known in memory; It was not added with 'addConnection'? - " + from->name);
        }

        iter->second = s; // update the memory.

        // check the memory, if all high sends low. Otherwise, high.
        for (auto& kvp : inputConnections) {
            if (kvp.second == Signal::LOW) {
                return Signal::HIGH;
            }
        }

        return Signal::LOW;
    }
};

std::ostream& operator<<(std::ostream& os, const Module& m) {
    os << "Module {\n";
    os << "\t" << m.name << "\n\tConnections {\n";
    for (auto ptr : m.outputConnections) {
        os << "\t\t" << ptr << "\n";
    }
    os << "\t}\n}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Signal& s) {
    switch(s) { default: os << "?"; break; case Signal::LOW: os << "LO"; break; case Signal::HIGH: os<< "HI"; break; case Signal::NONE: os << "NONE"; break; }
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            std::istringstream s(line);

            char mode = static_cast<char>(s.get());
            std::string name;
            s >> name;

            ModuleType t;
            switch (mode) {
                case '%':
                    t = ModuleType::FLIP;
                    break;
                case '&':
                    t = ModuleType::CONJUNCT;
                    break;
                default: // e.g. 'broadcaster' has no prefix. Such modules are assumed to be transmitters.
                    t = ModuleType::TRANSMIT;
                    name.insert(0, 1, mode);
                    break;
            }

            s.ignore(4); // ' -> '

            std::ostringstream buf;
            std::vector<std::string> thisModuleConnections;
            while (! s.eof()) {
                int c = s.get();
                switch (c) {
                    case EOF:
                    case ',':
                        thisModuleConnections.emplace_back(std::move(buf.str()));
                        buf.clear();
                        buf.str(std::string());
                        s.ignore(1); // skip over the space that comes after the comma. Does nothing in the EOF case.
                        break;
                    default:
                        buf << static_cast<char>(c);
                        break;
                }
            }

            moduleBlueprint.emplace_back(name, std::move(thisModuleConnections), t);
        }

        // at this point, the parsed Prints are mostly all made. Those not referenced on the LHS such as output modules are not yet in.
        // We can only find them by evaluating all output connections to see if they are missing.
        std::map<std::string, ModuleBlueprint> stragglers;
        for (auto& print : moduleBlueprint) {
            for (auto& name : print.outputConnections) {
                auto iter = std::find_if(moduleBlueprint.begin(), moduleBlueprint.end(), [&name](auto& p) {
                    return p.name == name;
                });

                if (iter == moduleBlueprint.end()) { // this referenced label does not exist. Straggler!
                    // do not mutate moduleBlueprint here. That invalidates iterators to the range-based for loop.
                    // Using the map here also prevents duplicates from being added.
                    stragglers.emplace(name, ModuleBlueprint(name, std::vector<std::string>(), ModuleType::OUTPUT));
                }
            }
        }

        std::for_each(stragglers.begin(), stragglers.end(), [this](auto& pair){
            moduleBlueprint.emplace_back(std::move(pair.second));
        });
    }

    void v1() const override {
        // make a mutable copy of the input data.
        std::vector<std::unique_ptr<Module>> circuit;
        createFromBlueprint(circuit);

        auto [lo, hi] = countLowAndHighPulses(1000, findByName(circuit, BROADCASTER_NAME));
        reportSolution(lo * hi);
    }

    // Very sneaky! Today's problem does not have a generalized "clever" solution!
    // No memoization, no general formula for time-to-get-low on a ConjunctModule!
    // It only works because the penultimate output is a Conjunct for a few binary counters.
    // These counters emit a HI every few cycles, and then immediately go LO again.
    // Thus, the answer is the LCM of these counters. And it only works due to the shape of the output.
    // Otherwise, the problem is allegedly NP-hard (The circuits could form Circuit-SAT).
    void v2() const override {

        // find the module that connects to the output node.
        auto outputIter = moduleBlueprint.end();
        for (auto iter = moduleBlueprint.begin(); iter != moduleBlueprint.end(); ++iter) {
            auto& c = iter->outputConnections;
            if (c.end() != std::find_if(c.begin(), c.end(), [](auto& label){ return label == P2_OUTPUT_NAME; })) {
                if (outputIter != moduleBlueprint.end()) { // this case does not exist in the puzzle input.
                    throw std::logic_error("Expected exactly one module connected to " + std::string(P2_OUTPUT_NAME) + ", found multiple.");
                } else {
                    outputIter = iter;
                }
            }
        }

        if (outputIter == moduleBlueprint.end()) throw std::logic_error("Could not find module connected to '" + std::string(P2_OUTPUT_NAME) + "'.");
        if (outputIter->t != ModuleType::CONJUNCT) throw std::logic_error("Expected the connected-to-output Module to be a ConjunctModule");

        // find the modules that connect to the output-connecting-node.
        std::vector<std::string> connectedToConjunct;
        for (auto& p : moduleBlueprint) {
            auto iter = std::find_if(p.outputConnections.begin(), p.outputConnections.end(), [&n = outputIter->name](auto& name){
                return name == n;
            });

            if (iter != p.outputConnections.end()) {
                connectedToConjunct.emplace_back(p.name);
            }
        }

        // The stop condition is defined as the first time one of the connected-to-conjuncts outputs HIGH.
        // Since it is ASSUMED THAT THESE ARE BINARY COUNTERS, I.E. PULSE HIGH AND INSTANTLY GO LOW AGAIN (!!)
        // The #cycles for the output to go low, is equal to that for all conjuncts to be high,
        // Which is equal to the LCM of the numbers found by this stop condition.
        auto stopCondition = [](Module * m){
            return [m](Module * to, Module * from, Signal s) -> bool {
                return from == m && s == Signal::HIGH;
            };
        };

        uint64_t lcm = 1;
        std::vector<std::unique_ptr<Module>> circuit;
        for (const auto& name : connectedToConjunct) {
            createFromBlueprint(circuit); // resets the vector, this is necessary each time to start from cycle 0 as intended.
            auto start = findByName(circuit, BROADCASTER_NAME);
            auto target = findByName(circuit, name); // Re-do this each time. They would be invalid after the mutable copy is re-cloned!

            uint64_t countUntilCycle = countCyclesUntilCondition(start, stopCondition(target));
            lcm = std::lcm(lcm, countUntilCycle);
        }

        reportSolution(lcm);
    }

    void parseBenchReset() override {
        moduleBlueprint.clear();
    }

private:
    // immutable blueprint of the parsed input.
    // Solvers should make copies of this object; This is to allow repeatability during benchmarking,
    // And disallow interference of subsequent solvers.
    std::vector<ModuleBlueprint> moduleBlueprint;
    static constexpr auto BROADCASTER_NAME = "broadcaster";
    static constexpr auto P2_OUTPUT_NAME = "rx";

    // very hairy code due to the use of pointers in member variables of Module.
    void createFromBlueprint(std::vector<std::unique_ptr<Module>>& modules) const {
        modules.clear();

        std::map<std::string, Module *> nameToPointer;
        for (auto& print : moduleBlueprint) {
            std::unique_ptr<Module> p;
            switch(print.t) {
                case ModuleType::OUTPUT:
                    p = std::make_unique<OutputModule>(print.name);
                    break;
                case ModuleType::TRANSMIT:
                    p = std::make_unique<TransmitModule>(print.name);
                    break;
                case ModuleType::FLIP:
                    p = std::make_unique<FlipModule>(print.name);
                    break;
                case ModuleType::CONJUNCT:
                    p = std::make_unique<ConjunctModule>(print.name);
                    break;
                default: throw std::logic_error("Unknown blueprint type");
            }

            modules.push_back(std::move(p));
            nameToPointer[print.name] = modules.back().get();
        }

        // the objects were created, but the connections have not been established yet.
        for (auto& print : moduleBlueprint) {
            auto * src = nameToPointer[print.name];
            for (auto& con : print.outputConnections) {
                auto * trg = nameToPointer[con];
                src->addConnection(trg);
                trg->connectedTo(src);
            }
        }
    }

    static Module * findByName(std::vector<std::unique_ptr<Module>>& modules, const std::string& name) {
        auto iter = std::find_if(modules.begin(), modules.end(), [&name](auto& ptr) {
            return ptr->name == name;
        });
        if (iter == modules.end()) throw std::logic_error("Could not find module: " + name);

        return iter->get();
    }

    // No null checks are made, Assumes the input Module pointer, and all transitively pointed-to modules to exist.
    // WARNING: No memoization is done, it just runs the cycle amount you give it.
    // A mutable pointer is necessary, because previous cycles can affect the current cycle.
    // e.g. a ConjunctModule's memory or FlipModule's state can be different per cycle.
    // returns the low and high signal count respectively.
    [[nodiscard]] static std::pair<int,int> countLowAndHighPulses(int cycles, Module * startingPoint) {
        int lo_count = 0;
        int hi_count = 0;

        // callback passed to emulateSignalEnteringModule, only the signal is important to us.
        auto registerPulse = [&lo_count, &hi_count](Module*,Module*,Signal s){
            switch (s) {
                default: break; // do nothing, e.g. for 'NONE' signal.
                case Signal::LOW: lo_count++; break;
                case Signal::HIGH: hi_count++; break;
            }
        };

        for (int i = 0; i < cycles; ++i) {
            emulateSignalEnteringModule(startingPoint, Signal::LOW, registerPulse);
        }

        return std::make_pair(lo_count, hi_count);
    }

    [[nodiscard]] static uint64_t countCyclesUntilCondition(
            Module * startingPoint,
            const std::function<bool(Module*, Module*, Signal)>& stopCondition,
            uint64_t errorAfterThisManyCycles = 1'000'000
    ) {
        bool work = true;

        auto callback = [&work, &stopCondition](Module* t,Module* f,Signal s) -> void {
            bool shouldStop = stopCondition(t,f,s);
            if (work && shouldStop) {
                work = false;
            }
        };

        uint64_t i = 0;
        while (work) {
            if (i > errorAfterThisManyCycles) throw std::logic_error("Could not satisfy the stop condition after " + std::to_string(errorAfterThisManyCycles) + " Cycles.");
            i++;
            emulateSignalEnteringModule(startingPoint, Signal::LOW, callback);
        }
        return i;
    }

    /**
     * Emulates the circuit (mutating the state of all affected Modules) given an incoming signal S to a module enterPoint.
     * @param enterPoint the Module that shall receive the input signal. The 'from' parameter of 'incomingSignal' shall be nullptr.
     * @param s the Signal that the module shall receive.
     * @param callback  this function is called for every module that is about to receive a signal. It has 3 params:
     *                  'to' (who is receiving),
     *                  'from': (who is sending)
     *                  's': (What signal is being sent)
     */
    static void emulateSignalEnteringModule(
            Module * enterPoint,
            Signal s,
            const std::function<void(Module* to, Module* from, Signal s)>& callback = [](auto,auto,auto){}
    ) {
        std::queue<std::tuple<Module *, Module *, Signal>> queue; // to, from, signal.
        queue.emplace(enterPoint, nullptr, s); // start by emplacing a low signal from 'nothing' to the starting point.

        while (! queue.empty()) {
            auto [to, from, signal] = queue.front();
            queue.pop();
            callback(to, from, signal);

            Signal output = to->incomingSignal(from, signal);
            if (output == Signal::NONE) continue; // The signal is eaten, connected modules receive nothing.

            for (auto * p : to->outputConnections) { // enqueue processing of connected modules.
                queue.emplace(p, to, output);
            }
        }
    }

    [[noreturn]] [[maybe_unused]] static void generateTableOfState(Module * startingPoint, Module * whom) {
        Signal lastKnownState = Signal::NONE;
        int i = 0;

        auto callback = [whom, &lastKnownState, &i](Module * t, Module * f, Signal s){
            if (f == whom && s != lastKnownState) {
                lastKnownState = s;
                std::cout << i << "\t: " << s << "\n";
            }
        };

        while (true) {
            i++;
            emulateSignalEnteringModule(startingPoint, Signal::LOW, callback);
        }
    }
};

} // namespace

#undef DAY