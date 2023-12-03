#define STR(x) #x
#define FILE_PATH(num)  "day_" STR(num) "/day" STR(num) ".txt"

#include <iostream>
#include <fstream>

#define DAY 0
#define FILE FILE_PATH(DAY)

namespace v1 {
    int solve(std::ifstream& text);
}

namespace v2 {
    int solve(std::ifstream& text);
}

int main () {

    {
        std::ifstream input(FILE);
        auto result = v1::solve(input);
        std::cout << "v1: " << result << "\n";
    }
    {
        std::ifstream input(FILE);
        auto result = v2::solve(input);
        std::cout << "v2: " << result << "\n";
    }

    return 0;
}

int v1::solve(std::ifstream& text) {
    return 0;
}

int v2::solve(std::ifstream& text) {
    return 0;
}