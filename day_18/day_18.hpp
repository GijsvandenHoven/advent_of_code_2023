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
        reportSolution(calculateSurfaceArea(instructions));
    }

    void v2() const override {
        reportSolution(calculateSurfaceArea(correct_instructions));
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
        // int maxX = vertical.back().start.first;

        const auto verticalScannerEnd = horizontal.end();
        const auto horizontalScannerStart = vertical.begin();
        const auto horizontalScannerEnd = vertical.end();

        int64_t surface = 0;
        int y = minY;
        auto verticalScanner = horizontal.begin();

        // Lambda function for finding the next Y target, used for rectangle area detection given a line surface area and the Y in the future.
        // Does not work if the line surface area calculated involved corners, only use for cleanly vertically intersected lines.
        auto findNextYTarget = [maxY](auto iterator, auto iterEnd, int currentY) -> int {
            while (iterator != iterEnd) {
                int newY = iterator->start.second;
                if (newY <= currentY) {
                    ++iterator;
                } else {
                    return newY;
                }
            }
            return maxY + 1;
        };

        while (verticalScanner != verticalScannerEnd) {
            // scan this line, what is its surface?
            auto [lineSurface, hadCorners] = surfaceOfLine(y, minX, horizontalScannerStart, horizontalScannerEnd);

            if (hadCorners) { // can't do any rectangle stuff, do just this line and try the next Y.
                surface += lineSurface;
                y++;
            } else {
                int nextY = findNextYTarget(verticalScanner, verticalScannerEnd, y);

                surface += (lineSurface * (nextY - y));
                y = nextY;
            }

            if (y > maxY) break; // shaves a useless iteration whose result would be '0'.
        }

        return surface;
    }

    // scans the surface of a single line given a y (the line), a starting X, and a start & end iterator.
    static std::pair<int64_t, bool> surfaceOfLine(
            const int y,
            const int minX,
            const auto horizontalScannerStart,
            const auto horizontalScannerEnd
    ) {
        int64_t lineSurface = 0;
        int x = minX;
        bool inside = false;
        bool hadCorners = false; // used in return for the vertical line rectangle recognition.
        auto scanner = horizontalScannerStart;

        while (scanner != horizontalScannerEnd) {
            int horizontalX = scanner->start.first;
            bool beforeX = horizontalX < x;
            bool intersects = scanner->alignedVertically(y);

            if (beforeX || !intersects) {
                ++scanner;
                continue;
            }

            int newX = scanner->start.first;
            int64_t xDiff = newX - x + 1;

            x = newX;
            if (scanner->isOnEdge(x, y)) {
                hadCorners = true;
                if (inside) { // we were inside before hitting the corner. Apply the diff, but one less, or we count a tile double.
                    lineSurface += xDiff - 1;
                }
                // we are intersecting a vertical piece through a corner, i.e. we are on a horizontal line.
                // Go forward until the end of the horizontal line, add that much to the line surface, determine if we are inside or not.
                // Because we always 'go forward until after the horizontal' on hitting a corner, we never hit the front-side of a corner, always the start.
                // It is therefore also impossible to hit horizontalScannerEnd, i.e. we do not need to check for this.
                bool thisCornerFacesUp = y > std::min(scanner->start.second, scanner->end.second);
                do {
                    ++scanner;
                } while (! scanner->alignedVertically(y));
                newX = scanner->start.first;
                int hLen = newX - x + 1;
                lineSurface += hLen; // always add, edges are inside.
                bool newCornerFacesUp = y > std::min(scanner->start.second, scanner->end.second);
                bool flipState = thisCornerFacesUp != newCornerFacesUp;
                if (flipState) { // opposing corners rule.
                    inside = ! inside;
                }
                x = newX + 1; // set the seek point past this X, so we do not count again the corner piece we're at now.
            } else { // cleanly through a vertical line, flip the inside status.
                if (inside) { // we were inside, so applly the diff now.
                    lineSurface += xDiff;
                }
                inside = !inside;
            }
            ++scanner; // No matter on corner or normal intersect, always discard the last considered piece, we are done with it.
        }

        return std::make_pair(lineSurface, hadCorners);
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