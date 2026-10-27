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
#include "Recipes.h"

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

    void testRecipeCopy() {
        const Recipe original = oatmealBanana();
        const std::string expected = printed(original);

        // copy constructor: same output, same price
        const Recipe copy{original};
        assert(printed(copy) == expected);
        assert(nearlyEqual(copy.recipePrice(), original.recipePrice()));

        // operator=: every field is replaced, the source stays as it was
        const std::vector<Ingredient> plain{Ingredient{3.40, "chicken_breast", "g", 200}};
        Recipe target{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 2};
        target = original;
        assert(printed(target) == expected);
        assert(printed(original) == expected);

        // assigning something else afterwards changes only the target
        target = Recipe{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 2};
        assert(printed(target).starts_with("plain_chicken (lunch, 2 servings)\n"));
        assert(printed(copy) == expected && "the copy is independent of the target");

        // chained assignment returns the assigned object
        Recipe first{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 1};
        Recipe second{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 1};
        first = second = original;
        assert(printed(first) == expected);
        assert(printed(second) == expected);

        // self-assignment (through a reference, so the compiler does not warn) leaves it intact
        const Recipe& same = target;
        target = same;
        assert(printed(target).starts_with("plain_chicken (lunch, 2 servings)\n"));
        assert(nearlyEqual(target.recipePrice(), 6.80));
    }

    void testRecipeScaling() {
        const Recipe original = oatmealBanana();
        const std::string before = printed(original);

        // 3 servings: every amount and price tripled, nutrition per serving unchanged
        const Recipe triple = original.scaled(3);
        assert(nearlyEqual(triple.recipePrice(), 11.91));
        assert(printed(triple) ==
               "oatmeal_banana (breakfast, 3 servings)\n"
               "oats (240 - g) (3.60 lei)\n"
               "milk (600 - ml) (4.80 lei)\n"
               "banana (3 - pieces) (3.51 lei)\n"
               "cinnamon (1.5 - tsp)\n"
               "Total price: 11.91 lei\n"
               "Calories: 549kcal, Protein: 21.5g, Carbohydrates: 90.5g, Fats: 13.1g, Sugar: 24.9g\n"
               "(price may vary because of cinnamon)\n");

        // scaled() is const: the original is untouched
        assert(printed(original) == before);

        // 1 serving gives back the same recipe
        assert(printed(original.scaled(1)) == before);

        // a refused number of servings throws and leaves the original as it was
        const int badServings[] = {0, -3};
        for (const int bad : badServings) {
            bool threw = false;
            try {
                const Recipe broken = original.scaled(bad);
                (void) broken;
            } catch (const std::invalid_argument&) {
                threw = true;
            }
            assert(threw && "servings <= 0 must throw");
            assert(printed(original) == before && "a failed call leaves the original untouched");
        }
    }

    // a small catalog in the shape main will hand over (no cinnamon price on purpose)
    Recipes smallCatalog() {
        const std::map<std::string, double> prices{
            {"oats", 1.50}, {"milk", 0.80}, {"banana", 1.17}, {"chicken_breast", 3.40}, {"eggs", 1.20},
        };
        const std::vector<RecipeRow> recipeRows{
            {"oatmeal_banana", "breakfast", Nutrition{549, 21.5, 90.5, 13.1, 24.9}},
            {"plain_chicken", "lunch", Nutrition{330, 62, 0, 7.2, 0}},
            {"boiled_eggs", "breakfast", Nutrition{210, 18, 1, 15, 1}},
            {"water", "dinner", Nutrition{0, 0, 0, 0, 0}},
        };
        const std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe{
            {"oatmeal_banana", {{"oats", 80, "g"}, {"milk", 200, "ml"}, {"banana", 1, "pieces"}, {"cinnamon", 0.5, "tsp"}}},
            {"plain_chicken", {{"chicken_breast", 200, "g"}}},
            {"boiled_eggs", {{"eggs", 3, "pieces"}}},
            // no rows for "water"; rows for a recipe that is not in recipeRows are ignored
            {"ghost_soup", {{"oats", 999, "g"}}},
        };
        return Recipes{prices, recipeRows, ingredientsByRecipe};
    }

    void testRecipesCatalog() {
        const Recipes catalog = smallCatalog();

        // every recipe lands in its own category, in file order
        const std::vector<Recipe>& breakfasts = catalog.getRecipesFromCategory(Category::breakfast);
        const std::vector<Recipe>& lunches = catalog.getRecipesFromCategory(Category::lunch);
        const std::vector<Recipe>& dinners = catalog.getRecipesFromCategory(Category::dinner);
        assert(breakfasts.size() == 2);
        assert(lunches.size() == 1);
        assert(dinners.size() == 1 && "ghost_soup is not a recipe, only an unused group of rows");

        // ingredients matched to their prices by name; cinnamon is missing → price 0 and the note
        assert(printed(breakfasts[0]) == printed(oatmealBanana()));
        assert(printed(breakfasts[1]).starts_with("boiled_eggs (breakfast, 1 servings)\n"));
        assert(nearlyEqual(breakfasts[1].recipePrice(), 3.60));
        assert(nearlyEqual(lunches[0].recipePrice(), 6.80));

        // a recipe with no ingredient rows is still in the catalog, with no ingredients
        assert(nearlyEqual(dinners[0].recipePrice(), 0));
        assert(printed(dinners[0]).starts_with("water (dinner, 1 servings)\nTotal price: 0.00 lei\n"));

        // const& to the originals: asking twice gives the same vector, not a new copy
        assert(&catalog.getRecipesFromCategory(Category::lunch) == &lunches);
    }

    std::string printed(const Recipes& catalog) {
        std::ostringstream out;
        out << catalog;
        return out.str();
    }

    void testRecipesOutput() {
        const Recipes catalog = smallCatalog();
        const std::vector<Recipe>& breakfasts = catalog.getRecipesFromCategory(Category::breakfast);
        const std::vector<Recipe>& lunches = catalog.getRecipesFromCategory(Category::lunch);
        const std::vector<Recipe>& dinners = catalog.getRecipesFromCategory(Category::dinner);

        // heading per category, then every recipe in full (Recipe's <<), each followed by a blank line
        assert(printed(catalog) ==
               "Breakfast:\n" +
               printed(breakfasts[0]) + "\n" +
               printed(breakfasts[1]) + "\n" +
               "Lunch:\n" +
               printed(lunches[0]) + "\n" +
               "Dinner:\n" +
               printed(dinners[0]) + "\n");

        // an empty catalog still prints the 3 headings
        const Recipes empty{{}, {}, {}};
        assert(printed(empty) == "Breakfast:\nLunch:\nDinner:\n");

        // the 2-decimal prices must not leak into what is printed next
        std::ostringstream out;
        out << catalog << 0.125;
        assert(out.str().ends_with("\n0.125"));
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
    testRecipeCopy();
    testRecipeScaling();
    testRecipesCatalog();
    testRecipesOutput();

    return 0;
}
