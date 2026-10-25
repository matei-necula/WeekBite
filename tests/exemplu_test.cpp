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
#include <vector>
#include <stdexcept>

#include "Example.h"      // the library under test; include/ comes from oop
#include "Ingredient.h"
#include "Nutrition.h"
#include "Recipe.h"

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

    void testIngredientScaling() {
        Ingredient oats{1.50, "oats", "g", 80};
        oats.scale(3);
        assert(nearlyEqual(oats.recipePrice(), 3.60));
        assert(printed(oats) == "oats (240 - g) (3.60 lei)");

        Ingredient eggs{1.20, "eggs", "pieces", 2};
        eggs.scale(1);
        assert(printed(eggs) == "eggs (2 - pieces) (2.40 lei)" && "factor 1 changes nothing");

        // a refused factor must leave the ingredient untouched
        const int badFactors[] = {0, -2};
        for (const int bad : badFactors) {
            Ingredient milk{0.80, "milk", "ml", 200};
            bool threw = false;
            try {
                milk.scale(bad);
            } catch (const std::invalid_argument&) {
                threw = true;
            }
            assert(threw && "a factor <= 0 must throw");
            assert(printed(milk) == "milk (200 - ml) (1.60 lei)" && "a failed call leaves the object untouched");
        }
    }

    // oatmeal_banana, as in assets/recipes.csv and assets/recipe_ingredients.csv
    Recipe oatmealBanana() {
        const std::vector<Ingredient> ingredients{
            Ingredient{1.50, "oats", "g", 80},
            Ingredient{0.80, "milk", "ml", 200},
            Ingredient{1.17, "banana", "pieces", 1},
            Ingredient{0, "cinnamon", "tsp", 0.5},
        };
        return Recipe{"oatmeal_banana", "breakfast", ingredients, Nutrition{549, 21.5, 90.5, 13.1, 24.9}, 1};
    }

    void testRecipePrice() {
        // 1.20 (oats) + 1.60 (milk) + 1.17 (banana) + 0 (cinnamon, tsp)
        assert(nearlyEqual(oatmealBanana().recipePrice(), 3.97));

        const Recipe empty{"water", "lunch", {}, Nutrition{0, 0, 0, 0, 0}, 1};
        assert(nearlyEqual(empty.recipePrice(), 0) && "no ingredients, no cost");

        const char* badCategories[] = {"Breakfast", "snack", ""};
        for (const char* bad : badCategories) {
            bool threw = false;
            try {
                const Recipe broken{"x", bad, {}, Nutrition{0, 0, 0, 0, 0}, 1};
                (void) broken;
            } catch (const std::invalid_argument&) {
                threw = true;
            }
            assert(threw && "an unknown category must throw");
        }
    }

    std::string printed(const Recipe& recipe) {
        std::ostringstream out;
        out << recipe;
        return out.str();
    }

    void testRecipeOutput() {
        assert(printed(oatmealBanana()) ==
               "oatmeal_banana (breakfast, 1 servings)\n"
               "oats (80 - g) (1.20 lei)\n"
               "milk (200 - ml) (1.60 lei)\n"
               "banana (1 - pieces) (1.17 lei)\n"
               "cinnamon (0.5 - tsp)\n"
               "Total price: 3.97 lei\n"
               "Calories: 549kcal, Protein: 21.5g, Carbohydrates: 90.5g, Fats: 13.1g, Sugar: 24.9g\n"
               "(price may vary because of cinnamon)\n");

        // two unpriced ingredients: one note, names separated by ", "
        const std::vector<Ingredient> spiced{
            Ingredient{3.40, "chicken_breast", "g", 200},
            Ingredient{0, "salt", "tsp", 0.5},
            Ingredient{0, "paprika", "tsp", 1},
        };
        const std::string twoNotes = printed(Recipe{"spiced_chicken", "dinner", spiced, Nutrition{400, 50, 0, 10, 0}, 1});
        assert(twoNotes.find("(price may vary because of salt, paprika)\n") != std::string::npos);

        // every ingredient priced: no note at all, nutrition is the last line
        const std::vector<Ingredient> plain{Ingredient{3.40, "chicken_breast", "g", 200}};
        const std::string noNote = printed(Recipe{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 1});
        assert(noNote.find("price may vary") == std::string::npos);
        assert(noNote.substr(noNote.size() - 1) == "\n" && "the last line ends with a newline");

        // the 2-decimal total must not leak into what is printed next
        std::ostringstream out;
        out << oatmealBanana() << 240.0;
        assert(out.str().substr(out.str().size() - 4) == "\n240");
    }

    void testNutritionOutput() {
        std::ostringstream out;
        out << Nutrition{549, 21.5, 90.5, 13.1, 24.9};
        assert(out.str() == "Calories: 549kcal, Protein: 21.5g, Carbohydrates: 90.5g, Fats: 13.1g, Sugar: 24.9g");

        // values from recipes.csv with a trailing .0 print without it
        out.str("");
        out << Nutrition{372, 15.6, 55.2, 11.0, 29.1};
        assert(out.str() == "Calories: 372kcal, Protein: 15.6g, Carbohydrates: 55.2g, Fats: 11g, Sugar: 29.1g");
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
    testIngredientScaling();
    testNutritionOutput();
    testRecipePrice();
    testRecipeOutput();

    return 0;
}
