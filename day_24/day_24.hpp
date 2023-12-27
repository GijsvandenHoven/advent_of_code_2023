#pragma once

#include <iostream>

#include "../util/Day.hpp"
#include "../util/macros.hpp"

#define DAY 24

NAMESPACE_DEF(DAY) {

struct Object {
    struct R3 { int64_t x; int64_t y; int64_t z; };

    R3 start{};
    R3 delta{};

    explicit Object(const std::string& str) {
        constexpr auto getN = [](int64_t& target, auto& stream) -> void {
            stream >> target;
            stream.ignore(2);
        };

        std::istringstream s(str);
        getN(start.x, s);
        getN(start.y, s);
        getN(start.z, s);

        getN(delta.x, s);
        getN(delta.y, s);
        getN(delta.z, s);
    }
};

std::ostream& operator<<(std::ostream& os, const Object::R3& r) {
    os << "x: " << r.x << ", y: " << r.y << ", z: " << r.z;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Object& o) {
    os << "Obj; start: (" << o.start << "), delta: (" << o.delta << ")";
    return os;
}

CLASS_DEF(DAY) {
public:
    DEFAULT_CTOR_DEF(DAY)

    void parse(std::ifstream &input) override {
        std::string line;
        while (std::getline(input, line)) {
            objects.emplace_back(line);
        }
    }

    void v1() const override {
        int intersectCount = 0;
        for (int i = 0; i < objects.size(); ++i) {
            for (int j = i + 1; j < objects.size(); ++j) {
                intersectCount += testIntersectXY(objects[i], objects[j]);
            }
        }

        reportSolution(intersectCount);
    }

    void v2() const override {
        /**
         * (define-fun B () Int
    229429688799267)
  (define-fun D () Int
    217160931330282)
  (define-fun F () Int
    133453231437025)
)
         */ // note: apparently does not work for some other input.txt , unless you use Real instead of Int. But the output is all Integers anyway. ???????????
        std::cout << (229429688799267LL + 217160931330282LL + 133453231437025LL) << "\n";
        reportSolution(P2MakeZ3Model());
    }

    void parseBenchReset() override {
        objects.clear();
    }

private:
    std::vector<Object> objects;
    const struct {
        int64_t ylow = 200000000000000;
        int64_t xlow = 200000000000000;
        int64_t yhigh = 400000000000000;
        int64_t xhigh = 400000000000000;
    } P1;

    /**
     * Given two objects in R3 space, check if they would intersect inside the area described by P1.
     *
     * A formula for the intersection is derived as such:
     * Ay = a t1 + b
     * By = c t2 + d
     * Ax = e t1 + f
     * Bx = g t2 + h
     *
     * Where Ay = By and Ax = Bx, by definition of intersection.
     *
     * Let t1x be the time at which object A touches the point of intersection, that is, object B will cross here too at some time.
     * Let t2x be the time at which object B touches the point of intersection.
     *
     * This system of equations can be algebraically transformed as such:
     * t1x = (gd - gb + cf - ch) / (ga - ce)
     * t2x = (be - de + ha - fa) / (ce - ga)
     *
     * When (ga - ce) is zero, the lines must be parallel; they will never cross, false will be returned.
     * When any of the two t is negative, the lines have crossed in the "past", and false will be returned.
     * When both t is positive, X and Y are calculated (Ax and Ay specifically), and true is returned iff this is within P1.
     * Some care is taken around casting high values to double (Namely the puzzle works with 100 trillion+ values, and this is close to the 53 bit integer limit of dbls)
     */
    [[nodiscard]] bool testIntersectXY(const Object& obj1, const Object& obj2) const {
        // Ay
        int64_t a = obj1.delta.y;
        int64_t b = obj1.start.y;
        // By
        int64_t c = obj2.delta.y;
        int64_t d = obj2.start.y;
        // Ax
        int64_t e = obj1.delta.x;
        int64_t f = obj1.start.x;
        // Bx
        int64_t g = obj2.delta.x;
        int64_t h = obj2.start.x;

        int64_t denominator_1x = g * a - c * e;
        int64_t denominator_2x = c * e - g * a;
        if (denominator_1x == 0) {
            return false;
        }

        int64_t numerator_1x = (g * d) - (g * b) + (c * f) - (c * h);
        int64_t numerator_2x = (b * e) - (d * e) + (h * a) - (f * a);

        int64_t t1x_int;
        double t1x_frac;
        { // calc t1x
            t1x_int = numerator_1x / denominator_1x;
            int64_t fraction = numerator_1x % denominator_1x; // serves to lower the number by lots. Deltas are low, numerators are not.
            t1x_frac = static_cast<double>(fraction) / static_cast<double>(denominator_1x);
        }
        if (t1x_int < 0 || (t1x_int == 0 && t1x_frac < 0)) {
            return false;
        }

        int64_t t2x_int;
        double t2x_frac;
        { // calc t2x
            t2x_int = numerator_2x / denominator_2x;
            int64_t fraction = numerator_2x % denominator_2x;
            t2x_frac = static_cast<double>(fraction) / static_cast<double>(denominator_2x);
        }
        if (t2x_int < 0 || (t2x_int == 0 && t2x_frac < 0)) {
            return false;
        }

        // Both values are in the future, but are they on the grid?
        double col_x = static_cast<double>(f) + static_cast<double>(e * t1x_int) + (static_cast<double>(e) * t1x_frac);
        double col_y = static_cast<double>(b) + static_cast<double>(a * t1x_int) + (static_cast<double>(a) * t1x_frac);

        double xref = static_cast<double>(h) + static_cast<double>(g * t2x_int) + (static_cast<double>(g) * t2x_frac); // same value but with t2 on the other line, should equal col_x;
        double yref = static_cast<double>(d) + static_cast<double>(c * t2x_int) + (static_cast<double>(c) * t2x_frac); // same value but with t2 on the other line, should equal col_y;

        bool collides = (
                              col_x >= static_cast<double>(P1.xlow) &&
                              col_x <= static_cast<double>(P1.xhigh)
                      ) && (
                              col_y >= static_cast<double>(P1.ylow) &&
                              col_y <= static_cast<double>(P1.yhigh)
                      );

        if (collides && (col_x != xref || col_y != yref)) {
            throw std::logic_error(std::to_string(col_x) + ", " + std::to_string(xref) + ", " + std::to_string(col_y) + ", " + std::to_string(yref));
        }

        return collides;
    }

    [[nodiscard]] std::string P2MakeZ3Model () const {
        std::ostringstream Z3;

        auto DeclareConst = [&Z3](const std::string& _const){
            Z3 << "(declare-const " << _const << " Int)\n";
        };

        auto iToTimeVar = [](int i){ return "T" + std::to_string(i); };

        DeclareConst("A");
        DeclareConst("B");
        DeclareConst("C");
        DeclareConst("D");
        DeclareConst("E");
        DeclareConst("F");
        Z3 << "\n"; // just for fun.

        for (int i = 0 ;i < objects.size(); ++i) {
            DeclareConst(iToTimeVar(i));
        }
        Z3 << "\n";

        Z3 << "(assert (and\n";
        {
            auto GEQZero = [&Z3](const std::string& variable){
                Z3 << "(>= " << variable << " 0) ";
            };

//            GEQZero("A");
//            GEQZero("B");
//            GEQZero("C");
//            GEQZero("D");
//            GEQZero("E");
//            GEQZero("F");

            for (int i = 0; i < objects.size(); ++i) {
                if (i % 3 == 0) Z3 << "\n";

                GEQZero(iToTimeVar(i));
            }

            Z3 << "\n\n; Linear Equation section\n\n";

            // Assert interesection of line ati+b with general line Ati+B.
            auto AssertEq = [&Z3](
                    int64_t a,
                    int64_t b,
                    const std::string& timeVar,
                    const std::string& coefficientVar,
                    const std::string& offsetVar
            ){
                Z3 << "(= (+ " << b << " (* " << a << " "  << timeVar << ")) (+ " << offsetVar << " (* " << coefficientVar << " " << timeVar << ")))\n";
            };
            for (int i = 0; i < objects.size(); ++i) {
                Z3 << "; Object " << i << "\n";
                auto& obj = objects[i];
                // For each object there exists 3 linear equations (X,Y,Z) where they must be equal to the general line at their time Ti.
                AssertEq(obj.delta.x, obj.start.x, iToTimeVar(i), "A", "B");
                AssertEq(obj.delta.y, obj.start.y, iToTimeVar(i), "C", "D");
                AssertEq(obj.delta.z, obj.start.z, iToTimeVar(i), "E", "F");
                Z3 << "\n\n";
            }
        }
        Z3 << "\n))";
        Z3 << "\n\n(check-sat)\n(get-model)";

        return Z3.str();
    }
};

} // namespace

#undef DAY