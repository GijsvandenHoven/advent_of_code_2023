#include "../util/Day.hpp"
#include <stdexcept>
#include <filesystem>

#define CONCATENATE(x, y) x##y
#define CLASS_DEF(D) class CONCATENATE(Day, D) : public Day
#define DEFAULT_CTOR_DEF(D) explicit CONCATENATE(Day, D) (const std::filesystem::path& p) : Day(D, p) {}

#define PLACEHOLD(DAY) CLASS_DEF(DAY) {             \
public: DEFAULT_CTOR_DEF(DAY)                       \
    void parse(std::ifstream&) override {           \
        throw std::runtime_error("Not Implemented");\
    }                                               \
    void parseBenchReset() override {               \
        throw std::runtime_error("Not implemented");\
    }                                               \
    void v1() const override {                      \
        throw std::runtime_error("Not Implemented");\
    }                                               \
    void v2() const override {                      \
        throw std::runtime_error("Not Implemented");\
    }                                               \
};

// to get main to compile. As classes for days get created, placeholders get commented out.

//PLACEHOLD(1)
//PLACEHOLD(2)
//PLACEHOLD(3)
//PLACEHOLD(4)
//PLACEHOLD(5)
//PLACEHOLD(6)
//PLACEHOLD(7)
//PLACEHOLD(8)
PLACEHOLD(9)
PLACEHOLD(10)
PLACEHOLD(11)
PLACEHOLD(12)
PLACEHOLD(13)
PLACEHOLD(14)
PLACEHOLD(15)
PLACEHOLD(16)
PLACEHOLD(17)
PLACEHOLD(18)
PLACEHOLD(19)
PLACEHOLD(20)
PLACEHOLD(21)
PLACEHOLD(22)
PLACEHOLD(23)
PLACEHOLD(24)
PLACEHOLD(25)

#undef CONCATENATE
#undef CLASS_DEF
#undef DEFAULT_CTOR_DEF
#undef DAY