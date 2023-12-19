#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 18

NAMESPACE_DEF(DAY) {

enum class Direction : uint8_t {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

enum class IntersectionType : uint8_t {
    NONE = 0,
    ON,
    INTERSECT
};

struct DigInstruction {
    Direction d;
    int dist;

    uint32_t rgba;

    explicit DigInstruction(const std::string& from) : dist(0), rgba(0) {
        std::stringstream s(from);
        switch (s.get()) {
            default: throw std::logic_error("Unknown direction in DigInstruction: " + from);
            case 'U': d = Direction::UP; break;
            case 'D': d = Direction::DOWN; break;
            case 'L': d = Direction::LEFT; break;
            case 'R': d = Direction::RIGHT; break;
        }

        s >> dist;

        while (s.get() != '#') { /* pass until we are ready to eat the hex string. */ }

        s << std::hex;
        s >> rgba;
    }

    DigInstruction(int distance, Direction direction) : dist(distance), d(direction), rgba(0) {}

    static DigInstruction generateCorrected(const DigInstruction& faulty) {
        int distance = static_cast<int>(faulty.rgba >> 4);
        Direction direction;
        switch (faulty.rgba & 0xF) {
            default: throw std::logic_error("Unknown distance enum decoding");
            case 0: direction = Direction::RIGHT;   break;
            case 1: direction = Direction::DOWN;   break;
            case 2: direction = Direction::LEFT;   break;
            case 3: direction = Direction::UP;      break;
        }

        return {distance, direction};
    }
};

struct LinePiece {
    std::pair<int, int> start;
    std::pair<int, int> end;

    [[nodiscard]] bool isVertical() const { return start.first == end.first; }
    [[nodiscard]] bool isHorizontal() const { return start.second == end.second; }

    [[nodiscard]] IntersectionType testVerticalIntersection(int x, int y) const {
        if (start.first == end.first) { // x does not change, this is a vertical piece that may intersect.
            if (x == start.first && x == end.first) {
                if ((y >= start.second && y <= end.second) || (y >= end.second && y <= start.second)) {
                    return IntersectionType::INTERSECT;
                } else {
                    return IntersectionType::NONE;
                }
            } else { // x does not match
                return IntersectionType::NONE;
            }
        } else { // horizontal piece, we might be on it though.
            if (y == start.second && y == end.second) {
                if ((x >= start.first && x <= end.first) || (x >= end.first && x <= start.first)) {
                    return IntersectionType::ON;
                } else { // Y matches, but it's not on top of it.
                    return IntersectionType::NONE;
                }
            } else { // nope, Y doesn't match.
                return IntersectionType::NONE;
            }
        }
    }
};

struct LinePieceList : private std::vector<LinePiece> {
    explicit LinePieceList(const std::vector<std::pair<int,int>>& coords) {
        this->reserve(coords.size());

        for (int i = 0; i < coords.size(); ++i) {
            int next = (i + 1) % static_cast<int>(coords.size());

            this->emplace_back(coords[i], coords[next]);
        }
    }

    [[nodiscard]] std::pair<const_iterator, const_iterator> testVerticalIntersection(int x, int y) const {
        // we have an intersection if we are on a line piece that is headed orthogonal to us.
        // A special case is for if we are on a line piece that is vertical just like us. Then we are 'ON' the piece.
        auto horizontalHit = this->end();
        auto verticalHit = this->end();
        auto iter = this->begin();
        for ( ; iter != this->end(); ++iter) {
            auto result = iter->testVerticalIntersection(x, y);
            if (result != IntersectionType::NONE) {
                if (iter->isHorizontal()) { // it could be a corner piece, and verticals should take priority. Keep looking.
                    horizontalHit = iter;
                } else {
                    verticalHit = iter;
                }

                if (verticalHit != this->end() && horizontalHit != this->end()) { // corner hit, we are done.
                    return std::make_pair(verticalHit, horizontalHit);
                }
            }
        }
        return std::make_pair(verticalHit, horizontalHit);
    }

    [[nodiscard]] const_iterator _end() const { return this->end(); }
};

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            instructions.emplace_back(line);
            correct_instructions.emplace_back(DigInstruction::generateCorrected(instructions.back()));
        }
    }

    void v1() const override {
        std::vector<std::pair<int, int>> coords;
        coords.reserve(instructions.size());

        {
            int x = 0;
            int y = 0;
            coords.emplace_back(x, y);
            for (auto &ins: instructions) {
                switch (ins.d) {
                    default:
                        throw std::logic_error("Unknonw Direction enum value in v1");
                    case Direction::DOWN:
                        y += ins.dist;
                        break;
                    case Direction::UP:
                        y -= ins.dist;
                        break;
                    case Direction::LEFT:
                        x -= ins.dist;
                        break;
                    case Direction::RIGHT:
                        x += ins.dist;
                        break;
                }

                coords.emplace_back(x, y);
            }
        }

        LinePieceList lines(coords);

        // find minX, maxX, minY, maxY to get an upper bound on the size of this polygon.
        int minX = std::min_element(coords.begin(), coords.end(), [](auto& a, auto& b) {
            return a.first < b.first;
        })->first;
        int maxX = std::min_element(coords.begin(), coords.end(), [](auto& a, auto& b) {
            return a.first > b.first;
        })->first;
        int minY = std::min_element(coords.begin(), coords.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        })->second;
        int maxY = std::min_element(coords.begin(), coords.end(), [](auto& a, auto& b) {
            return a.second > b.second;
        })->second;

        int surface = (maxX - minX + 1) * (maxY - minY + 1);

        for (int y = minY; y <= maxY; ++y) {
            bool inside = false;
            for (int x = minX; x <= maxX; ++x) {
                // std::cout << "Test " << x << ", " << y << "\n";
                auto [verticalHit, horizontalHit] = lines.testVerticalIntersection(x, y);

                if (verticalHit != lines._end() && horizontalHit != lines._end()) { // corner!
                    // up-corner or down-corner? Up corners flip, down corners do not.
                    int top = std::min(verticalHit->start.second, verticalHit->end.second); // 'down' is more.
                    if (top == y) { // down-corner.
                        // std::cout << "\t\tdown corner, no flip\n";
                    } else {
                        // std::cout << "\t\tup corner, flip\n";
                        inside = ! inside;
                    }
                } else if (verticalHit != lines._end()) {
                    // std::cout << "\t\tvertical\n";
                    inside = ! inside;
                } else if (horizontalHit != lines._end()) { // we are on a horizontal piece. Skip x ahead to the corner.
                        // std::cout << "\t\thorizontal\n";
                        x = std::max(horizontalHit->start.first, horizontalHit->end.first) - 1;
                } else {
                    // std::cout << "\tIt is not on anything.\n";
                    surface -= (inside ? 0 : 1);
                    // std::cout << "\t\tnew surface: " << surface << "\n";
                }
            }
        }

        reportSolution(surface);
    }

    // todo: revise problem 1 to do it with some kind of sort on the line pieces instead.
    //  Such that we can always skip to the next line pieces of a line.
    void v2() const override {
        reportSolution(0);
    }

    void parseBenchReset() override {
        instructions.clear();
        correct_instructions.clear();
    }

private:
    std::vector<DigInstruction> instructions;
    std::vector<DigInstruction> correct_instructions;
};

} // namespace

#undef DAY