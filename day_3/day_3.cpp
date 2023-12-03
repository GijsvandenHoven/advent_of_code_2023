#define STR(x) #x
#define FILE_PATH(num)  "day_" STR(num) "/day" STR(num) ".txt"

#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <map>

#define DAY 3
#define FILE FILE_PATH(DAY)

namespace v1 {
    int solve(std::ifstream& text);
}

namespace v2 {
    uint64_t solve(std::ifstream& text);

    struct Gear {
        std::vector<int> parts;
    };
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
    int xcoord = 0;
    int ycoord = 0;

    std::set<uint64_t> safe_spaces;
    auto coord_to_id = [](uint64_t x, uint64_t y){ return x << 32 | y; };

    int c = 0;
    while (c != EOF) {
        c = text.get();
        switch (c) {
            case '.': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                break; // ignore these
            case '\n':
            case EOF:
                xcoord = -1; // disgusting. Necessary due to incrementing at the end of this loop.
                ycoord++;
                break;
            default: // not dot, not number, is symbol.
                // around the dot add coords to the safe_spaces. going "out of bounds" e.g. negative nums is OK,
                // numbers check for safe spaces not the other way around.
                safe_spaces.emplace(coord_to_id(xcoord-1, ycoord-1));
                safe_spaces.emplace(coord_to_id(xcoord+0, ycoord-1));
                safe_spaces.emplace(coord_to_id(xcoord+1, ycoord-1));

                safe_spaces.emplace(coord_to_id(xcoord-1, ycoord+0));
                safe_spaces.emplace(coord_to_id(xcoord+0, ycoord+0)); // redundant.
                safe_spaces.emplace(coord_to_id(xcoord+1, ycoord+0));

                safe_spaces.emplace(coord_to_id(xcoord-1, ycoord+1));
                safe_spaces.emplace(coord_to_id(xcoord+0, ycoord+1));
                safe_spaces.emplace(coord_to_id(xcoord+1, ycoord+1));
        }
        xcoord ++;
    }

    // now reset the stream.
    text.clear();
    text.seekg(0);
    xcoord = 0;
    ycoord = 0;

    // and seek digits adjacent to safe spaces.
    bool digit_parsing_mode = false;
    bool digit_is_safe = false;
    int parsed_number = 0;
    int safe_numbers_sum = 0;

    c = 0;
    while (c != EOF) {
        c = text.get();
        switch (c) {
            case '\n':
            case EOF:
                xcoord = -1; // ugh
                ycoord++;
            default:
                if (digit_parsing_mode && digit_is_safe) {
                    safe_numbers_sum += parsed_number;
                }

                digit_parsing_mode = false;
                digit_is_safe = false;
                parsed_number = 0;
                break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                digit_parsing_mode = true;
                parsed_number *= 10;
                parsed_number += c - '0';

                // for every digit added to the number we should check if it's on a safe space.
                auto coord = coord_to_id(xcoord, ycoord);
                if (safe_spaces.find(coord) != safe_spaces.end()) {
                    digit_is_safe = true;
                }
                break;
        }
        xcoord++;
    }

    return safe_numbers_sum;
}

uint64_t v2::solve(std::ifstream& text) {

    int xcoord = 0;
    int ycoord = 0;

    std::set<uint64_t> safe_spaces;
    std::map<uint64_t, Gear> gears;
    auto coord_to_id = [](uint64_t x, uint64_t y){ return x << 32 | y; };

    int c = 0;
    while (c != EOF) {
        c = text.get();
        switch (c) {
            case '*':
                safe_spaces.emplace(coord_to_id(xcoord-1, ycoord-1));
                safe_spaces.emplace(coord_to_id(xcoord+0, ycoord-1));
                safe_spaces.emplace(coord_to_id(xcoord+1, ycoord-1));

                safe_spaces.emplace(coord_to_id(xcoord-1, ycoord+0));
                safe_spaces.emplace(coord_to_id(xcoord+1, ycoord+0));

                safe_spaces.emplace(coord_to_id(xcoord-1, ycoord+1));
                safe_spaces.emplace(coord_to_id(xcoord+0, ycoord+1));
                safe_spaces.emplace(coord_to_id(xcoord+1, ycoord+1));

                gears.insert(std::make_pair<uint64_t, Gear>(coord_to_id(xcoord, ycoord), {}));
                break;
            case '\n':
            case EOF:
                xcoord = -1; // disgusting. Necessary due to incrementing at the end of this loop.
                ycoord++;
                break;
            default:
                break;
        }
        xcoord ++;
    }

    // now reset the stream.
    text.clear();
    text.seekg(0);
    xcoord = 0;
    ycoord = 0;

    // and seek digits adjacent to safe spaces.
    bool digit_parsing_mode = false;
    bool digit_is_safe = false;
    int parsed_number = 0;


    // I don't know if this is OK or super stupid.
    // After finding a digit in a whitelisted space, what gear does it belong to? we don't know without seeking.
    // We cant seek around it since it is a stream. So defer seeking for later, comparing it against the GearMap.
    // This also easily enables detecting parts contributing to multiple gears, e.g. ".1*512*2.", 512 is connected to 2 gears.
    std::vector<uint64_t> spaces_to_check_for_gears;

    c = 0;
    while (c != EOF) {
        c = text.get();
        switch (c) {
            case '\n':
            case EOF:
                xcoord = -1; // ugh
                ycoord++;
            default:
                if (digit_parsing_mode && digit_is_safe) {
                    // attaches to a '*', but which one?
                    for (auto possible_gear : spaces_to_check_for_gears) {
                        auto gear_seek = gears.find(possible_gear);
                        if (gear_seek != gears.end()) {
                            gear_seek->second.parts.push_back(parsed_number);
                        }
                    }
                }

                digit_parsing_mode = false;
                digit_is_safe = false;
                parsed_number = 0;
                spaces_to_check_for_gears.clear();
                break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                digit_parsing_mode = true;
                parsed_number *= 10;
                parsed_number += c - '0';

                // for every digit added to the number we should check if it's on a safe space.
                auto coord = coord_to_id(xcoord, ycoord);
                if (safe_spaces.find(coord) != safe_spaces.end()) {
                    if (! digit_is_safe) { // first time gear found for this digit
                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord - 1, ycoord - 1));
                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord - 0, ycoord - 1));
                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord + 1, ycoord - 1));

                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord - 1, ycoord - 0));
                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord + 1, ycoord - 0));

                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord - 1, ycoord + 1));
                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord - 0, ycoord + 1));
                        spaces_to_check_for_gears.push_back(coord_to_id(xcoord + 1, ycoord + 1));
                    }

                    digit_is_safe = true;
                }
                break;
        }
        xcoord++;
    }

    uint64_t gearPowerSum = 0;
    for (auto& kvp : gears) {
        auto gear_parts = kvp.second.parts;
        if (gear_parts.size() == 2) { // not a qualifying gear otherwise.
            gearPowerSum += gear_parts[0] * gear_parts[1];
        }
    }

    return gearPowerSum;
}