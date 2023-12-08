#include "Day.hpp"

// This shouldn't even be used undeclared. It's a shame I can't just tell the compiler "make memory for it in static storage and don't touch it".
std::filesystem::path Day::root = "";