#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 21

// printing parts of the derivation / calculation process.
#define DO_P2_COUT false

// instead of using the problem input,
// use an empty grid for comparign with the constexpr derivation & the bruteforce reference value.
// The solution reported is 1 if equal (what we want to see), 0 otherwise.
#define DO_P2_EMPTY_GRID_ALGO_COMPARE false

NAMESPACE_DEF(DAY) {

// Mathematical derivation of the input size given an empty grid.
// The formula derived is re-used in part 2,
// where the formulae for volumes on full, tippy, inner & outer corner grids are replaced by numbers found by BFS.
constexpr int GRID_SIZE = 131;
constexpr int N_STEPS = 26501365;

// grid size should be an odd number.
static_assert(GRID_SIZE % 2 == 1);

// nsteps - floor(gridsize/2) should be perfectly divisible by grid size.
static_assert((N_STEPS - (GRID_SIZE / 2)) % GRID_SIZE == 0);

constexpr int64_t N_FULL_GRIDS_PER_DIRECTION = (N_STEPS - (GRID_SIZE / 2)) / GRID_SIZE - 1;
constexpr int64_t N_RINGS_OF_FULL_GRIDS = N_FULL_GRIDS_PER_DIRECTION; // cheeky little fact about that.

// we are assuming the perimeter is oddly matched, so the outer complete grid ring is evenly matched.
static_assert(N_RINGS_OF_FULL_GRIDS % 2 == 1 && N_STEPS % 2 == 1);

constexpr int64_t N_EVEN_GRID_RINGS = (N_RINGS_OF_FULL_GRIDS / 2) + 1;
constexpr int64_t N_ODD_GRID_RINGS = N_RINGS_OF_FULL_GRIDS / 2;

// for reference on what is really going on.
constexpr auto PYRAMID_VOLUME = [](int64_t n) constexpr -> int64_t { return ((n+1)*n) / 2; };
constexpr auto ODD_GRID_COUNT_FORMULA = [](int64_t n_grid_rings) constexpr -> int64_t {
    return 1 + 8 * PYRAMID_VOLUME(n_grid_rings);
};
constexpr auto EVEN_GRID_COUNT_FORMULA = [](int64_t n_grid_rings) constexpr-> int64_t {
    return 4 * n_grid_rings + 8 * PYRAMID_VOLUME(n_grid_rings-1);
};

constexpr int64_t ODD_GRID_COUNT = ODD_GRID_COUNT_FORMULA(N_ODD_GRID_RINGS);
constexpr int64_t EVEN_GRID_COUNT = EVEN_GRID_COUNT_FORMULA(N_EVEN_GRID_RINGS);

constexpr int64_t N_EVEN_PLACES_IN_EMPTY_GRID = GRID_SIZE * GRID_SIZE / 2 + 1; // works because we asserted grid_size is an odd number.
constexpr int64_t N_ODD_PLACES_IN_EMPTY_GRID = GRID_SIZE * GRID_SIZE / 2;

constexpr int64_t N_PLACES_IN_FULL_GRIDS = N_ODD_PLACES_IN_EMPTY_GRID * ODD_GRID_COUNT + N_EVEN_PLACES_IN_EMPTY_GRID * EVEN_GRID_COUNT;

constexpr int64_t N_CORNERS_PER_SIDE = N_FULL_GRIDS_PER_DIRECTION * 2 + 1; // an inner and outer per tile, plus 1 inner for tippy.
constexpr int64_t N_INNER_PER_SIDE = N_CORNERS_PER_SIDE / 2 + 1;
constexpr int64_t N_OUTER_PER_SIDE = N_CORNERS_PER_SIDE / 2;

// for a grid, the value of tiles per inner/outer is the same on each side, so we can take shortcuts here.
auto SUM_N_DECR_BY_2 = [](int64_t n) constexpr -> int64_t { return (n+1)*(n+1)/4; };
constexpr int64_t N_INNER_CORNER_PLACES_IN_EMPTY_GRID = SUM_N_DECR_BY_2(GRID_SIZE/2); // (k+1)(k+1)/4
constexpr int64_t N_OUTER_CORNER_PLACES_IN_EMPTY_GRID = N_ODD_PLACES_IN_EMPTY_GRID - SUM_N_DECR_BY_2(GRID_SIZE/2-1);

constexpr int64_t N_TILES_ON_INNER_CORNERS = N_INNER_CORNER_PLACES_IN_EMPTY_GRID * N_INNER_PER_SIDE * 4;
constexpr int64_t N_TILES_ON_OUTER_CORNERS = N_OUTER_CORNER_PLACES_IN_EMPTY_GRID * N_OUTER_PER_SIDE * 4;

constexpr int64_t N_TIPPY_TILES = 4 * (N_ODD_PLACES_IN_EMPTY_GRID - 2 * SUM_N_DECR_BY_2(GRID_SIZE/2-1));

constexpr int64_t TOTAL_IF_EMPTY_GRID = N_TILES_ON_INNER_CORNERS + N_TILES_ON_OUTER_CORNERS + N_PLACES_IN_FULL_GRIDS + N_TIPPY_TILES;

// this entire formula for an empty grid should equal to the surface of the diamond we're making. Great sanity check.
static_assert(TOTAL_IF_EMPTY_GRID == static_cast<int64_t>(N_STEPS + 1) * static_cast<int64_t>(N_STEPS + 1));

struct Tile {
    bool occupied;
};

constexpr int UNVISITED_DIST = std::numeric_limits<int>::min();
struct PathfindingTile : public Tile {
    int dist = UNVISITED_DIST;

    explicit PathfindingTile(bool b) : Tile(b) {}
    explicit PathfindingTile(const Tile& t) : Tile(t) {}
};

template <typename T>
requires std::is_base_of_v<Tile, T>
class Garden : private std::vector<std::vector<T>> {
    template <typename V>
    friend std::ostream& operator<<(std::ostream& os, const Garden<V>& g);
public:
    int startX = -1;
    int startY = -1;

    [[nodiscard]] T& at(int x, int y) { return this->operator[](y)[x]; }

    [[nodiscard]] std::tuple<bool, int, int> up(int x, int y) {
        int newY = y - 1;
        if (newY < 0) return {false, 0, 0};

        return {true, x, newY};
    }

    [[nodiscard]] std::tuple<bool, int, int> down(int x, int y) {
        int newY = y + 1;
        if (newY >= this->size()) return {false, 0, 0};

        return {true, x, newY};
    }

    [[nodiscard]] std::tuple<bool, int, int> left(int x, int y) {
        int newX = x - 1;
        if (newX < 0) return {false, 0, 0};

        return {true, newX, y};
    }

    [[nodiscard]] std::tuple<bool, int, int> right(int x, int y) {
        int newX = x + 1;
        if (this->size() == 0 || newX > this->back().size()) return {false, 0, 0};

        return {true, newX, y};
    }

    void addRow(const std::string& row) {
        std::istringstream s(row);
        this->emplace_back(); // empty vec to the back, to be filled
        this->back().reserve(row.size());

        int c;
        int i = 0;
        while ((c = s.get()) != EOF) {
            this->back().emplace_back(c == '#');

            if (c == 'S') {
                startX = i;
                startY = static_cast<int>(this->size() - 1);
            }

            i++;
        }
    }

    void addRow(std::vector<T>&& r) {
        this->emplace_back(r);
    }

    void mutableCopy(Garden<PathfindingTile>& g) const {
        g.startX = startX;
        g.startY = startY;
        for (auto& r : *this) {
            std::vector<PathfindingTile> copy;
            for (auto& t : r) {
                copy.emplace_back(PathfindingTile(t));
            }
            g.addRow(std::move(copy));
        }
    }

    template <typename U = T>
    [[nodiscard]] typename std::enable_if_t<std::is_same_v<U, PathfindingTile>, int> testReachability(int nSteps) const {
        int count = 0;
        int parity = nSteps % 2;
        for (auto& row : *this) {
            for (auto& t : row) {
                int dist = t.dist;
                int thisParity = dist % 2;
                if (dist != UNVISITED_DIST && dist <= nSteps && (parity == thisParity)) {
                    count += 1;
                }
            }
        }
        return count;
    }

    void dims() const {
        std::cout << this->size() << ", " << this->back().size() << "\n";
    }

    static void makeBlankGrid(int size, Garden<T>& g) {
        g.clear();
        std::string s = ".";
        std::istringstream ss(s);
        std::ostringstream buf;
        auto readN = [&ss, &buf](int n) {
            for (int i = 0; i < n; ++i) {
                buf << static_cast<char>(ss.get());
                ss.clear();
                ss.seekg(0);
            }
        };


        for (int i = 0; i < size; ++i) {
            readN(size);
            g.addRow(buf.str());
            buf.clear();
            buf.str(std::string());
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Tile& t) {
    return t.occupied ? os << '#' : os << '.';
}

std::ostream& operator<<(std::ostream& os, const PathfindingTile& t) {
    return t.occupied ? os << "#" : ( t.dist == UNVISITED_DIST ? os << "." : os << t.dist);
}

template <typename V>
std::ostream& operator<<(std::ostream& os, const Garden<V>& g) {
    int y = 0;
    for (auto& row : g) {
        int x = 0;
        for (auto& t : row) {
            if (y == g.startY && x == g.startX) {
                os << 'S';
            } else {
                os << t;
            }
            x++;
        }
        y++;
        os << '\n';
    }
    return os;
}

using BlueprintGarden = Garden<Tile>;
using MutableGarden = Garden<PathfindingTile>;

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            grid.addRow(line);
        }

        if (grid.startX < 0 || grid.startY < 0) throw std::logic_error("Start position was not set.");
    }

    void v1() const override {
        MutableGarden copy;
        grid.mutableCopy(copy);
        BFS(copy, grid.startX, grid.startY);

        int reachable = copy.testReachability(64);

        reportSolution(reachable);
    }

    void v2() const override {

#if DO_P2_COUT
        std::cout << "CONSTEXPR = " << TOTAL_IF_EMPTY_GRID << "\n";
#endif

        MutableGarden _seed_;
#if DO_P2_EMPTY_GRID_ALGO_COMPARE
        MutableGarden::makeBlankGrid(GRID_SIZE, _seed_);
#else
        grid.mutableCopy(_seed_);
#endif
        const MutableGarden blueprint = std::move(_seed_);

#if DO_P2_COUT
        if constexpr (N_STEPS < 1000) {
            MutableGarden reference;
            MutableGarden::makeBlankGrid(2 * N_STEPS + 1, reference);
            BFS(reference, N_STEPS, N_STEPS);
            std::cout << "BRUTEFORCE REF VALUE = " << reference.testReachability(N_STEPS) << "\n";
        } else {
            std::cout << "NO BRUTEFORCE REF VALUE, TOO LARGE\n";
        }
#endif

        MutableGarden full_grid = blueprint;
        BFS(full_grid, GRID_SIZE/2, GRID_SIZE/2);
        int reachable_odd = full_grid.testReachability(GRID_SIZE);
        int reachable_even = full_grid.testReachability(GRID_SIZE+1);

        auto reachabilityFrom = [&blueprint](int startX, int startY, int n_steps) -> int {
            auto worksheet = blueprint; // copy the blueprint into a version we can mutate.
            BFS(worksheet, startX, startY);
            return worksheet.testReachability(n_steps);
        };

        // left tippy enters from mid-right, and has GRID_SIZE-1 steps left. (minus one for entering this grid)
        int64_t leftTippyReach = reachabilityFrom(GRID_SIZE-1, GRID_SIZE/2, GRID_SIZE-1);
        // right tippy enters from mid-left, and has GRID_SIZE-1 steps left.
        int64_t rightTippyReach = reachabilityFrom(0, GRID_SIZE/2, GRID_SIZE-1);
        // top tippy enters from mid-bottom, and has GRID_SIZE-1 steps left.
        int64_t topTippyReach = reachabilityFrom(GRID_SIZE/2, GRID_SIZE-1, GRID_SIZE-1);
        // bottom tippy enters from mid-top, and has GRID_SIZE-1 steps left.
        int64_t bottomTippyReach = reachabilityFrom(GRID_SIZE/2, 0, GRID_SIZE-1);

        int64_t tippyReachSum = leftTippyReach + rightTippyReach + topTippyReach + bottomTippyReach;

        // top-right inner corner enters from top right, and has GRID_SIZE/2-1 steps left. (Half lost from moving to the corner in a straight line)
        int64_t TRInnerReach = reachabilityFrom(GRID_SIZE-1, 0, GRID_SIZE/2-1);
        // top-right outer corner enters from top right, and has GRID_SIZE+GRID_SIZE/2-1 steps left. (half loss from going down, but down 1 grid earlier than an edge)
        int64_t TROuterReach = reachabilityFrom(GRID_SIZE-1, 0, GRID_SIZE+GRID_SIZE/2-1);

        // top-left inner corner enters from top left, and has GRID_SIZE/2-1 steps left.
        int64_t TLInnerReach = reachabilityFrom(0, 0, GRID_SIZE/2-1);
        // top-right outer corner enters from top left, and has GRID_SIZE+GRID_SIZE/2-1 steps left.
        int64_t TLOuterReach = reachabilityFrom(0, 0, GRID_SIZE+GRID_SIZE/2-1);

        // bot-left inner corner enters from bottom left, and has GRID_SIZE/2-1 steps left.
        int64_t BLInnerReach = reachabilityFrom(0, GRID_SIZE-1, GRID_SIZE/2-1);
        // bot-left outer corner enters from bottom left, and has GRID_SIZE+GRID_SIZE/2-1 steps left.
        int64_t BLOuterReach = reachabilityFrom(0, GRID_SIZE-1, GRID_SIZE+GRID_SIZE/2-1);

        // bot-right inner corner enters from bottom right, and has GRID_SIZE/2-1 steps left.
        int64_t BRInnerReach = reachabilityFrom(GRID_SIZE-1,GRID_SIZE-1, GRID_SIZE/2-1);
        // bot-right outer corner enters from bottom right, and has GRID_SIZE+GRID_SIZE/2-1 steps left.
        int64_t BROuterReach = reachabilityFrom(GRID_SIZE-1, GRID_SIZE-1, GRID_SIZE+GRID_SIZE/2-1);

        int64_t grid_reach = (N_STEPS - GRID_SIZE/2) / GRID_SIZE;  // with steps as what remains after escaping the core. how many grids can we span?
        int64_t full_grid_reach = grid_reach - 1; // the last grid is not full, it's a tippy. The 'central' grid is also not counted.

        // assumption: the tippy grids are odd-matching.
        // thus the outer ring of full grids is even-matching, and of this there are more.
        int64_t full_grid_reach_even_partition = full_grid_reach / 2 + 1;
        int64_t full_grid_reach_odd_partition = full_grid_reach / 2;

        // each subsequent ring has 4 more grids.
        // Thus, since odd and even are alternating, each subsequent odd/even ring has 8 more grids (2x4)
        int64_t full_grid_total_even = 4 * full_grid_reach_even_partition + 8 * PYRAMID_VOLUME(full_grid_reach_even_partition - 1);
        int64_t full_grid_total_odd = 1 /*core*/ + 8 * PYRAMID_VOLUME(full_grid_reach_odd_partition);

        int64_t fullGridSum = full_grid_total_even * reachable_even + full_grid_total_odd * reachable_odd;

        int64_t n_inner_corners_per_side = grid_reach; // every grid including tippy has one matched inner corner per side.
        int64_t n_outer_corners_per_side = full_grid_reach; // every grid except tippy has one matched outer corner per side.

        int64_t n_tiles_in_inner_TR = TRInnerReach * n_inner_corners_per_side;
        int64_t n_tiles_in_outer_TR = TROuterReach * n_outer_corners_per_side;

        int64_t n_tiles_in_inner_TL = TLInnerReach * n_inner_corners_per_side;
        int64_t n_tiles_in_outer_TL = TLOuterReach * n_outer_corners_per_side;

        int64_t n_tiles_in_inner_BL = BLInnerReach * n_inner_corners_per_side;
        int64_t n_tiles_in_outer_BL = BLOuterReach * n_outer_corners_per_side;

        int64_t n_tiles_in_inner_BR = BRInnerReach * n_inner_corners_per_side;
        int64_t n_tiles_in_outer_BR = BROuterReach * n_outer_corners_per_side;

        int64_t innerSum = n_tiles_in_inner_BR + n_tiles_in_inner_BL + n_tiles_in_inner_TR + n_tiles_in_inner_TL;
        int64_t outerSum = n_tiles_in_outer_BR + n_tiles_in_outer_BL + n_tiles_in_outer_TR + n_tiles_in_outer_TL;

        int64_t cornerSum = innerSum + outerSum;

#if DO_P2_COUT
        std::cout << "ro " << reachable_odd << ", re " << reachable_even << "\n";
        std::cout << "tippies: " << leftTippyReach << ", " << rightTippyReach << ", " << topTippyReach << ", " << bottomTippyReach << "\n";
        std::cout << "\ttippySum: " << tippyReachSum << "\n";
        std::cout << "outers: " << BROuterReach << ", " << BLOuterReach << ", " << TLOuterReach << ", " << TROuterReach << "\n";
        std::cout << "inners: " << BRInnerReach << ", " << BLInnerReach << ", " << TLInnerReach << ", " << TRInnerReach << "\n";
        std::cout << "fgte " << full_grid_total_even << ", fgto: " << full_grid_total_odd << "\n";
        std::cout << "fgs " << fullGridSum << "\n";
        std::cout << "inners: BR " << n_tiles_in_inner_BR << ", BL " << n_tiles_in_inner_BL << ", TL " << n_tiles_in_inner_TL << ", TR " << n_tiles_in_inner_TR << "\n";
        std::cout << "outers: BR " << n_tiles_in_outer_BR << ", BL " << n_tiles_in_outer_BL << ", TL " << n_tiles_in_outer_TL << ", TR " << n_tiles_in_outer_TR << "\n";
        std::cout << "Corner sum " << cornerSum << "\n";
#endif
#if DO_P2_EMPTY_GRID_ALGO_COMPARE
        reportSolution(TOTAL_IF_EMPTY_GRID == cornerSum + tippyReachSum + fullGridSum);
#else
        int64_t solution = cornerSum + tippyReachSum + fullGridSum;
        if (solution > TOTAL_IF_EMPTY_GRID) {
            throw std::logic_error(
                    "solution exceeds upper bound of " + std::to_string(TOTAL_IF_EMPTY_GRID)
                    + " Is the constexpr upper bound correct? grid size: "
                    + std::to_string(GRID_SIZE) + ", N_STEPS: " + std::to_string(N_STEPS) + "."
            );
        }
        reportSolution(solution);
#endif
    }

    void parseBenchReset() override {
        grid = BlueprintGarden(); // private inheritance, no clear. So let's just yeet it all.
    }

private:
    BlueprintGarden grid;

    static void BFS(MutableGarden& subject, int startX, int startY) {

        std::queue<std::tuple<int,int,int>> bfs; // x,y,dist.
        bfs.emplace(startX, startY, 0);
        subject.at(startX, startY).dist = 0;

        while (! bfs.empty()) {
            auto [x, y, d] = bfs.front();
            bfs.pop();

            std::array<std::tuple<bool,int,int>,4> next = { subject.up(x,y), subject.down(x,y), subject.left(x,y), subject.right(x,y) };

            for (auto [ok, nx, ny] : next) {
                if (! ok) continue;

                auto& maybe = subject.at(nx, ny);
                if (! maybe.occupied && maybe.dist == UNVISITED_DIST) {
                    bfs.emplace(nx, ny, d+1);
                    maybe.dist = d + 1;
                }
            }
        }
    }
};

} // namespace

#undef DAY
#undef DO_P2_COUT
#undef DO_P2_EMPTY_GRID_ALGO_COMPARE