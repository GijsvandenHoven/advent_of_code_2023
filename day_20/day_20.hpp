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

std::ostream& operator<<(std::ostream& os, const Signal& s) {
    switch(s) { default: os << "?"; break; case Signal::LOW: os << "LO"; break; case Signal::HIGH: os<< "HI"; break; case Signal::NONE: os << "NONE"; break; }
    return os;
}

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
    [[nodiscard]] virtual std::unique_ptr<Module> clone() const = 0; // beware, also clones e.g. the outputConnections vector, referencing non-cloned pointers.
};

struct TransmitModule : public Module {
    using Module::Module;

    Signal incomingSignal(const Module * from, Signal s) override {
        return s; // it just forwards the signal.
    }

    [[nodiscard]] std::unique_ptr<Module> clone() const override {
        return std::make_unique<TransmitModule>(*this);
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

    [[nodiscard]] std::unique_ptr<Module> clone() const override {
        return std::make_unique<OutputModule>(*this);
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

    [[nodiscard]] std::unique_ptr<Module> clone() const override {
        return std::make_unique<FlipModule>(*this);
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
            throw std::logic_error("Conjunct Module received signal from a recipient not known in memory; It was not added with 'addConnection'?");
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

    [[nodiscard]] std::unique_ptr<Module> clone() const override {
        return std::make_unique<ConjunctModule>(*this);
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

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        std::vector<std::pair<std::unique_ptr<Module>, std::vector<std::string>>> connections;
        while (std::getline(input, line)) {
            std::istringstream s(line);

            char mode = static_cast<char>(s.get());
            std::string name;
            s >> name;

            std::unique_ptr<Module> m;
            switch (mode) {
                case '%':
                    m = std::make_unique<FlipModule>(name);
                    break;
                case '&':
                    m = std::make_unique<ConjunctModule>(name);
                    break;
                default: // e.g. 'broadcaster' has no prefix. Such modules are assumed to be transmitters.
                    m = std::make_unique<TransmitModule>(mode + name);
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

            connections.emplace_back(std::move(m), std::move(thisModuleConnections));
        }

        // after this point, the connections vec is filled with modules and who they should connect to.
        // And all labels referenced should exist now.
        std::vector<std::unique_ptr<Module>> stragglers; // Only referenced in output, do not transmit anything. The second example does this with 'output'.
        for (auto& [ptr, cons] : connections) {
            for (auto& lbl : cons) {
                // std::cout << "For " << ptr->name << " Try to connect '" << lbl << "'\n";
                auto iter = std::find_if(connections.begin(), connections.end(), [&lbl](auto& item) {
                    return item.first->name == lbl;
                });

                if (iter == connections.end()) {
                    // std::cout << "WARNING: " << "Connection references unknown module: " << lbl << "\n";
                    // std::cout << "It may be created as an output module, as it was not parsed during the first pass.\n";

                    // If multiple nodes reference the same output module, it might already exist? This is not present in the puzzle, however.
                    auto seek = std::find_if(stragglers.begin(), stragglers.end(), [&lbl](auto& s){
                        return s->name == lbl;
                    });

                    if (seek == stragglers.end()) {
                        stragglers.emplace_back(std::make_unique<OutputModule>(lbl));
                        ptr->addConnection(stragglers.back().get());
                        stragglers.back()->connectedTo(ptr.get());
                    } else {
                        ptr->addConnection(seek->get());
                        seek->get()->connectedTo(ptr.get());
                    }
                } else {
                    ptr->addConnection(iter->first.get());
                    iter->first->connectedTo(ptr.get());
                }
            }
        }

        for (auto & connection : connections) { // Module creating complete, move the pointer ownership to the member vector.
            moduleBlueprint.emplace_back(std::move(connection.first));
        }
        for (auto & straggler : stragglers) {
            moduleBlueprint.emplace_back(std::move(straggler));
        }
    }

    void v1() const override {
        // make a mutable copy of the input data.
        std::vector<std::unique_ptr<Module>> mutableCopy;
        createMutableCopy(mutableCopy);

        auto [lo, hi] = countLowAndHighPulses(1000, findByName(mutableCopy, BROADCASTER_NAME));
        reportSolution(lo * hi);
    }

    void v2() const override {
        std::vector<std::unique_ptr<Module>> mutableCopy;
        createMutableCopy(mutableCopy);

        Module * target = findByName(mutableCopy, "output");

        auto stopCondition = [target](Module * t, Module * f, Signal s){
            return t == target && s == Signal::LOW;
        };

        reportSolution(countCyclesUntilCondition(findByName(mutableCopy, BROADCASTER_NAME), stopCondition));
    }

    void parseBenchReset() override {
        moduleBlueprint.clear();
    }

private:
    // immutable blueprint of the parsed input.
    // Solvers should make copies of this object; This is to allow repeatability during benchmarking,
    // And disallow interference of subsequent solvers.
    std::vector<std::unique_ptr<const Module>> moduleBlueprint;
    static constexpr std::string BROADCASTER_NAME = "broadcaster";

    // very hairy code due to the use of pointers in member variables of Module.
    void createMutableCopy(std::vector<std::unique_ptr<Module>>& modules) const {
        modules.clear();
        for (auto& ptr : moduleBlueprint) {
            modules.emplace_back(ptr->clone());

            // ConjunctModules should have their memory cleared, they hold pointers to the non-mutable instances of module.
            auto * copy = modules.back().get();
            auto * derivedCopy = dynamic_cast<ConjunctModule *>(copy);
            if (derivedCopy != nullptr) {
                derivedCopy->inputConnections.clear();
            }
        }
        // need to update the outputConnections vector also, or we are pointing to const Modules in the blueprint! very dangerous.
        for (auto& ptr : modules) {
            for (auto& oldTarget : ptr->outputConnections) {
                const std::string& name = oldTarget->name;
                auto iter = std::find_if(modules.begin(), modules.end(), [&name](auto& newP) {
                    return newP->name == name;
                });
                if (iter == modules.end()) throw std::logic_error("Could not find " + name + " In mutable copy?");

                oldTarget = iter->get(); // this is a manual instance of addConnection(), rather, it is a swap.
                // oldTarget is now updated. Notify it that it was connected to ptr.
                oldTarget->connectedTo(ptr.get());
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

    [[nodiscard]] static int countCyclesUntilCondition(
            Module * startingPoint,
            const std::function<bool(Module*, Module*, Signal)>& stopCondition
    ) {
        int i = 0;
        bool work = true;

        auto callback = [&work, &stopCondition](Module* t,Module* f,Signal s) -> void {
            bool shouldStop = stopCondition(t,f,s);
            if (work && shouldStop) {
                work = false;
            }
        };

        while (work) {
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
};

} // namespace

#undef DAY