// Main ships no test framework on purpose: plain assert / static_assert only.
// Full framework examples live on tests/gtest and tests/Boost-ext-ut branches.

// assert() disappears when NDEBUG is defined (Release, RelWithDebInfo): the
// binary would exit 0 having checked nothing, and CI would call that a pass.
// Keep the checks on in every build type.
#undef NDEBUG
#include <cassert>
#include <cmath>
#include <sstream>
#include <string>
#include <stdexcept>

#include "Example.h"      // the library under test; include/ comes from oop
#include "Ingredient.h"

static_assert(Example::limit > 0, "the invariant below assumes a positive limit");

namespace {
    bool nearlyEqual(double a, double b) {
        return std::abs(a - b) < 1e-9;
    }

    void testIngredient() {
        // g and ml: price is per 100
        const Ingredient oats{1.50, "oats", "g", 80};
        assert(nearlyEqual(oats.recipePrice(), 1.20));

        const Ingredient milk{0.80, "milk", "ml", 200};
        assert(nearlyEqual(milk.recipePrice(), 1.60));

        // pieces: price is per piece, fractional amounts allowed
        const Ingredient eggs{1.20, "eggs", "pieces", 3};
        assert(nearlyEqual(eggs.recipePrice(), 3.60));

        const Ingredient cucumber{2.50, "cucumber", "pieces", 0.5};
        assert(nearlyEqual(cucumber.recipePrice(), 1.25));

        // tbsp and tsp cost 0, whatever the price
        const Ingredient cinnamon{0, "cinnamon", "tsp", 0.5};
        assert(nearlyEqual(cinnamon.recipePrice(), 0));

        const Ingredient oil{5.00, "olive_oil", "tbsp", 1};
        assert(nearlyEqual(oil.recipePrice(), 0) && "tbsp is never priced");

        // price 0 = missing ingredient, so the cost is 0 for any unit
        const Ingredient unknown{0, "saffron", "g", 100};
        assert(nearlyEqual(unknown.recipePrice(), 0));

        // the unit text must match the CSV exactly
        const char* badUnits[] = {"gr", "G", "", "piece"};
        for (const char* bad : badUnits) {
            bool threw = false;
            try {
                const Ingredient broken{1.0, "x", bad, 1};
                (void) broken;
            } catch (const std::invalid_argument&) {
                threw = true;
            }
            assert(threw && "an unknown unit must throw");
        }
    }

    std::string printed(const Ingredient& ingredient) {
        std::ostringstream out;
        out << ingredient;
        return out.str();
    }

    void testIngredientOutput() {
        // price with 2 decimals, in lei
        assert(printed(Ingredient{1.50, "oats", "g", 80}) == "oats (80 - g) (1.20 lei)");
        assert(printed(Ingredient{1.20, "eggs", "pieces", 3}) == "eggs (3 - pieces) (3.60 lei)");

        // tbsp/tsp lines have no price
        assert(printed(Ingredient{0, "cinnamon", "tsp", 0.5}) == "cinnamon (0.5 - tsp)");
        assert(printed(Ingredient{5.00, "olive_oil", "tbsp", 1}) == "olive_oil (1 - tbsp)");

        // the 2-decimal formatting must not leak into what is printed next
        std::ostringstream out;
        out << Ingredient{1.50, "oats", "g", 80} << '\n'
            << Ingredient{2.08, "chickpeas", "g", 240} << '\n'
            << 0.125;
        assert(out.str() == "oats (80 - g) (1.20 lei)\n"
                            "chickpeas (240 - g) (4.99 lei)\n"
                            "0.125");
    }
}

// checked through the public interface only: the helper behind the invariant is
// file-local in src/Example.cpp and cannot even be named here
int main() {
    Example e;
    assert(e.value() == 0);

    e.add(7);
    assert(e.value() == 7);
    e.add(-7);
    assert(e.value() == 0);

    bool threw = false;
    try {
        e.add(-1);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "add() must refuse to go below 0");
    assert(e.value() == 0 && "a failed call leaves the object untouched");

    threw = false;
    try {
        e.add(Example::limit + 1);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "add() must refuse to go above limit");

    threw = false;
    try {
        Example broken{Example::limit + 1};
        (void) broken;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "the constructor must not hand out a broken object either");

    testIngredient();
    testIngredientOutput();

    return 0;
}
