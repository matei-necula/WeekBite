// Main ships no test framework on purpose: plain assert / static_assert only.
// Full framework examples live on tests/gtest and tests/Boost-ext-ut branches.

// assert() disappears when NDEBUG is defined (Release, RelWithDebInfo): the
// binary would exit 0 having checked nothing, and CI would call that a pass.
// Keep the checks on in every build type.
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <fstream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <utility>

// the library under test; include/ comes from oop
#include "DayMeals.h"
#include "Ingredient.h"
#include "MealPlan.h"
#include "Nutrition.h"
#include "Recipe.h"
#include "Recipes.h"
#include "ShoppingList.h"
#include "UserChoices.h"
#include "Errors.h"
#include "PlanConstraint.h"
#include "MealBalanceConstraint.h"
#include "NutritionTargetConstraint.h"
#include "WeekdayPrepTimeConstraint.h"
#include "DietRestrictionConstraint.h"
#include "textHelper.h"

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
            } catch (const InputError&) {
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
            } catch (const AmountError&) {
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
        return Recipe{"oatmeal_banana", "breakfast", ingredients, Nutrition{549, 21.5, 90.5, 13.1, 24.9}, 1, 10};
    }

    void testRecipePrice() {
        // 1.20 (oats) + 1.60 (milk) + 1.17 (banana) + 0 (cinnamon, tsp)
        assert(nearlyEqual(oatmealBanana().recipePrice(), 3.97));

        const Recipe empty{"water", "lunch", {}, Nutrition{0, 0, 0, 0, 0}, 1, 10};
        assert(nearlyEqual(empty.recipePrice(), 0) && "no ingredients, no cost");

        const char* badCategories[] = {"Breakfast", "snack", ""};
        for (const char* bad : badCategories) {
            bool threw = false;
            try {
                const Recipe broken{"x", bad, {}, Nutrition{0, 0, 0, 0, 0}, 1, 10};
                (void) broken;
            } catch (const InputError&) {
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
        const std::string twoNotes = printed(Recipe{"spiced_chicken", "dinner", spiced, Nutrition{400, 50, 0, 10, 0}, 1, 10});
        assert(twoNotes.find("(price may vary because of salt, paprika)\n") != std::string::npos);

        // every ingredient priced: no note at all, nutrition is the last line
        const std::vector<Ingredient> plain{Ingredient{3.40, "chicken_breast", "g", 200}};
        const std::string noNote = printed(Recipe{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 1, 10});
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
        Recipe target{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 2, 10};
        target = original;
        assert(printed(target) == expected);
        assert(printed(original) == expected);

        // assigning something else afterwards changes only the target
        target = Recipe{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 2, 10};
        assert(printed(target).starts_with("plain_chicken (lunch, 2 servings)\n"));
        assert(printed(copy) == expected && "the copy is independent of the target");

        // chained assignment returns the assigned object
        Recipe first{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 1, 10};
        Recipe second{"plain_chicken", "lunch", plain, Nutrition{330, 62, 0, 7.2, 0}, 1, 10};
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
            } catch (const AmountError&) {
                threw = true;
            }
            assert(threw && "servings <= 0 must throw");
            assert(printed(original) == before && "a failed call leaves the original untouched");
        }
    }

    // a small catalog in the shape main will hand over (no cinnamon price on purpose)
    Recipes smallCatalog() {
        const std::map<std::string, IngredientInfo> prices{
            {"oats", {1.50, {}, {}}}, {"milk", {0.80, {}, {}}}, {"banana", {1.17, {}, {}}},
            {"chicken_breast", {3.40, {}, {}}}, {"eggs", {1.20, {}, {}}},
        };
        const std::vector<RecipeRow> recipeRows{
            {"oatmeal_banana", "breakfast", Nutrition{549, 21.5, 90.5, 13.1, 24.9}, 10},
            {"plain_chicken", "lunch", Nutrition{330, 62, 0, 7.2, 0}, 10},
            {"boiled_eggs", "breakfast", Nutrition{210, 18, 1, 15, 1}, 10},
            {"water", "dinner", Nutrition{0, 0, 0, 0, 0}, 10},
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

    void testNutritionSum() {
        Nutrition day{549, 21.5, 90.5, 13.1, 24.9};
        const Nutrition lunch{372, 15.6, 55.2, 11.0, 29.1};

        // += returns the left object itself, so it can be chained
        const Nutrition& result = (day += lunch);
        assert(&result == &day);

        std::ostringstream out;
        out << day;
        assert(out.str() == "Calories: 921kcal, Protein: 37.1g, Carbohydrates: 145.7g, Fats: 24.1g, Sugar: 54g");

        // the right side stays unchanged
        out.str("");
        out << lunch;
        assert(out.str() == "Calories: 372kcal, Protein: 15.6g, Carbohydrates: 55.2g, Fats: 11g, Sugar: 29.1g");

        // three meals added into a zero total, the way a day adds them up
        Nutrition total{0, 0, 0, 0, 0};
        total += lunch;
        total += lunch;
        total += lunch;
        out.str("");
        out << total;
        assert(out.str() == "Calories: 1116kcal, Protein: 46.8g, Carbohydrates: 165.6g, Fats: 33g, Sugar: 87.3g");
    }

    Recipe plainChicken() {
        return Recipe{"plain_chicken", "lunch", {Ingredient{3.40, "chicken_breast", "g", 200}}, Nutrition{330, 62, 0, 7.2, 0}, 1, 10};
    }

    Recipe spicedChicken() {
        const std::vector<Ingredient> spiced{
            Ingredient{3.40, "chicken_breast", "g", 200},
            Ingredient{0, "salt", "tsp", 0.5},
        };
        return Recipe{"spiced_chicken", "dinner", spiced, Nutrition{400, 50, 0, 10, 0}, 1, 10};
    }

    std::string printed(const DayMeals& day) {
        std::ostringstream out;
        out << day;
        return out.str();
    }

    void testDayMeals() {
        const DayMeals day{oatmealBanana(), plainChicken(), spicedChicken()};

        // 3.97 + 6.80 + 6.80
        assert(nearlyEqual(day.dayPrice(), 17.57));

        std::ostringstream out;
        out << day.dayNutrition();
        assert(out.str() == "Calories: 1279kcal, Protein: 133.5g, Carbohydrates: 90.5g, Fats: 30.3g, Sugar: 24.9g");

        // getNutrition hands out the recipe's own values
        out.str("");
        out << oatmealBanana().getNutrition();
        assert(out.str() == "Calories: 549kcal, Protein: 21.5g, Carbohydrates: 90.5g, Fats: 13.1g, Sugar: 24.9g");

        // the 3 recipes in full, a blank line after each, then the day totals
        assert(printed(day) ==
               printed(oatmealBanana()) + "\n" +
               printed(plainChicken()) + "\n" +
               printed(spicedChicken()) + "\n" +
               "Day price: 17.57 lei\n"
               "Total nutrition: Calories: 1279kcal, Protein: 133.5g, Carbohydrates: 90.5g, Fats: 30.3g, Sugar: 24.9g\n");

        // the day keeps copies: changing the recipe it was built from does not change the day
        Recipe lunch = plainChicken();
        const DayMeals copied{oatmealBanana(), lunch, spicedChicken()};
        const std::string before = printed(copied);
        lunch = lunch.scaled(4);
        assert(printed(copied) == before);

        // a scaled recipe in the day: price goes up, nutrition per serving stays
        const DayMeals scaledDay{oatmealBanana(), plainChicken().scaled(2), spicedChicken()};
        assert(nearlyEqual(scaledDay.dayPrice(), 3.97 + 13.60 + 6.80));
        out.str("");
        out << scaledDay.dayNutrition();
        assert(out.str() == "Calories: 1279kcal, Protein: 133.5g, Carbohydrates: 90.5g, Fats: 30.3g, Sugar: 24.9g");

        // the 2-decimal price must not leak into what is printed next
        out.str("");
        out << day << 240.0;
        assert(out.str().substr(out.str().size() - 4) == "\n240");
    }

    void testDayMealsScaling() {
        const DayMeals day{oatmealBanana(), plainChicken(), spicedChicken()};
        const std::string before = printed(day);

        // every recipe in the day gets the same servings: price doubled, nutrition per serving unchanged
        const DayMeals doubled = day.scaled(2);
        assert(nearlyEqual(doubled.dayPrice(), 2 * 17.57));
        const std::string expected =
            printed(oatmealBanana().scaled(2)) + "\n" +
            printed(plainChicken().scaled(2)) + "\n" +
            printed(spicedChicken().scaled(2)) + "\n" +
            "Day price: 35.14 lei\n"
            "Total nutrition: Calories: 1279kcal, Protein: 133.5g, Carbohydrates: 90.5g, Fats: 30.3g, Sugar: 24.9g\n";
        assert(printed(doubled) == expected);

        // scaled() is const: the day it was called on is untouched
        assert(printed(day) == before);

        const int badServings[] = {0, -2};
        for (const int bad : badServings) {
            bool threw = false;
            try {
                const DayMeals broken = day.scaled(bad);
                (void) broken;
            } catch (const AmountError&) {
                threw = true;
            }
            assert(threw && "servings <= 0 must throw");
            assert(printed(day) == before && "a failed call leaves the day untouched");
        }
    }

    void testGetters() {
        const Nutrition nutrition{549, 21.5, 90.5, 13.1, 24.9};
        assert(nearlyEqual(nutrition.getCalories(), 549));
        assert(oatmealBanana().getName() == "oatmeal_banana");
        // the greedy reads calories through the recipe
        assert(nearlyEqual(plainChicken().getNutrition().getCalories(), 330));
    }

    // one row per recipe: name, category, kcal, price; each recipe gets a single ingredient
    // "<name>_ing" (1 piece), so the recipe price is exactly the given price
    using PlanRow = std::tuple<std::string, std::string, double, double>;

    Recipes catalogOf(const std::vector<PlanRow>& rows) {
        std::map<std::string, IngredientInfo> prices;
        std::vector<RecipeRow> recipeRows;
        std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe;
        for (const auto& [name, category, kcal, price] : rows) {
            prices[name + "_ing"] = {price, {}, {}};
            recipeRows.emplace_back(name, category, Nutrition{kcal, 0, 0, 0, 0}, 10);
            ingredientsByRecipe[name] = {{name + "_ing", 1, "pieces"}};
        }
        return Recipes{prices, recipeRows, ingredientsByRecipe};
    }

    // 7 breakfasts, lunches and dinners at 1 leu each; lunch kcal 700, 710, ..., 760
    std::vector<PlanRow> weekRows() {
        std::vector<PlanRow> rows;
        for (int i = 1; i <= 7; i++) {
            rows.emplace_back("b" + std::to_string(i), "breakfast", 400 + 10 * i, 1);
            rows.emplace_back("l" + std::to_string(i), "lunch", 690 + 10 * i, 1);
            rows.emplace_back("d" + std::to_string(i), "dinner", 450 + 10 * i, 1);
        }
        return rows;
    }

    std::string printed(const MealPlan& plan) {
        std::ostringstream out;
        out << plan;
        return out.str();
    }

    // how many times a recipe is served: its header "name (" (ingredient lines read "name_ing (")
    int timesServed(const std::string& output, const std::string& name) {
        int count = 0;
        for (std::size_t at = output.find(name + " ("); at != std::string::npos; at = output.find(name + " (", at + 1)) {
            count++;
        }
        return count;
    }

    // the text of one day: from "Day n" up to the next day (or the week total)
    std::string dayText(const std::string& output, int n) {
        const std::size_t start = output.find("Day " + std::to_string(n) + "\n");
        const std::size_t next = output.find(n < 7 ? "Day " + std::to_string(n + 1) + "\n" : "Total money spent:");
        return output.substr(start, next - start);
    }

    // the printed plan has Day 1 .. Day 7 and no Day 8 (operator<< is the only way to see the days)
    bool hasSevenDays(const MealPlan& plan) {
        const std::string output = printed(plan);
        for (int day = 1; day <= 7; day++) {
            if (output.find("Day " + std::to_string(day) + "\n") == std::string::npos) {
                return false;
            }
        }
        return output.find("Day 8\n") == std::string::npos;
    }

    const std::string budgetReason = "Plan impossible error: The budget is too small for 7 days";
    const std::string restrictionsReason = "Plan impossible error: The restrictions leave too few recipes";

    // the what() of the PlanImpossibleError the constructor throws; "" when the plan was built
    std::string planFailure(double budget, double kcal, const Recipes& catalog,
                            std::vector<std::unique_ptr<PlanConstraint>> constraints = {}) {
        try {
            const MealPlan plan{budget, kcal, catalog, std::move(constraints)};
            (void) plan;
        } catch (const PlanImpossibleError& e) {
            return e.what();
        }
        return "";
    }

    void testMealPlanWeek() {
        const Recipes catalog = catalogOf(weekRows());
        // 70 lei / 7 = 10 lei a day; every day costs 3
        const MealPlan plan{70, 2000, catalog};
        assert(hasSevenDays(plan));

        const std::string output = printed(plan);
        assert(output.starts_with("Day 1\n"));
        assert(output.find("Day 7\n") != std::string::npos);
        assert(output.find("Day 8") == std::string::npos);
        // each day ends with a blank line, then the week total
        assert(output.find("\n\nDay 2\n") != std::string::npos);
        assert(output.ends_with("\n\nTotal money spent: 21.00 lei\n"));

        // every recipe served exactly once across the week
        for (int i = 1; i <= 7; i++) {
            const std::string number = std::to_string(i);
            assert(timesServed(output, "b" + number) == 1);
            assert(timesServed(output, "l" + number) == 1);
            assert(timesServed(output, "d" + number) == 1);
        }

        // lunch closest to 40% of 2000 = 800 kcal first: l7 (760 kcal) on day 1, l1 (700 kcal) on day 7
        assert(timesServed(dayText(output, 1), "l7") == 1);
        assert(timesServed(dayText(output, 7), "l1") == 1);

        // the catalog is only read: still 7 recipes per category
        assert(catalog.getRecipesFromCategory(Category::breakfast).size() == 7);
        assert(catalog.getRecipesFromCategory(Category::lunch).size() == 7);
        assert(catalog.getRecipesFromCategory(Category::dinner).size() == 7);

        // the 2-decimal total must not leak into what is printed next
        std::ostringstream out;
        out << plan << 0.125;
        assert(out.str().ends_with("\n0.125"));
    }

    void testMealPlanClosestCalories() {
        // day 1: lunch l7 = 760 kcal, target 2000 → half = 620
        // breakfast "near" (630, 10 away) beats b7 (470) and "far" (700, 80 away)
        std::vector<PlanRow> rows = weekRows();
        rows.emplace_back("near", "breakfast", 630, 1);
        rows.emplace_back("far", "breakfast", 700, 1);
        const MealPlan plan{70, 2000, catalogOf(rows)};
        assert(hasSevenDays(plan));
        const std::string output = printed(plan);
        assert(timesServed(dayText(output, 1), "near") == 1);
        // closest is closest even when it is far from half: no breakfast is left out for being far
        assert(timesServed(output, "far") == 1);
    }

    void testMealPlanLunchOrder() {
        // 40% of 2000 = 800 kcal: the closest lunch goes first, above or below 800
        std::vector<PlanRow> rows = weekRows();
        rows.emplace_back("exact", "lunch", 800, 1);
        rows.emplace_back("above", "lunch", 805, 1);
        // the most kcal, but the farthest from 800 → never reached in 7 days
        rows.emplace_back("huge", "lunch", 1600, 1);
        const MealPlan plan{70, 2000, catalogOf(rows)};
        assert(hasSevenDays(plan));
        const std::string output = printed(plan);
        assert(timesServed(dayText(output, 1), "exact") == 1);
        assert(timesServed(dayText(output, 2), "above") == 1);
        assert(timesServed(dayText(output, 3), "l7") == 1);
        assert(timesServed(output, "huge") == 0);
        // 10 lunches for 7 days: the two farthest from 800 (l1, l2) are left out too
        assert(timesServed(output, "l1") == 0);
        assert(timesServed(output, "l2") == 0);

        // a low target turns the order around: 40% of 1300 = 520, so l1 (700) first and l7 (760) last
        const MealPlan low{70, 1300, catalogOf(weekRows())};
        assert(hasSevenDays(low));
        const std::string lowOutput = printed(low);
        assert(timesServed(dayText(lowOutput, 1), "l1") == 1);
        assert(timesServed(dayText(lowOutput, 7), "l7") == 1);
    }

    void testMealPlanBudget() {
        // 10 lei a day
        std::vector<PlanRow> rows = weekRows();
        // exactly 40% of the target, so tried first, but costs more than the whole day → skipped, the next lunch is tried
        rows.emplace_back("lobster", "lunch", 800, 11);
        // tried second; fits the day alone, but leaves (10 - 9.5) / 2 = 0.25 lei per meal: no breakfast fits → next lunch
        rows.emplace_back("steak", "lunch", 795, 9.5);
        // the closest breakfast to half, but over (10 - 1) / 2 = 4.5 lei → the next closest is taken
        rows.emplace_back("caviar", "breakfast", 620, 5);
        const MealPlan plan{70, 2000, catalogOf(rows)};
        assert(hasSevenDays(plan));
        const std::string output = printed(plan);
        assert(timesServed(output, "lobster") == 0);
        assert(timesServed(output, "steak") == 0);
        assert(timesServed(output, "caviar") == 0);
        assert(timesServed(dayText(output, 1), "l7") == 1);
    }

    void testMealPlanFails() {
        // 7 lei a week = 1 leu a day: the lunch fits, nothing is left for breakfast → no day at all
        const std::string poor = planFailure(7, 2000, catalogOf(weekRows()));
        assert(poor == budgetReason);

        // only 6 lunches: recipes are not repeated, so day 7 has no lunch (no constraints → the budget is blamed)
        std::vector<PlanRow> rows;
        for (const PlanRow& row : weekRows()) {
            if (std::get<0>(row) != "l1") {
                rows.push_back(row);
            }
        }
        const std::string sixLunches = planFailure(70, 2000, catalogOf(rows));
        assert(sixLunches == budgetReason);

        // an empty catalog cannot fill a single day
        const Recipes empty{{}, {}, {}};
        const std::string nothing = planFailure(70, 2000, empty);
        assert(nothing == budgetReason);

        // a plan that works throws nothing
        const std::string works = planFailure(70, 2000, catalogOf(weekRows()));
        assert(works.empty());
    }
    // how many times a piece of text appears in the output
    int countOf(const std::string& output, const std::string& piece) {
        int count = 0;
        for (std::size_t at = output.find(piece); at != std::string::npos; at = output.find(piece, at + 1)) {
            count++;
        }
        return count;
    }

    void testMealPlanScaling() {
        const MealPlan plan{70, 2000, catalogOf(weekRows())};
        const std::string before = printed(plan);

        // the greedy does not run again: same 7 days, same recipes in the same days, 3 servings each
        const MealPlan triple = plan.scaled(3);
        assert(hasSevenDays(triple));
        const std::string output = printed(triple);
        assert(countOf(output, ", 3 servings)\n") == 21);
        assert(countOf(output, ", 1 servings)\n") == 0);
        assert(timesServed(dayText(output, 1), "l7") == 1);
        assert(timesServed(dayText(output, 7), "l1") == 1);
        // 3 lei a day before, 9 after
        assert(countOf(output, "Day price: 9.00 lei\n") == 7);
        assert(output.ends_with("\n\nTotal money spent: 63.00 lei\n"));

        // scaled() is const: the plan it was called on is untouched
        assert(printed(plan) == before);
        // 1 serving gives back the same plan
        const MealPlan same = plan.scaled(1);
        assert(printed(same) == before);

        const int badServings[] = {0, -1};
        for (const int bad : badServings) {
            bool threw = false;
            try {
                const MealPlan broken = plan.scaled(bad);
                (void) broken;
            } catch (const AmountError&) {
                threw = true;
            }
            assert(threw && "servings <= 0 must throw");
            assert(printed(plan) == before && "a failed call leaves the plan untouched");
        }
    }

    void testRecipeQuestions() {
        const Recipe oatmeal = oatmealBanana();

        assert(oatmeal.questionCategory(Category::breakfast));
        assert(!oatmeal.questionCategory(Category::lunch));

        // partial match on the name; Recipe expects lowercase text (UserChoices lowercases it)
        assert(oatmeal.questionName("banana"));
        assert(oatmeal.questionName("oatmeal_banana"));
        assert(!oatmeal.questionName("chicken"));
        assert(!oatmeal.questionName("BANANA"));

        // any ingredient may match, partially; an unpriced one (cinnamon) still counts
        assert(oatmeal.questionIngredient("milk"));
        assert(oatmeal.questionIngredient("cinna"));
        assert(!oatmeal.questionIngredient("egg"));

        // a recipe with no ingredients matches no ingredient text
        const Recipes catalog = smallCatalog();
        const Recipe& water = catalog.getRecipesFromCategory(Category::dinner)[0];
        assert(!water.questionIngredient("o"));
    }

    void testPrintShort() {
        std::ostringstream out;
        oatmealBanana().printShort(out);
        out << 3.14159;
        // one line: name, kcal, price with 2 decimals; the stream's precision is restored afterwards
        assert(out.str() == "Name: oatmeal_banana, kcal: 549.00, price: 3.97 lei\n3.14159");
    }

    bool throwsInputError(bool namePicked, bool ingredientPicked, const std::string& text) {
        try {
            const UserChoices choices{false, namePicked, ingredientPicked, Category::breakfast, text, text,
                                      SortChoice::price};
            (void) choices;
        } catch (const InputError&) {
            return true;
        }
        return false;
    }

    void testUserChoices() {
        const Recipe oatmeal = oatmealBanana();
        const Recipe chicken = plainChicken();

        // the user's text is lowercased once, so "Banana" finds oatmeal_banana
        const UserChoices byName{false, true, false, Category::breakfast, "Banana", "", SortChoice::price};
        assert(byName.matches(oatmeal));
        assert(!byName.matches(chicken));

        const UserChoices byIngredient{false, false, true, Category::breakfast, "", "MILK", SortChoice::price};
        assert(byIngredient.matches(oatmeal));
        assert(!byIngredient.matches(chicken));

        // every picked criterion must match (AND)
        const UserChoices lunchWithOats{true, false, true, Category::lunch, "", "oats", SortChoice::price};
        assert(!lunchWithOats.matches(oatmeal) && "has oats, but is a breakfast");
        assert(!lunchWithOats.matches(chicken) && "is a lunch, but has no oats");
        const UserChoices breakfastWithOats{true, false, true, Category::breakfast, "", "oats", SortChoice::price};
        assert(breakfastWithOats.matches(oatmeal));

        // a criterion that is not picked is ignored, even if its value would not match
        const UserChoices nothingPicked{false, false, false, Category::dinner, "zzz", "zzz", SortChoice::price};
        assert(nothingPicked.matches(oatmeal));
        assert(nothingPicked.matches(chicken));

        // an empty text is rejected only for a picked criterion
        assert(throwsInputError(true, false, ""));
        assert(throwsInputError(false, true, ""));
        assert(!throwsInputError(false, false, ""));

        // ascending by the user's sort choice; equal recipes do not come before each other
        // (oatmeal: 549 kcal, 3.97 lei; chicken: 330 kcal, 6.80 lei)
        const UserChoices byCalories{false, false, false, Category::breakfast, "", "", SortChoice::calories};
        assert(byCalories.comesBefore(chicken, oatmeal));
        assert(!byCalories.comesBefore(oatmeal, chicken));
        assert(!byCalories.comesBefore(oatmeal, oatmeal));
        assert(nothingPicked.comesBefore(oatmeal, chicken));
        assert(!nothingPicked.comesBefore(chicken, oatmeal));
        assert(!nothingPicked.comesBefore(oatmeal, oatmeal));
    }

    std::string namesOf(const std::vector<Recipe>& recipes) {
        std::string names;
        for (const Recipe& recipe : recipes) {
            names += recipe.getName() + ";";
        }
        return names;
    }

    void testSearch() {
        const Recipes catalog = smallCatalog();
        // water 0 kcal 0 lei, boiled_eggs 210 kcal 3.60 lei, plain_chicken 330 kcal 6.80 lei,
        // oatmeal_banana 549 kcal 3.97 lei

        const UserChoices allByPrice{false, false, false, Category::breakfast, "", "", SortChoice::price};
        const std::string byPrice = namesOf(catalog.searchUserChoices(allByPrice));
        assert(byPrice == "water;boiled_eggs;oatmeal_banana;plain_chicken;");

        const UserChoices allByCalories{false, false, false, Category::breakfast, "", "", SortChoice::calories};
        const std::string byCalories = namesOf(catalog.searchUserChoices(allByCalories));
        assert(byCalories == "water;boiled_eggs;plain_chicken;oatmeal_banana;");

        // only the matching recipes, from every category
        const UserChoices withE{false, false, true, Category::breakfast, "", "e", SortChoice::price};
        const std::string matchingE = namesOf(catalog.searchUserChoices(withE));
        assert(matchingE == "boiled_eggs;plain_chicken;" && "eggs and chicken_breast contain 'e'");

        // nothing matches → empty result
        const UserChoices noMatch{false, true, false, Category::breakfast, "pizza", "", SortChoice::price};
        const std::vector<Recipe> none = catalog.searchUserChoices(noMatch);
        assert(none.empty());

        // the results are copies: the catalog's own recipes stay where they are
        const std::vector<Recipe> breakfasts = catalog.searchUserChoices(
            UserChoices{true, false, false, Category::breakfast, "", "", SortChoice::price});
        const std::vector<Recipe>& originals = catalog.getRecipesFromCategory(Category::breakfast);
        assert(breakfasts.size() == 2);
        assert(&breakfasts[0] != &originals[0] && &breakfasts[0] != &originals[1]);
    }

    void testSearchKeepsOrderOnTies() {
        // two lunches with the same price: stable sort keeps them in catalog order
        const std::map<std::string, IngredientInfo> prices{{"rice", {1.00, {}, {}}}};
        const std::vector<RecipeRow> recipeRows{
            {"rice_one", "lunch", Nutrition{500, 10, 100, 1, 0}, 10},
            {"rice_two", "lunch", Nutrition{400, 10, 80, 1, 0}, 10},
        };
        const std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe{
            {"rice_one", {{"rice", 100, "g"}}},
            {"rice_two", {{"rice", 100, "g"}}},
        };
        const Recipes catalog{prices, recipeRows, ingredientsByRecipe};
        const UserChoices byPrice{false, true, false, Category::lunch, "rice", "", SortChoice::price};
        const std::string names = namesOf(catalog.searchUserChoices(byPrice));
        assert(names == "rice_one;rice_two;");
    }

    std::string readerResult(const std::string& input) {
        try {
            const auto [category, name, ingredient] = reader(input);
            return std::string(category ? "c" : "-") + (name ? "n" : "-") + (ingredient ? "i" : "-");
        } catch (const InputError&) {
            return "throws";
        }
    }

    void testReader() {
        const std::string twoCriteria = readerResult("category,name");
        const std::string oneCriterion = readerResult("ingredient");
        const std::string allCriteria = readerResult("name,ingredient,category");
        const std::string repeated = readerResult("name,name");
        const std::string misspelled = readerResult("categry");
        const std::string withSpace = readerResult("category, name");
        const std::string empty = readerResult("");
        const std::string onlyComma = readerResult(",");

        assert(twoCriteria == "cn-");
        assert(oneCriterion == "--i");
        assert(allCriteria == "cni");
        assert(repeated == "-n-");
        assert(misspelled == "throws");
        assert(withSpace == "throws" && "no spaces allowed");
        assert(empty == "throws");
        assert(onlyComma == "throws");
    }

    // which Recipe a stats function returned, by name
    std::string nameOf(const Recipe& recipe) {
        return recipe.getName();
    }

    // true when the call throws Error (or a class derived from it)
    template <typename Error, typename Call>
    bool throwsError(Call call) {
        try {
            call();
        } catch (const Error&) {
            return true;
        }
        return false;
    }

    void testCategoryToText() {
        assert(categoryToText(Category::breakfast) == "breakfast");
        assert(categoryToText(Category::lunch) == "lunch");
        assert(categoryToText(Category::dinner) == "dinner");
    }

    void testMealPlanStats() {
        // b/d cost 1 leu each; lunch prices differ. Lunch closest to 800 kcal first,
        // so day 1 gets l7, day 2 l6, ..., day 7 l1
        std::vector<PlanRow> rows = weekRows();
        const double lunchPrices[] = {5, 2, 9, 2, 4, 7, 3}; // l1..l7
        for (PlanRow& row : rows) {
            const std::string& name = std::get<0>(row);
            if (name.starts_with("l")) {
                std::get<3>(row) = lunchPrices[name[1] - '1'];
            }
        }
        // day prices: 5, 9, 6, 4, 11, 4, 7 = 46 lei
        const MealPlan plan{700, 2000, catalogOf(rows)};
        assert(hasSevenDays(plan));

        const double left = plan.budgetLeft();
        assert(nearlyEqual(left, 654));

        // all 21 recipes are served: 3080 + 5110 + 3430 = 11620 kcal
        std::ostringstream out;
        out << plan.weekNutritionTotal();
        assert(out.str() == "Calories: 11620kcal, Protein: 0g, Carbohydrates: 0g, Fats: 0g, Sugar: 0g");
        const double average = plan.averageKcalPerDay();
        assert(nearlyEqual(average, 11620.0 / 7));

        // day numbers count from 1; day 4 and day 6 both cost 4 → the first one
        const int cheapest = plan.cheapestDay();
        const int mostExpensive = plan.mostExpensiveDay();
        assert(cheapest == 4);
        assert(mostExpensive == 5);
    }

    void testMealPlanStatsTies() {
        // every day costs 3 lei: the first day wins both ways
        const MealPlan plan{70, 2000, catalogOf(weekRows())};
        const int cheapest = plan.cheapestDay();
        const int mostExpensive = plan.mostExpensiveDay();
        assert(cheapest == 1);
        assert(mostExpensive == 1);
        const double left = plan.budgetLeft();
        assert(nearlyEqual(left, 70 - 21));
    }

    void testRecipesStats() {
        // breakfasts: oatmeal_banana 3.97 lei / 549 kcal, boiled_eggs 3.60 lei / 210 kcal
        const Recipes catalog = smallCatalog();
        const double breakfastPrice = catalog.averagePrice(Category::breakfast);
        const double breakfastKcal = catalog.averageKcal(Category::breakfast);
        assert(nearlyEqual(breakfastPrice, (3.97 + 3.60) / 2));
        assert(nearlyEqual(breakfastKcal, (549.0 + 210.0) / 2));

        const std::string cheapest = nameOf(catalog.cheapestRecipe(Category::breakfast));
        const std::string mostExpensive = nameOf(catalog.mostExpensiveRecipe(Category::breakfast));
        assert(cheapest == "boiled_eggs");
        assert(mostExpensive == "oatmeal_banana");

        // a category with one recipe: it is the average, the cheapest and the most expensive
        const double lunchPrice = catalog.averagePrice(Category::lunch);
        const double lunchKcal = catalog.averageKcal(Category::lunch);
        const std::string onlyLunchLow = nameOf(catalog.cheapestRecipe(Category::lunch));
        const std::string onlyLunchHigh = nameOf(catalog.mostExpensiveRecipe(Category::lunch));
        assert(nearlyEqual(lunchPrice, 6.80));
        assert(nearlyEqual(lunchKcal, 330));
        assert(onlyLunchLow == "plain_chicken");
        assert(onlyLunchHigh == "plain_chicken");

        // the result is a copy: scaling it leaves the catalog alone
        const Recipe copy = catalog.cheapestRecipe(Category::breakfast).scaled(3);
        const double catalogPrice = catalog.getRecipesFromCategory(Category::breakfast)[1].recipePrice();
        assert(nearlyEqual(copy.recipePrice(), 3 * 3.60));
        assert(nearlyEqual(catalogPrice, 3.60));
    }

    void testRecipesStatsTies() {
        // same price: the first one in the catalog wins both ways
        const Recipes catalog = catalogOf({{"first", "dinner", 300.5, 2}, {"second", "dinner", 500, 2}});
        const std::string cheapest = nameOf(catalog.cheapestRecipe(Category::dinner));
        const std::string mostExpensive = nameOf(catalog.mostExpensiveRecipe(Category::dinner));
        const double average = catalog.averageKcal(Category::dinner);
        assert(cheapest == "first");
        assert(mostExpensive == "first");
        assert(nearlyEqual(average, 400.25) && "decimals kept: the sum starts at 0.0");
    }

    void testRecipesStatsEmptyCategory() {
        const Recipes empty{{}, {}, {}};
        const bool price = throwsError<EmptyCategoryError>([&empty] { (void) empty.averagePrice(Category::lunch); });
        const bool kcal = throwsError<EmptyCategoryError>([&empty] { (void) empty.averageKcal(Category::lunch); });
        const bool cheapest = throwsError<EmptyCategoryError>([&empty] { (void) empty.cheapestRecipe(Category::lunch); });
        const bool mostExpensive = throwsError<EmptyCategoryError>([&empty] { (void) empty.mostExpensiveRecipe(Category::lunch); });
        assert(price);
        assert(kcal);
        assert(cheapest);
        assert(mostExpensive);
    }

    void testIngredientAdd() {
        Ingredient oats{1.50, "oats", "g", 80};
        oats += Ingredient{1.50, "oats", "g", 50};
        assert(printed(oats) == "oats (130 - g) (1.95 lei)");

        // += hands back the ingredient itself
        const Ingredient& same = (oats += Ingredient{1.50, "oats", "g", 20});
        assert(&same == &oats);
        assert(printed(oats) == "oats (150 - g) (2.25 lei)");

        // another name or another unit is refused, and the ingredient stays as it was
        const bool otherName = throwsError<UnitError>([&oats] { oats += Ingredient{0.80, "milk", "g", 100}; });
        const bool otherUnit = throwsError<UnitError>([&oats] { oats += Ingredient{1.50, "oats", "pieces", 1}; });
        assert(otherName);
        assert(otherUnit);
        assert(printed(oats) == "oats (150 - g) (2.25 lei)");
    }

    // the names of some ingredients, in order, separated by spaces
    std::string ingredientNames(const std::vector<Ingredient>& ingredients) {
        std::string names;
        for (const Ingredient& ingredient : ingredients) {
            names += (names.empty() ? "" : " ") + ingredient.getName();
        }
        return names;
    }

    void testGiveIngredients() {
        // a recipe: its own list, in order
        const Recipe oatmeal = oatmealBanana();
        assert(ingredientNames(oatmeal.giveIngredients()) == "oats milk banana cinnamon");

        // a day: breakfast, then lunch, then dinner, nothing merged yet
        const DayMeals day{oatmealBanana(), plainChicken(), spicedChicken()};
        const std::vector<Ingredient> dayList = day.giveIngredients();
        assert(ingredientNames(dayList) == "oats milk banana cinnamon chicken_breast chicken_breast salt");

        // a week: all 7 days, day 1 first (each catalogOf recipe has one ingredient "<name>_ing")
        const MealPlan plan{70, 2000, catalogOf(weekRows())};
        const std::vector<Ingredient> week = plan.giveIngredients();
        assert(week.size() == 21);
        const std::vector<Ingredient> dayOne(week.begin(), week.begin() + 3);
        assert(ingredientNames(dayOne) == "b7_ing l7_ing d7_ing");
        const std::vector<Ingredient> dayTwo(week.begin() + 3, week.begin() + 6);
        assert(ingredientNames(dayTwo) == "b6_ing l6_ing d6_ing");
    }

    std::string printed(const ShoppingList& list) {
        std::ostringstream out;
        out << list;
        return out.str();
    }

    void testShoppingListMerge() {
        // the same name → one line with the amounts added up, in the order first seen
        const std::vector<Ingredient> all{
            Ingredient{1.50, "oats", "g", 80},
            Ingredient{0.80, "milk", "ml", 200},
            Ingredient{1.50, "oats", "g", 50},
            Ingredient{0, "cinnamon", "tsp", 0.5},
            Ingredient{0.80, "milk", "ml", 100},
            Ingredient{0, "cinnamon", "tsp", 1},
        };
        const ShoppingList list{all};
        // tbsp/tsp have no price, so they add nothing to the total
        assert(printed(list) == "oats (130 - g) (1.95 lei)\n"
                                "milk (300 - ml) (2.40 lei)\n"
                                "cinnamon (1.5 - tsp)\n"
                                "Total: 4.35 lei\n");

        // only the total gets 2 decimals; the stream is given back as it was
        std::ostringstream out;
        out << list << 1234.5678;
        assert(out.str().ends_with("Total: 4.35 lei\n1234.57"));

        const ShoppingList empty{std::vector<Ingredient>{}};
        assert(printed(empty) == "Total: 0.00 lei\n");
    }

    void testShoppingListFromPlan() {
        // one day: chicken_breast is in lunch and dinner → one line, 400 g; total = the day's price
        const DayMeals day{oatmealBanana(), plainChicken(), spicedChicken()};
        const ShoppingList dayList{day.giveIngredients()};
        assert(printed(dayList) == "oats (80 - g) (1.20 lei)\n"
                                   "milk (200 - ml) (1.60 lei)\n"
                                   "banana (1 - pieces) (1.17 lei)\n"
                                   "cinnamon (0.5 - tsp)\n"
                                   "chicken_breast (400 - g) (13.60 lei)\n"
                                   "salt (0.5 - tsp)\n"
                                   "Total: 17.57 lei\n");

        // a week of 21 different recipes at 1 leu each: 21 lines, total = the week's money spent
        const MealPlan plan{70, 2000, catalogOf(weekRows())};
        const std::string week = printed(ShoppingList{plan.giveIngredients()});
        assert(countOf(week, "\n") == 22);
        assert(week.starts_with("b7_ing (1 - pieces) (1.00 lei)\n"));
        assert(week.ends_with("Total: 21.00 lei\n"));

        // the scaled plan's list: every amount and the total × 3
        const MealPlan scaledPlan = plan.scaled(3);
        const std::string scaledWeek = printed(ShoppingList{scaledPlan.giveIngredients()});
        assert(scaledWeek.starts_with("b7_ing (3 - pieces) (3.00 lei)\n"));
        assert(scaledWeek.ends_with("Total: 63.00 lei\n"));
    }

    void testShoppingListExport() {
        const ShoppingList list{{Ingredient{1.50, "oats", "g", 80}, Ingredient{1.50, "oats", "g", 20}}};
        const std::string fileName = "shopping_list_test.txt";
        list.exportList(fileName);
        std::ifstream file{fileName};
        std::stringstream content;
        content << file.rdbuf();
        file.close();
        std::remove(fileName.c_str());
        assert(content.str() == "oats (100 - g) (1.50 lei)\nTotal: 1.50 lei\n");

        // a folder that does not exist: openFile throws FileError
        bool threw = false;
        try {
            list.exportList("no_such_folder/list.txt");
        } catch (const FileError&) {
            threw = true;
        }
        assert(threw);
    }

    void testIngredientSubtract() {
        Ingredient oats{1.50, "oats", "g", 150};
        const Ingredient& same = (oats -= Ingredient{0, "oats", "g", 50});
        assert(&same == &oats);
        assert(printed(oats) == "oats (100 - g) (1.50 lei)");
        assert(!oats.isEmpty());

        // another unit is refused, and the ingredient stays as it was
        const bool otherUnit = throwsError<UnitError>([&oats] { oats -= Ingredient{0, "oats", "pieces", 1}; });
        const bool otherName = throwsError<UnitError>([&oats] { oats -= Ingredient{0, "milk", "g", 1}; });
        assert(otherUnit);
        assert(otherName);
        assert(printed(oats) == "oats (100 - g) (1.50 lei)");

        // more than needed at home: nothing left to buy, never a negative amount
        oats -= Ingredient{0, "oats", "g", 300};
        assert(oats.isEmpty());
        assert(printed(oats) == "oats (0 - g) (0.00 lei)");

        // exactly what is needed is also empty
        Ingredient milk{0.80, "milk", "ml", 200};
        milk -= Ingredient{0, "milk", "ml", 200};
        assert(milk.isEmpty());
    }

    // the pantry items, one printed ingredient per line (pantry items have no price)
    std::string pantryText(const std::string& input) {
        std::string text;
        for (const Ingredient& item : pantryReader(input)) {
            text += printed(item) + "\n";
        }
        return text;
    }

    void testPantryReader() {
        assert(pantryText("oats 200 g") == "oats (200 - g) (0.00 lei)\n");
        assert(pantryText("Peanut_Butter 50 g") == "peanut_butter (50 - g) (0.00 lei)\n");
        assert(pantryText("oats 200 g, milk 500 ml,eggs 2 pieces") ==
               "oats (200 - g) (0.00 lei)\nmilk (500 - ml) (0.00 lei)\neggs (2 - pieces) (0.00 lei)\n");

        // every bad shape throws a WeekBiteError (InputError for the shape, AmountError for <= 0)
        const std::string bad[] = {
            "",                       // nothing typed
            "oats",                   // no amount
            "oats 200",               // no unit
            "oats g 200",             // amount is not a number
            "oats 200 g extra",       // a word too many
            "oats 0 g",               // amount must be > 0
            "oats -5 g",
            "oats 200 cups",          // unknown unit
            "oats 200 g,,milk 1 ml",  // an empty item between commas
        };
        for (const std::string& input : bad) {
            const bool threw = throwsError<WeekBiteError>([&input] { (void) pantryReader(input); });
            assert(threw);
        }
    }

    ShoppingList breakfastList() {
        return ShoppingList{{
            Ingredient{1.50, "oats", "g", 130},
            Ingredient{0.80, "milk", "ml", 300},
            Ingredient{0, "cinnamon", "tsp", 1.5},
            Ingredient{1.17, "banana", "pieces", 2},
        }};
    }

    void testShoppingListPantry() {
        // oats: 200 g at home covers 130 g → gone; milk: 300 - 100 = 200 ml; total recomputed
        ShoppingList list = breakfastList();
        list.subtractPantry(pantryReader("milk 100 ml, oats 200 g"));
        assert(printed(list) == "milk (200 - ml) (1.60 lei)\n"
                                "cinnamon (1.5 - tsp)\n"
                                "banana (2 - pieces) (2.34 lei)\n"
                                "Total: 3.94 lei\n");

        // nothing missing → no names come back
        ShoppingList found = breakfastList();
        const std::vector<std::string> noneMissing = found.subtractPantry(pantryReader("milk 100 ml"));
        assert(noneMissing.empty());

        // things at home that are not on the list change nothing; their names come back, in the order typed
        ShoppingList same = breakfastList();
        const std::vector<std::string> missing = same.subtractPantry(pantryReader("rice 500 g, milk 0.5 ml, eggs 2 pieces"));
        assert(missing == std::vector<std::string>({"rice", "eggs"}));
        assert(printed(same) == "oats (130 - g) (1.95 lei)\n"
                                "milk (299.5 - ml) (2.40 lei)\n"
                                "cinnamon (1.5 - tsp)\n"
                                "banana (2 - pieces) (2.34 lei)\n"
                                "Total: 6.69 lei\n");

        // names are read in lowercase, so "OATS" matches oats
        ShoppingList upper = breakfastList();
        const std::vector<std::string> upperMissing = upper.subtractPantry(pantryReader("OATS 130 g, Milk 300 ml"));
        assert(upperMissing.empty());
        assert(printed(upper) == "cinnamon (1.5 - tsp)\nbanana (2 - pieces) (2.34 lei)\nTotal: 2.34 lei\n");

        // one bad item (banana in g) → refused, and the whole list stays as it was (oats is not touched either)
        ShoppingList kept = breakfastList();
        const bool threw = throwsError<UnitError>([&kept] { kept.subtractPantry(pantryReader("oats 50 g, banana 100 g")); });
        assert(threw);
        assert(printed(kept) == printed(breakfastList()));

        // everything at home → empty list
        ShoppingList none = breakfastList();
        none.subtractPantry(pantryReader("oats 130 g, milk 1000 ml, cinnamon 2 tsp, banana 2 pieces"));
        assert(printed(none) == "Total: 0.00 lei\n");
    }

    void testShoppingListSort() {
        // prices: oats 1.95, milk 2.40, cinnamon 0, banana 2.34
        ShoppingList byPrice = breakfastList();
        byPrice.sortBy(ListSort::price);
        assert(printed(byPrice) == "milk (300 - ml) (2.40 lei)\n"
                                   "banana (2 - pieces) (2.34 lei)\n"
                                   "oats (130 - g) (1.95 lei)\n"
                                   "cinnamon (1.5 - tsp)\n"
                                   "Total: 6.69 lei\n");

        ShoppingList byName = breakfastList();
        byName.sortBy(ListSort::name);
        assert(printed(byName) == "banana (2 - pieces) (2.34 lei)\n"
                                  "cinnamon (1.5 - tsp)\n"
                                  "milk (300 - ml) (2.40 lei)\n"
                                  "oats (130 - g) (1.95 lei)\n"
                                  "Total: 6.69 lei\n");

        // same price → the order they had stays (stable sort)
        ShoppingList ties{{
            Ingredient{1.00, "b_item", "pieces", 1},
            Ingredient{1.00, "a_item", "pieces", 1},
            Ingredient{2.00, "c_item", "pieces", 1},
        }};
        ties.sortBy(ListSort::price);
        assert(printed(ties) == "c_item (1 - pieces) (2.00 lei)\n"
                                "b_item (1 - pieces) (1.00 lei)\n"
                                "a_item (1 - pieces) (1.00 lei)\n"
                                "Total: 4.00 lei\n");
    }

    void testNutritionLimits() {
        // 2000 kcal, 90 g protein, 250 g carbs, 70 g fats, 45 g sugar
        const Nutrition day{2000, 90, 250, 70, 45};
        // maximum: a value above its limit fails; a limit of 0 means no limit; equal is fine
        assert(day.fitsUnder(Nutrition{0, 0, 0, 0, 50}));
        assert(!day.fitsUnder(Nutrition{0, 0, 0, 0, 40}));
        assert(day.fitsUnder(Nutrition{0, 0, 0, 0, 0}));
        assert(day.fitsUnder(Nutrition{0, 0, 250, 0, 0}));
        assert(!day.fitsUnder(Nutrition{0, 0, 249, 0, 0}));
        assert(!day.fitsUnder(Nutrition{1999, 0, 0, 0, 0}));
        assert(!day.fitsUnder(Nutrition{0, 89, 0, 0, 0}));
        assert(!day.fitsUnder(Nutrition{0, 0, 0, 69, 0}));
        // minimum: a value below its minimum fails; a minimum of 0 is always reached; equal is fine
        assert(day.reaches(Nutrition{0, 80, 0, 0, 0}));
        assert(!day.reaches(Nutrition{0, 100, 0, 0, 0}));
        assert(day.reaches(Nutrition{0, 0, 0, 0, 0}));
        assert(day.reaches(Nutrition{0, 90, 0, 0, 0}));
        assert(!day.reaches(Nutrition{0, 0, 0, 0, 46}));
        assert(!day.reaches(Nutrition{2001, 0, 0, 0, 0}));
    }

    // a recipe that only has calories (enough for the balance rule)
    Recipe kcalRecipe(const std::string& name, double kcal) {
        return Recipe{name, "lunch", {}, Nutrition{kcal, 0, 0, 0, 0}, 1, 10};
    }

    void testMealBalanceConstraint() {
        // used the way MealPlan uses it: through a pointer to the base class
        const std::unique_ptr<PlanConstraint> balance = std::make_unique<MealBalanceConstraint>();
        const std::vector<DayMeals> noDays;
        const Recipe schnitzel = kcalRecipe("schnitzel", 780);
        const Recipe consomme = kcalRecipe("consomme", 120);
        const Recipe porridge = kcalRecipe("porridge", 611);

        // the first meal of the day has nothing to compare with
        assert(balance->isValid(noDays, {}, schnitzel));
        assert(balance->isValid(noDays, {}, consomme));
        // more than 3x in either direction is rejected
        assert(!balance->isValid(noDays, {schnitzel}, consomme));
        assert(!balance->isValid(noDays, {consomme}, schnitzel));
        assert(balance->isValid(noDays, {schnitzel}, porridge));
        // exactly 3x is still allowed
        assert(balance->isValid(noDays, {kcalRecipe("a", 300)}, kcalRecipe("b", 900)));
        assert(!balance->isValid(noDays, {kcalRecipe("a", 300)}, kcalRecipe("b", 901)));
        // compared with every meal so far, not only the last one
        assert(!balance->isValid(noDays, {schnitzel, porridge}, consomme));
        assert(!balance->isValid(noDays, {porridge, schnitzel}, kcalRecipe("small", 250)));

        // a clone answers the same way
        const std::unique_ptr<PlanConstraint> copy = balance->clone();
        assert(copy != nullptr && copy != balance);
        assert(!copy->isValid(noDays, {schnitzel}, consomme));
        assert(copy->isValid(noDays, {schnitzel}, porridge));
    }

    std::vector<std::unique_ptr<PlanConstraint>> onlyBalance() {
        std::vector<std::unique_ptr<PlanConstraint>> constraints;
        constraints.push_back(std::make_unique<MealBalanceConstraint>());
        return constraints;
    }

    void testMealPlanWithBalance() {
        // target 6000: lunch l7 (760), half = (6000 - 760) / 2 = 2620, breakfast b7 (470);
        // "feast" (2500 kcal, 1 leu) is the dinner closest to 2620, but 2500 > 3 x 760
        std::vector<PlanRow> rows = weekRows();
        rows.emplace_back("feast", "dinner", 2500, 1);

        const MealPlan noRules{70, 6000, catalogOf(rows)};
        assert(hasSevenDays(noRules));
        assert(timesServed(dayText(printed(noRules), 1), "feast") == 1);

        const MealPlan balanced{70, 6000, catalogOf(rows), onlyBalance()};
        assert(hasSevenDays(balanced));
        const std::string output = printed(balanced);
        assert(timesServed(output, "feast") == 0);
        assert(timesServed(dayText(output, 1), "d7") == 1);

        // no constraints passed (the default {}) keeps the old behaviour
        const MealPlan none{70, 6000, catalogOf(rows), {}};
        assert(printed(none) == printed(noRules));
    }

    // a recipe with calories, protein and sugar
    Recipe nutrientRecipe(const std::string& name, double protein, double sugar) {
        return Recipe{name, "lunch", {}, Nutrition{500, protein, 0, 0, sugar}, 1, 10};
    }

    void testNutritionTargetConstraint() {
        // at least 80 g protein and at most 50 g sugar per day
        const std::unique_ptr<PlanConstraint> target = std::make_unique<NutritionTargetConstraint>(
            Nutrition{0, 80, 0, 0, 0}, Nutrition{0, 0, 0, 0, 50});
        const std::vector<DayMeals> noDays;
        const Recipe lean = nutrientRecipe("lean", 30, 10);
        const Recipe sweet = nutrientRecipe("sweet", 5, 45);
        const Recipe rich = nutrientRecipe("rich", 40, 5);

        // a maximum is checked at every meal: 45 + 10 > 50
        assert(target->isValid(noDays, {}, sweet));
        assert(!target->isValid(noDays, {lean}, sweet));
        // the minimum is not judged before the last meal: 30 g protein is fine for now
        assert(target->isValid(noDays, {}, lean));
        assert(target->isValid(noDays, {lean}, lean));
        // last meal (third): 30 + 30 + 30 = 90 >= 80 is reached, 30 + 30 + 5 = 65 is not
        assert(target->isValid(noDays, {lean, lean}, lean));
        assert(!target->isValid(noDays, {lean, lean}, nutrientRecipe("low", 5, 0)));
        // exactly the minimum is enough: 40 + 30 + 10 = 80
        assert(target->isValid(noDays, {rich, lean}, nutrientRecipe("ten", 10, 0)));

        // no targets at all (0 everywhere): every day is fine, also the last meal
        const NutritionTargetConstraint none{Nutrition{0, 0, 0, 0, 0}, Nutrition{0, 0, 0, 0, 0}};
        const PlanConstraint& noTargets = none;
        assert(noTargets.isValid(noDays, {lean, lean}, nutrientRecipe("low", 5, 0)));
        assert(noTargets.isValid(noDays, {sweet, sweet}, sweet));

        // a clone keeps the user's targets
        const std::unique_ptr<PlanConstraint> copy = target->clone();
        assert(!copy->isValid(noDays, {lean}, sweet));
        assert(!copy->isValid(noDays, {lean, lean}, nutrientRecipe("low", 5, 0)));
        assert(copy->isValid(noDays, {lean, lean}, lean));
    }

    // a recipe that only matters for its cooking time
    Recipe timedRecipe(const std::string& name, int minutes) {
        return Recipe{name, "lunch", {}, Nutrition{500, 0, 0, 0, 0}, 1, minutes};
    }

    // n finished days, so the next one is day n (0 = Monday)
    std::vector<DayMeals> daysBuilt(int n) {
        const Recipe meal = timedRecipe("meal", 10);
        return std::vector<DayMeals>(static_cast<std::size_t>(n), DayMeals{meal, meal, meal});
    }

    void testRecipeTakesAtMost() {
        const Recipe stew = timedRecipe("stew", 45);
        assert(stew.takesAtMost(45));
        assert(stew.takesAtMost(60));
        assert(!stew.takesAtMost(44));
        // the cooking time survives copies and scaling
        const Recipe copy = stew;
        assert(!copy.takesAtMost(44));
        Recipe assigned = timedRecipe("quick", 5);
        assigned = stew;
        assert(!assigned.takesAtMost(44));
        assert(!stew.scaled(3).takesAtMost(44));
    }

    void testWeekdayPrepTimeConstraint() {
        const std::unique_ptr<PlanConstraint> weekdays = std::make_unique<WeekdayPrepTimeConstraint>(30, true);
        const std::unique_ptr<PlanConstraint> everyDay = std::make_unique<WeekdayPrepTimeConstraint>(30, false);
        const Recipe quick = timedRecipe("quick", 30);
        const Recipe slow = timedRecipe("slow", 31);

        // Monday (no days built yet) to Friday (4 built): a meal over 30 minutes is rejected
        for (int built = 0; built <= 4; built++) {
            assert(weekdays->isValid(daysBuilt(built), {}, quick));
            assert(!weekdays->isValid(daysBuilt(built), {}, slow));
        }
        // Saturday and Sunday: no limit when only weekdays are limited
        assert(weekdays->isValid(daysBuilt(5), {}, slow));
        assert(weekdays->isValid(daysBuilt(6), {}, slow));
        // every day limited: the weekend too
        assert(!everyDay->isValid(daysBuilt(0), {}, slow));
        assert(!everyDay->isValid(daysBuilt(6), {}, slow));
        assert(everyDay->isValid(daysBuilt(6), {}, quick));

        // a clone keeps both settings
        const std::unique_ptr<PlanConstraint> weekdaysCopy = weekdays->clone();
        assert(!weekdaysCopy->isValid(daysBuilt(0), {}, slow));
        assert(weekdaysCopy->isValid(daysBuilt(5), {}, slow));
        const std::unique_ptr<PlanConstraint> everyDayCopy = everyDay->clone();
        assert(!everyDayCopy->isValid(daysBuilt(6), {}, slow));
    }

    std::vector<std::unique_ptr<PlanConstraint>> onlyPrepTime(bool weekdaysOnly) {
        std::vector<std::unique_ptr<PlanConstraint>> constraints;
        constraints.push_back(std::make_unique<WeekdayPrepTimeConstraint>(30, weekdaysOnly));
        return constraints;
    }

    void testMealPlanWithPrepTime() {
        // "roast" is exactly 40% of 2000 kcal, so it is the first lunch tried every day, but takes 90 minutes
        // (every other test recipe takes 10)
        std::vector<PlanRow> rows = weekRows();
        rows.emplace_back("roast", "lunch", 800, 1);
        std::map<std::string, IngredientInfo> prices;
        std::vector<RecipeRow> recipeRows;
        std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe;
        for (const auto& [name, category, kcal, price] : rows) {
            prices[name + "_ing"] = {price, {}, {}};
            recipeRows.emplace_back(name, category, Nutrition{kcal, 0, 0, 0, 0}, name == "roast" ? 90 : 10);
            ingredientsByRecipe[name] = {{name + "_ing", 1, "pieces"}};
        }
        const Recipes catalog{prices, recipeRows, ingredientsByRecipe};

        // no limit: the roast is Monday's lunch
        const MealPlan noLimit{70, 2000, catalog};
        assert(timesServed(dayText(printed(noLimit), 1), "roast") == 1);
        // weekdays only: skipped Monday to Friday, served on Saturday (day 6)
        const MealPlan weekdays{70, 2000, catalog, onlyPrepTime(true)};
        assert(hasSevenDays(weekdays));
        const std::string weekdaysOutput = printed(weekdays);
        assert(timesServed(weekdaysOutput, "roast") == 1);
        assert(timesServed(dayText(weekdaysOutput, 6), "roast") == 1);
        // every day: never served, the week still completes with the other 7 lunches
        const MealPlan everyDay{70, 2000, catalog, onlyPrepTime(false)};
        assert(hasSevenDays(everyDay));
        assert(timesServed(printed(everyDay), "roast") == 0);
    }

    void testSplitText() {
        assert((splitText("vegan;vegetarian", ';') == std::vector<std::string>{"vegan", "vegetarian"}));
        assert((splitText("red meat;pork", ';') == std::vector<std::string>{"red meat", "pork"}));
        assert((splitText("vegan", ',') == std::vector<std::string>{"vegan"}));
        // empty pieces are skipped, an empty text gives nothing
        assert((splitText("vegan,,nopork,", ',') == std::vector<std::string>{"vegan", "nopork"}));
        assert(splitText("", ';').empty());
    }

    void testIngredientTags() {
        const Ingredient bread{1.0, "bread", "g", 100, {"vegan", "vegetarian"}, {"gluten"}};
        assert(bread.isSuitableFor("vegan"));
        assert(bread.isSuitableFor("vegetarian"));
        assert(bread.hasTag("gluten"));
        assert(!bread.hasTag("lactose"));
        // no tags at all (the default): suitable for nothing, contains nothing
        const Ingredient chicken{3.40, "chicken_breast", "g", 200};
        assert(!chicken.isSuitableFor("vegetarian"));
        assert(!chicken.hasTag("pork"));
        // the tags are copied with the ingredient
        const Ingredient copy = bread;
        assert(copy.isSuitableFor("vegan") && copy.hasTag("gluten"));
    }

    // tags used below: tofu (vegan, vegetarian), cheese (vegetarian, lactose), bread (vegan, vegetarian, gluten),
    // bacon (red meat, pork), beef (red meat)
    Ingredient taggedIngredient(const std::string& name) {
        if (name == "tofu") { return Ingredient{1, name, "g", 100, {"vegan", "vegetarian"}, {}}; }
        if (name == "cheese") { return Ingredient{1, name, "g", 100, {"vegetarian"}, {"lactose"}}; }
        if (name == "bread") { return Ingredient{1, name, "g", 100, {"vegan", "vegetarian"}, {"gluten"}}; }
        if (name == "bacon") { return Ingredient{1, name, "g", 100, {}, {"red meat", "pork"}}; }
        return Ingredient{1, name, "g", 100, {}, {"red meat"}};
    }

    Recipe taggedRecipe(const std::string& name, const std::vector<std::string>& parts) {
        std::vector<Ingredient> ingredients;
        for (const std::string& ingredientName : parts) {
            ingredients.push_back(taggedIngredient(ingredientName));
        }
        return Recipe{name, "lunch", ingredients, Nutrition{500, 0, 0, 0, 0}, 1, 10};
    }

    void testRecipeTags() {
        const Recipe tofuToast = taggedRecipe("tofu_toast", {"tofu", "bread"});
        const Recipe cheeseToast = taggedRecipe("cheese_toast", {"cheese", "bread"});
        // every ingredient must be suitable
        assert(tofuToast.allIngredientsSuitableFor("vegan"));
        assert(!cheeseToast.allIngredientsSuitableFor("vegan"));
        assert(cheeseToast.allIngredientsSuitableFor("vegetarian"));
        // one ingredient with the tag is enough
        assert(tofuToast.containsAny("gluten"));
        assert(cheeseToast.containsAny("lactose"));
        assert(!tofuToast.containsAny("lactose"));
        // a recipe without ingredients contains nothing and fits every diet
        const Recipe water = taggedRecipe("water", {});
        assert(water.allIngredientsSuitableFor("vegan"));
        assert(!water.containsAny("gluten"));
    }

    void testDietFromText() {
        assert(dietFromText("vegan") == Diet::vegan);
        assert(dietFromText("vegetarian") == Diet::vegetarian);
        assert(dietFromText("glutenfree") == Diet::glutenFree);
        assert(dietFromText("lactosefree") == Diet::lactoseFree);
        assert(dietFromText("nopork") == Diet::noPork);
        assert(dietFromText("noredmeat") == Diet::noRedMeat);
        // any case
        assert(dietFromText("Vegan") == Diet::vegan);
        assert(dietFromText("NoRedMeat") == Diet::noRedMeat);
        const std::string unknown[] = {"vegn", "", "no pork", "glutenfree "};
        for (const std::string& text : unknown) {
            bool threw = false;
            try {
                (void) dietFromText(text);
            } catch (const InputError&) {
                threw = true;
            }
            assert(threw && "an unknown diet name must throw");
        }
    }

    bool dietAccepts(Diet diet, const Recipe& recipe) {
        const std::unique_ptr<PlanConstraint> constraint = std::make_unique<DietRestrictionConstraint>(diet);
        return constraint->isValid({}, {}, recipe);
    }

    void testDietRestrictionConstraint() {
        const Recipe tofuToast = taggedRecipe("tofu_toast", {"tofu", "bread"});      // vegan, gluten
        const Recipe cheeseSalad = taggedRecipe("cheese_salad", {"cheese", "tofu"});  // vegetarian, lactose
        const Recipe blt = taggedRecipe("blt", {"bacon", "bread"});                   // pork, red meat, gluten
        const Recipe steak = taggedRecipe("steak", {"beef"});                         // red meat, no pork

        assert(dietAccepts(Diet::vegan, tofuToast));
        assert(!dietAccepts(Diet::vegan, cheeseSalad));
        assert(!dietAccepts(Diet::vegan, steak));
        assert(dietAccepts(Diet::vegetarian, cheeseSalad));
        assert(!dietAccepts(Diet::vegetarian, blt));
        assert(!dietAccepts(Diet::glutenFree, tofuToast));
        assert(dietAccepts(Diet::glutenFree, cheeseSalad));
        assert(!dietAccepts(Diet::lactoseFree, cheeseSalad));
        assert(dietAccepts(Diet::lactoseFree, tofuToast));
        assert(!dietAccepts(Diet::noPork, blt));
        assert(dietAccepts(Diet::noPork, steak));
        assert(!dietAccepts(Diet::noRedMeat, blt));
        assert(!dietAccepts(Diet::noRedMeat, steak));
        assert(dietAccepts(Diet::noRedMeat, tofuToast));

        // a clone keeps the diet
        const std::unique_ptr<PlanConstraint> glutenFree = std::make_unique<DietRestrictionConstraint>(Diet::glutenFree);
        const std::unique_ptr<PlanConstraint> copy = glutenFree->clone();
        assert(!copy->isValid({}, {}, tofuToast));
        assert(copy->isValid({}, {}, cheeseSalad));
    }

    void testMealPlanWithDiet() {
        // weekRows plus a pork lunch that is exactly 40% of 2000 kcal, so it would be Monday's lunch
        std::vector<PlanRow> rows = weekRows();
        rows.emplace_back("pork_roast", "lunch", 800, 1);
        std::map<std::string, IngredientInfo> info;
        std::vector<RecipeRow> recipeRows;
        std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe;
        for (const auto& [name, category, kcal, price] : rows) {
            const std::vector<std::string> contains =
                name == "pork_roast" ? std::vector<std::string>{"red meat", "pork"} : std::vector<std::string>{};
            info[name + "_ing"] = {price, {}, contains};
            recipeRows.emplace_back(name, category, Nutrition{kcal, 0, 0, 0, 0}, 10);
            ingredientsByRecipe[name] = {{name + "_ing", 1, "pieces"}};
        }
        const Recipes catalog{info, recipeRows, ingredientsByRecipe};

        const MealPlan anything{70, 2000, catalog};
        assert(timesServed(dayText(printed(anything), 1), "pork_roast") == 1);

        std::vector<std::unique_ptr<PlanConstraint>> noPork;
        noPork.push_back(std::make_unique<DietRestrictionConstraint>(Diet::noPork));
        const MealPlan withoutPork{70, 2000, catalog, std::move(noPork)};
        assert(hasSevenDays(withoutPork));
        assert(timesServed(printed(withoutPork), "pork_roast") == 0);

        // vegan: no test ingredient is tagged vegan, so not even one day can be built;
        // the same 70 lei fill the week without the diet (anything above) → the restrictions are blamed
        std::vector<std::unique_ptr<PlanConstraint>> vegan;
        vegan.push_back(std::make_unique<DietRestrictionConstraint>(Diet::vegan));
        const std::string veganFailure = planFailure(70, 2000, catalog, std::move(vegan));
        assert(veganFailure == restrictionsReason + ": only 0 breakfast recipes fit your diets");
    }

    // one recipe of the diet-shortage catalogs: vegan = its ingredient is tagged vegan, gluten = it contains gluten
    struct DietMeal {
        std::string name;
        std::string category;
        double kcal = 0;
        int minutes = 10;
        bool vegan = false;
        bool gluten = false;
    };

    // adds `count` recipes of one category (kcal like weekRows, 1 leu each): "<prefix>1" .. "<prefix><count>"
    void addMeals(std::vector<DietMeal>& meals, const std::string& prefix, const std::string& category, int count,
                  bool vegan, bool gluten, int minutes = 10) {
        const double base = category == "breakfast" ? 400 : category == "lunch" ? 690 : 450;
        for (int i = 1; i <= count; i++) {
            meals.push_back({prefix + std::to_string(i), category, base + 10 * i, minutes, vegan, gluten});
        }
    }

    Recipes dietCatalogOf(const std::vector<DietMeal>& meals) {
        std::map<std::string, IngredientInfo> info;
        std::vector<RecipeRow> recipeRows;
        std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe;
        for (const DietMeal& meal : meals) {
            const std::vector<std::string> suitableFor = meal.vegan ? std::vector<std::string>{"vegan"}
                                                                    : std::vector<std::string>{};
            const std::vector<std::string> contains = meal.gluten ? std::vector<std::string>{"gluten"}
                                                                  : std::vector<std::string>{};
            info[meal.name + "_ing"] = {1, suitableFor, contains};
            recipeRows.emplace_back(meal.name, meal.category, Nutrition{meal.kcal, 0, 0, 0, 0}, meal.minutes);
            ingredientsByRecipe[meal.name] = {{meal.name + "_ing", 1, "pieces"}};
        }
        return Recipes{info, recipeRows, ingredientsByRecipe};
    }

    std::vector<std::unique_ptr<PlanConstraint>> dietRules(const std::vector<Diet>& diets) {
        std::vector<std::unique_ptr<PlanConstraint>> rules;
        for (const Diet diet : diets) {
            rules.push_back(std::make_unique<DietRestrictionConstraint>(diet));
        }
        return rules;
    }

    // the message the diet rules add after "too few recipes" (one case per call, 70 lei a week)
    void testDietShortageMessage() {
        const std::string prefix = restrictionsReason + ": ";

        // two diets that are fine alone but not together: 8 breakfasts are vegan, 8 are gluten-free, only 6 are both
        // (a count per diet would say 8; the message must count the recipes that ALL the diets allow)
        std::vector<DietMeal> both;
        addMeals(both, "bothOk", "breakfast", 6, true, false);
        addMeals(both, "veganGluten", "breakfast", 2, true, true);
        addMeals(both, "plainOnly", "breakfast", 2, false, false);
        addMeals(both, "l", "lunch", 7, true, false);
        addMeals(both, "d", "dinner", 7, true, false);
        const Recipes bothCatalog = dietCatalogOf(both);
        assert(planFailure(70, 2000, bothCatalog).empty());
        assert(planFailure(70, 2000, bothCatalog, dietRules({Diet::vegan})).empty());
        assert(planFailure(70, 2000, bothCatalog, dietRules({Diet::glutenFree})).empty());
        const std::string together = planFailure(70, 2000, bothCatalog, dietRules({Diet::vegan, Diet::glutenFree}));
        assert(together == prefix + "only 6 breakfast recipes fit your diets");
        // the order of the rules changes nothing
        const std::string reversed = planFailure(70, 2000, bothCatalog, dietRules({Diet::glutenFree, Diet::vegan}));
        assert(reversed == together);

        // breakfasts are fine, lunches are short: the lunch is named, with its own count
        std::vector<DietMeal> fewLunches;
        addMeals(fewLunches, "b", "breakfast", 7, true, false);
        addMeals(fewLunches, "l", "lunch", 5, true, false);
        addMeals(fewLunches, "meatyLunch", "lunch", 3, false, false);
        addMeals(fewLunches, "d", "dinner", 7, true, false);
        const Recipes lunchCatalog = dietCatalogOf(fewLunches);
        assert(planFailure(70, 2000, lunchCatalog).empty());
        assert(planFailure(70, 2000, lunchCatalog, dietRules({Diet::vegan})) ==
               prefix + "only 5 lunch recipes fit your diets");

        // only the dinners are short: the dinner is named
        std::vector<DietMeal> fewDinners;
        addMeals(fewDinners, "b", "breakfast", 7, true, false);
        addMeals(fewDinners, "l", "lunch", 7, true, false);
        addMeals(fewDinners, "d", "dinner", 2, true, false);
        addMeals(fewDinners, "meatyDinner", "dinner", 6, false, false);
        const Recipes dinnerCatalog = dietCatalogOf(fewDinners);
        assert(planFailure(70, 2000, dinnerCatalog).empty());
        assert(planFailure(70, 2000, dinnerCatalog, dietRules({Diet::vegan})) ==
               prefix + "only 2 dinner recipes fit your diets");

        // several meals short: the first one in the order breakfast, lunch, dinner is the one reported
        std::vector<DietMeal> manyShort;
        addMeals(manyShort, "b", "breakfast", 1, true, false);
        addMeals(manyShort, "meatyB", "breakfast", 7, false, false);
        addMeals(manyShort, "l", "lunch", 3, true, false);
        addMeals(manyShort, "meatyL", "lunch", 7, false, false);
        addMeals(manyShort, "d", "dinner", 4, true, false);
        addMeals(manyShort, "meatyD", "dinner", 7, false, false);
        const Recipes manyCatalog = dietCatalogOf(manyShort);
        assert(planFailure(70, 2000, manyCatalog, dietRules({Diet::vegan})) ==
               prefix + "only 1 breakfast recipes fit your diets");
        // without the breakfast problem, the lunch comes before the dinner
        std::vector<DietMeal> lunchAndDinner;
        addMeals(lunchAndDinner, "b", "breakfast", 7, true, false);
        addMeals(lunchAndDinner, "l", "lunch", 3, true, false);
        addMeals(lunchAndDinner, "meatyL", "lunch", 7, false, false);
        addMeals(lunchAndDinner, "d", "dinner", 4, true, false);
        addMeals(lunchAndDinner, "meatyD", "dinner", 7, false, false);
        assert(planFailure(70, 2000, dietCatalogOf(lunchAndDinner), dietRules({Diet::vegan})) ==
               prefix + "only 3 lunch recipes fit your diets");

        // boundary: exactly 7 fitting recipes in every meal are enough, so the diet is not named.
        // The plan still fails, because one of the 7 lunches takes 90 minutes and the other rule rejects it
        // (the cast skips the cooking-time rule: only the diet is counted)
        std::vector<DietMeal> seven;
        addMeals(seven, "b", "breakfast", 7, true, false);
        addMeals(seven, "l", "lunch", 6, true, false);
        addMeals(seven, "slow", "lunch", 1, true, false, 90);
        addMeals(seven, "d", "dinner", 7, true, false);
        const Recipes sevenCatalog = dietCatalogOf(seven);
        assert(planFailure(70, 2000, sevenCatalog).empty());
        assert(planFailure(70, 2000, sevenCatalog, dietRules({Diet::vegan})).empty());
        std::vector<std::unique_ptr<PlanConstraint>> veganAndFast = dietRules({Diet::vegan});
        veganAndFast.push_back(std::make_unique<WeekdayPrepTimeConstraint>(30, false));
        const std::string sevenFit = planFailure(70, 2000, sevenCatalog, std::move(veganAndFast));
        assert(sevenFit == restrictionsReason);

        // the same catalog with one lunch that is not vegan: 6 fit, which is one too few
        std::vector<DietMeal> six = seven;
        for (DietMeal& meal : six) {
            if (meal.name == "slow1") {
                meal.vegan = false;
            }
        }
        std::vector<std::unique_ptr<PlanConstraint>> veganAndFastAgain = dietRules({Diet::vegan});
        veganAndFastAgain.push_back(std::make_unique<WeekdayPrepTimeConstraint>(30, false));
        const std::string sixFit = planFailure(70, 2000, dietCatalogOf(six), std::move(veganAndFastAgain));
        assert(sixFit == prefix + "only 6 lunch recipes fit your diets");

        // only rules that are not diets: no diet to count, so just the plain reason (no ':' at all)
        std::vector<std::unique_ptr<PlanConstraint>> onlyFast;
        onlyFast.push_back(std::make_unique<WeekdayPrepTimeConstraint>(30, false));
        onlyFast.push_back(std::make_unique<MealBalanceConstraint>());
        const std::string noDiets = planFailure(70, 2000, sevenCatalog, std::move(onlyFast));
        assert(noDiets == restrictionsReason);
        assert(noDiets.find(':', restrictionsReason.find(':') + 1) == std::string::npos);

        // the budget is still blamed first: 7 lei fail even without the rules, so no diet text
        const std::string poor = planFailure(7, 2000, bothCatalog, dietRules({Diet::vegan, Diet::glutenFree}));
        assert(poor == budgetReason);
    }

    // allows() is the same question isValid asks through a PlanConstraint pointer
    void testDietAllowsMatchesIsValid() {
        const Recipe tofuToast = taggedRecipe("tofu_toast", {"tofu", "bread"});
        const Recipe cheeseSalad = taggedRecipe("cheese_salad", {"cheese", "tofu"});
        const Recipe blt = taggedRecipe("blt", {"bacon", "bread"});
        const Recipe steak = taggedRecipe("steak", {"beef"});
        const Diet allDiets[] = {Diet::vegan, Diet::vegetarian, Diet::glutenFree,
                                 Diet::lactoseFree, Diet::noPork, Diet::noRedMeat};
        for (const Diet diet : allDiets) {
            const DietRestrictionConstraint rule{diet};
            const PlanConstraint& base = rule;
            for (const Recipe* recipe : {&tofuToast, &cheeseSalad, &blt, &steak}) {
                const bool direct = rule.allows(*recipe);
                const bool viaBase = base.isValid({}, {}, *recipe);
                assert(direct == viaBase);
            }
        }
        // a few answers spelled out
        const DietRestrictionConstraint vegan{Diet::vegan};
        const DietRestrictionConstraint noPork{Diet::noPork};
        assert(vegan.allows(tofuToast));
        assert(!vegan.allows(cheeseSalad));
        assert(noPork.allows(steak));
        assert(!noPork.allows(blt));
    }

    void testMealPlanCopyWithConstraints() {
        std::vector<PlanRow> rows = weekRows();
        rows.emplace_back("feast", "dinner", 2500, 1);
        const MealPlan balanced{70, 6000, catalogOf(rows), onlyBalance()};
        const std::string expected = printed(balanced);

        // copy constructor: the copy clones the constraints (ASan/Valgrind catch a double delete)
        const MealPlan copy = balanced;
        assert(printed(copy) == expected);
        // scaled() copies the plan too
        const MealPlan doubled = balanced.scaled(2);
        assert(hasSevenDays(doubled));
        assert(printed(balanced) == expected);

        // operator=: the old plan and its constraints are replaced, the source stays the same
        MealPlan target{70, 1300, catalogOf(weekRows())};
        assert(printed(target) != expected);
        target = balanced;
        assert(printed(target) == expected);
        assert(printed(balanced) == expected);
        // assigning a temporary (copy-and-swap takes it by value)
        const std::string plain = printed(MealPlan{70, 1300, catalogOf(weekRows())});
        target = MealPlan{70, 1300, catalogOf(weekRows())};
        assert(printed(target) == plain);
        // a temporary that cannot be built throws before operator= runs: target keeps its plan.
        // 7 lei fail with or without the balance rule → the budget is blamed, not the constraint
        std::string reason;
        try {
            target = MealPlan{7, 2000, catalogOf(weekRows()), onlyBalance()};
        } catch (const PlanImpossibleError& e) {
            reason = e.what();
        }
        assert(reason == budgetReason);
        assert(printed(target) == plain);
    }

    std::string printed(const PlanConstraint& constraint) {
        std::ostringstream out;
        out << constraint;
        return out.str();
    }

    void testConstraintOutput() {
        // every rule printed through a PlanConstraint pointer: operator<< -> the derived print_
        std::vector<std::unique_ptr<PlanConstraint>> rules;
        rules.push_back(std::make_unique<MealBalanceConstraint>());
        rules.push_back(std::make_unique<NutritionTargetConstraint>(Nutrition{0, 100, 0, 0, 0},
                                                                    Nutrition{0, 0, 300, 0, 50}));
        rules.push_back(std::make_unique<WeekdayPrepTimeConstraint>(30, true));
        rules.push_back(std::make_unique<WeekdayPrepTimeConstraint>(45, false));
        rules.push_back(std::make_unique<DietRestrictionConstraint>(Diet::glutenFree));
        const std::string expected[] = {
            "Balanced meals: no meal more than 3x the calories of another",
            "Daily nutrition: minimum: protein 100 g; maximum: carbohydrates 300 g, sugar 50 g",
            "Cooking time: at most 30 min per meal, Monday-Friday",
            "Cooking time: at most 45 min per meal, every day",
            "Diet: gluten-free",
        };
        for (std::size_t i = 0; i < rules.size(); i++) {
            assert(printed(*rules[i]) == expected[i]);
            // the clone describes the same rule
            const std::unique_ptr<PlanConstraint> clone = rules[i]->clone();
            assert(printed(*clone) == expected[i]);
        }

        // only the values that are set; "none" when a side is empty; calories in kcal
        const NutritionTargetConstraint noMinimum{Nutrition{0, 0, 0, 0, 0}, Nutrition{2000, 0, 0, 40, 0}};
        assert(printed(noMinimum) == "Daily nutrition: minimum: none; maximum: calories 2000 kcal, fats 40 g");
        const NutritionTargetConstraint nothingSet{Nutrition{0, 0, 0, 0, 0}, Nutrition{0, 0, 0, 0, 0}};
        assert(printed(nothingSet) == "Daily nutrition: minimum: none; maximum: none");
        const NutritionTargetConstraint everyValue{Nutrition{1, 2, 3, 4, 5}, Nutrition{0, 0, 0, 0, 0}};
        assert(printed(everyValue) ==
               "Daily nutrition: minimum: calories 1 kcal, protein 2 g, carbohydrates 3 g, fats 4 g, sugar 5 g; "
               "maximum: none");

        // every diet has its own text (dietToText) and the rule uses it
        const std::pair<Diet, std::string> diets[] = {
            {Diet::vegan, "vegan"}, {Diet::vegetarian, "vegetarian"}, {Diet::glutenFree, "gluten-free"},
            {Diet::lactoseFree, "lactose-free"}, {Diet::noPork, "no pork"}, {Diet::noRedMeat, "no red meat"},
        };
        for (const auto& [diet, text] : diets) {
            assert(dietToText(diet) == text);
            const std::unique_ptr<PlanConstraint> rule = std::make_unique<DietRestrictionConstraint>(diet);
            assert(printed(*rule) == "Diet: " + text);
        }
    }

    void testMealPlanPrintsRules() {
        // no rules: the plan starts with Day 1, no "Rules:" block
        const MealPlan plain{70, 1300, catalogOf(weekRows())};
        assert(printed(plain).rfind("Day 1\n", 0) == 0);
        assert(printed(plain).find("Rules:") == std::string::npos);

        // two rules: listed in the order they were given, then an empty line, then Day 1
        std::vector<std::unique_ptr<PlanConstraint>> rules = onlyBalance();
        rules.push_back(std::make_unique<DietRestrictionConstraint>(Diet::noPork));
        const MealPlan withRules{70, 1300, catalogOf(weekRows()), std::move(rules)};
        const std::string header = "Rules:\n"
                                   "- Balanced meals: no meal more than 3x the calories of another\n"
                                   "- Diet: no pork\n"
                                   "\n"
                                   "Day 1\n";
        assert(printed(withRules).rfind(header, 0) == 0);
        // the rules are printed once, and the days after them are the same as without rules
        assert(printed(withRules).find("Rules:", 1) == std::string::npos);
        assert(printed(withRules).substr(header.size() - 6) == printed(plain));
        // copies and the scaled plan keep (clone) the rules, so they show them too
        const MealPlan copy = withRules;
        assert(printed(copy) == printed(withRules));
        const MealPlan doubled = withRules.scaled(2);
        assert(printed(doubled).rfind(header, 0) == 0);
    }
}

// checked through the public interface only
int main() {
    testIngredient();
    testIngredientOutput();
    testIngredientScaling();
    testNutritionOutput();
    testNutritionSum();
    testRecipePrice();
    testRecipeOutput();
    testRecipeCopy();
    testRecipeScaling();
    testRecipesCatalog();
    testRecipesOutput();
    testDayMeals();
    testDayMealsScaling();
    testGetters();
    testMealPlanWeek();
    testMealPlanClosestCalories();
    testMealPlanLunchOrder();
    testMealPlanBudget();
    testMealPlanFails();
    testMealPlanScaling();
    testRecipeQuestions();
    testPrintShort();
    testUserChoices();
    testSearch();
    testSearchKeepsOrderOnTies();
    testReader();
    testCategoryToText();
    testMealPlanStats();
    testMealPlanStatsTies();
    testRecipesStats();
    testRecipesStatsTies();
    testRecipesStatsEmptyCategory();
    testIngredientAdd();
    testGiveIngredients();
    testShoppingListMerge();
    testShoppingListFromPlan();
    testShoppingListExport();
    testIngredientSubtract();
    testPantryReader();
    testShoppingListPantry();
    testShoppingListSort();
    testNutritionLimits();
    testMealBalanceConstraint();
    testMealPlanWithBalance();
    testMealPlanCopyWithConstraints();
    testNutritionTargetConstraint();
    testRecipeTakesAtMost();
    testWeekdayPrepTimeConstraint();
    testMealPlanWithPrepTime();
    testSplitText();
    testIngredientTags();
    testRecipeTags();
    testDietFromText();
    testDietRestrictionConstraint();
    testMealPlanWithDiet();
    testDietShortageMessage();
    testDietAllowsMatchesIsValid();
    testConstraintOutput();
    testMealPlanPrintsRules();

    return 0;
}
