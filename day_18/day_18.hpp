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
        int maxX = vertical.back().start.first;

        const auto verticalScannerEnd = horizontal.end();
        const auto horizontalScannerStart = vertical.begin();
        const auto horizontalScannerEnd = vertical.end();

        int64_t surface = 0;
        int y = minY;
        std::cout << "Work until " << maxY << "\n";
        auto verticalScanner = horizontal.begin();
        while (verticalScanner != verticalScannerEnd) {
            // scan this line, what is its surface?
            //std::cout << "scan line " << y << "\n";
            int64_t lineSurface = surfaceOfLine(y, minX, horizontalScannerStart, horizontalScannerEnd);
            //std::cout << "scan line result " << lineSurface << "\n";

            // todo remove this section
            surface += lineSurface;
            y++;
            if (y % 4096 == 0) { std::cout << y << "\n"; }
//            // scan forward until the shape changes. This is indicated by hitting a horizontal piece past our current Y.
//            int newY;
//            while (verticalScanner != verticalScannerEnd) {
//                newY = verticalScanner->start.second;
//                if (newY <= y) {
//                    ++verticalScanner;
//                } else {
//                    break;
//                }
//            }
//            // verticalScanner iterator now points to the first horizontal line piece past y.
//
//            if (verticalScanner == verticalScannerEnd) { // end of the grid. calculate the last lines with the last known surface size.
//                std::cout << "END\n";
//                return 0;
//            } else {
//                // hit a horizontal piece past our current y.
//                // first, how many times the last line was the rectangle?
//                int numberOfLines = newY - y;
//                int64_t rectSize = lineSurface * numberOfLines;
//                std::cout << "Scanned downward to " << newY << ", thus there are " << numberOfLines << " lines w/ surface " << lineSurface << " for a total of " << rectSize << "\n";
//                surface += rectSize;
//                // this line itself is a special case, calculate this line in isolation.
//                int64_t cornerScan = surfaceOfLine(newY, minX, horizontalScannerStart, horizontalScannerEnd);
//                std::cout << "Corner scan result: " << cornerScan << "\n";
//                surface += cornerScan;
//                // prepare the next loop iteration with this y.
//                y = newY + 1;
//            }

            if (y > maxY) break; // shaves a useless iteration whose result would be '0'.
        }

        return surface;
    }

    // scans the surface of a single line given a y (the line), a starting X, and a start & end iterator.
    static int64_t surfaceOfLine(
            const int y,
            const int minX,
            const auto horizontalScannerStart,
            const auto horizontalScannerEnd
    ) {
        int64_t lineSurface = 0;
        int x = minX;
        bool inside = false;
        auto scanner = horizontalScannerStart;

        while (scanner != horizontalScannerEnd) {
            //std::cout << "\tWith " << x << ", " << y << " Currently considering: " << scanner->brief() << "\n";
            int horizontalX = scanner->start.first;
            bool beforeX = horizontalX < x;
            bool intersects = scanner->alignedVertically(y);

            if (beforeX || !intersects) {
                ++scanner;
                continue;
            }

            // We are now at a line piece after the current X, and it is on our path.
            //std::cout << "\t\tINTERSECT " << x << ", " << y << " with " << scanner->brief() << "\n";

            int newX = scanner->start.first;
            int64_t xDiff = newX - x + 1;
            //std::cout << "\t\t\tdiff is " << xDiff << "...\n";

            x = newX;
            if (scanner->isOnEdge(x, y)) {
                //std::cout << "\t\tDevious corner spotted\n";
                if (inside) { // we were inside before hitting the corner. Apply the diff, but one less or we count a tile double.
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
                //std::cout << "\t\tSpotted horizontal line from " << x << " until " << newX << "\n";
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
            //std::cout << "\tEnd of iteration, surface: " << lineSurface << ", inside: " << inside << ", x: " << x << "\n";
            ++scanner; // No matter on corner or normal intersect, always discard the last considered piece, we are done with it.
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