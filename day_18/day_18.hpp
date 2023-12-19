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

enum class IntersectionType : uint8_t { // todo this can probably be a bool.
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

    [[nodiscard]] bool isOnEdge(int x, int y) const {
        if (isVertical()) { // the x is unchanging
            return x == start.first && (y == start.second || y == end.second);
        } else if (isHorizontal()) { // the y is unchanging
            return y == start.second && (x == start.first || x == end.first);
        } else {
            throw std::logic_error("Unsupported edge type, neither vertical nor horizontal in isOnEdge");
        }
    }

    // whether this line piece is on a path through line y.
    [[nodiscard]] bool alignedVertically(int y) const {
        if (! isVertical()) return false;

        return (y >= start.second && y <= end.second) || (y >= end.second && y <= start.second);
    }

    [[nodiscard]] bool intersecting(int x, int y) const {
        if (isVertical()) { // unchanging x.
            return x == start.first && ((y >= start.second && y <= end.second) || (y >= end.second && y <= start.second));
        } else if (isHorizontal()) { // unchanging y.
            return y == start.first && ((x >= start.first && x <= end.first) || (x >= end.first && x <= end.first));
        } else throw std::logic_error("Unsupported edge type.");
    }

    [[nodiscard]] std::string brief() const {
        return "(" + std::to_string(start.first) + ", " + std::to_string(start.second) + ") - (" + std::to_string(end.first) + ", " + std::to_string(end.second) + ")";
    }

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

std::ostream& operator<<(std::ostream& os, const LinePiece& l) {
    os << "LinePiece (" << (l.isVertical() ? "vertical" : (l.isHorizontal() ? "horizontal" : "scuffed")) << ") {\n";
    os << "\tstart: (" << l.start.first << ", " << l.start.second << ")\n";
    os << "\tend: (" << l.end.first << ", " << l.end.second << ")\n";
    os << "}";
    return os;
}

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
        reportSolution(calculateSurfaceArea(instructions));
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

    static int64_t calculateSurfaceArea(const std::vector<DigInstruction>& instructions) {
        std::vector<std::pair<int, int>> coords;

        instructionsToCoords(instructions, coords);

        // transform coordinate pairs into line pieces.
        std::vector<LinePiece> vertical;
        std::vector<LinePiece> horizontal;

        coordsToSortedLinePieces(coords, horizontal, vertical);

        int minY = horizontal[0].start.second;
        int maxY = horizontal.back().start.second;
        int minX = vertical[0].start.first;
        int maxX = vertical.back().start.first;

        const auto verticalScannerEnd = horizontal.end();
        const auto horizontalScannerStart = vertical.begin();
        const auto horizontalScannerEnd = vertical.end();

        int64_t surface = 0;
        int y = minY;
        auto verticalScanner = horizontal.begin();
        while (verticalScanner != verticalScannerEnd) {
            // scan this line, what is its surface?
            std::cout << "scan line " << y << "\n";
            int64_t lineSurface = 0;
            bool inside = false;
            bool onHorizontal = false;
            int x = minX;
            auto horizontalScanner = horizontalScannerStart;
            while (horizontalScanner != horizontalScannerEnd) {
                std::cout << "\tWith " << x << ", " << y << " Currently considering: " << horizontalScanner->brief() << "\n";
                // get the difference between our current X and the next vertical line intersecting us.
                // so find the first vertical line intersecting us...
                int horizontalX = horizontalScanner->start.first;

                if (horizontalScanner->start.first < x || ! horizontalScanner->alignedVertically(y)) { // not intersecting, continue
                    ++horizontalScanner;
                    continue;
                }

                std::cout << "\t\tINTERSECTION " << x << ", " << y << " with " << horizontalScanner->brief() << "\n";

                // this piece is past us. how much?
                int diff = horizontalX - x + 1; // include itself, plus one.
                if (inside || onHorizontal) {
                    lineSurface += diff;
                    std::cout << "\t\t\tIncrease linesurface by " << diff << " to " << lineSurface << "\n";
                }

                // update variables.
                if (horizontalScanner->isOnEdge(x, y)) { // riding a corner now, we do this to not flip inside again when we hit the other corner.
                    onHorizontal = ! onHorizontal;
                } else { // cleanly going through a vertical line, we are now flipping the status.
                    inside = ! inside;
                }
                x = horizontalX;
                std::cout << "\t\tNew params. inside: " << inside << ", onHorizontal: " << onHorizontal << ", x: " << x << "\n";
                ++horizontalScanner;
            }
            std::cout << "\tscan line result " << lineSurface << "\n";

            // scan forward until the shape changes. This is indicated by hitting a horizontal piece past our current Y.
            while (verticalScanner != verticalScannerEnd) {
                int newY = verticalScanner->start.second;
                if (newY <= y) {
                    ++verticalScanner;
                } else { // hit a horizontal piece past our current y, for this line we must recalculate.

                    break;
                }
            }

            return 0; // todo remove
        }


        return surface;
    }

    static int64_t surfaceOfLine(int y, int minX, auto horizontalScannerStart, auto horizontalScannerEnd) {
        // scan this line, what is its surface?
        std::cout << "scan line " << y << "\n";
        int64_t lineSurface = 0;
        bool inside = false;
        bool onHorizontal = false;
        int x = minX;
        auto horizontalScanner = horizontalScannerStart;
        while (horizontalScanner != horizontalScannerEnd) {
            std::cout << "\tWith " << x << ", " << y << " Currently considering: " << horizontalScanner->brief() << "\n";
            // get the difference between our current X and the next vertical line intersecting us.
            // so find the first vertical line intersecting us...
            int horizontalX = horizontalScanner->start.first;

            if (horizontalScanner->start.first < x || ! horizontalScanner->alignedVertically(y)) { // not intersecting, continue
                ++horizontalScanner;
                continue;
            }

            std::cout << "\t\tINTERSECTION " << x << ", " << y << " with " << horizontalScanner->brief() << "\n";

            // this piece is past us. how much?
            int diff = horizontalX - x + 1; // include itself, plus one.
            if (inside || onHorizontal) {
                lineSurface += diff;
                std::cout << "\t\t\tIncrease linesurface by " << diff << " to " << lineSurface << "\n";
            }

            // update variables.
            if (horizontalScanner->isOnEdge(x, y)) { // riding a corner now, we do this to not flip inside again when we hit the other corner.
                onHorizontal = ! onHorizontal;
            } else { // cleanly going through a vertical line, we are now flipping the status.
                inside = ! inside;
            }
            x = horizontalX;
            std::cout << "\t\tNew params. inside: " << inside << ", onHorizontal: " << onHorizontal << ", x: " << x << "\n";
            ++horizontalScanner;
        }

        return lineSurface;
    }

    static void instructionsToCoords(const std::vector<DigInstruction>& ins, std::vector<std::pair<int,int>>& coords) {
        coords.clear();
        coords.reserve(ins.size());

        int x = 0;
        int y = 0;
        coords.emplace_back(x, y);
        for (auto &i: ins) {
            switch (i.d) {
                default:
                    throw std::logic_error("Unknonw Direction enum value in v1");
                case Direction::DOWN:
                    y += i.dist;
                    break;
                case Direction::UP:
                    y -= i.dist;
                    break;
                case Direction::LEFT:
                    x -= i.dist;
                    break;
                case Direction::RIGHT:
                    x += i.dist;
                    break;
            }

            coords.emplace_back(x, y);
        }
    }

    static void coordsToSortedLinePieces(
            const std::vector<std::pair<int,int>>& coords,
            std::vector<LinePiece>& horizontal,
            std::vector<LinePiece>& vertical
    ) {
        vertical.clear();
        horizontal.clear();

        vertical.reserve(coords.size() / 2);
        horizontal.reserve(coords.size() / 2);

        if (coords.back() != coords[0]) { throw std::logic_error("Coordinates should complete a ring."); }

        for (int i = 0; i < coords.size(); ++i) {
            int j = (i+1); // every i is with a successor j, except the last one. The coordinates are assumed to complete a ring already.
            if (j == coords.size()) continue;

            LinePiece l(coords[i], coords[j]);

            // Imagine how broken this would be for 0 length line pieces :)
            if (l.isHorizontal()) horizontal.push_back(l);
            else if (l.isVertical()) vertical.push_back(l);
            else throw std::logic_error("Neither vertical nor horizontal line piece.");
        }

        // verticals are sorted by x. A piece is less than another piece when its x is lower.
        std::sort(vertical.begin(), vertical.end(), [](auto& a, auto& b){
            return a.start.first < b.start.first; // be definition of vertical line pieces, for any piece 'L', L.start.first == L.end.first.
        });
        // verticals are sorted by y. A piece is less than another piece when its y is lower.
        std::sort(horizontal.begin(), horizontal.end(), [](auto& a, auto& b) {
            return a.start.second < b.start.second; // L.start.second == L.end.second for any horizontal line piece.
        });
    }
};

} // namespace

#undef DAY