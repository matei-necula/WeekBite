//
// Created by Matei on 10/27/2026.
//

#include "Recipes.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>

const std::vector<Recipe>& Recipes::getRecipesFromCategory(Category category) const {
    switch (category) {
        case Category::breakfast:
            return breakfasts;
        case Category::lunch:
            return lunches;
        case Category::dinner:
            return dinners;
    }
    throw std::invalid_argument("Invalid category");
}

Recipes::Recipes(const std::map<std::string, double>& prices,
                 const std::vector<RecipeRow>& recipeRows,
                 const std::map<std::string, std::vector<IngredientRow> >& ingredientsByRecipe) {
    for (const auto& [name, categoryText, nutrition]: recipeRows) {
        std::vector<Ingredient> ingredients;
        const auto found = ingredientsByRecipe.find(name); // ask map
        if (found != ingredientsByRecipe.end()) {
            // check
            for (const auto& [ingredientName, amount, unit]: found->second) {
                // use
                double price = 0;
                const auto priceFound = prices.find(ingredientName);
                if (priceFound != prices.end()) { price = priceFound->second; }
                ingredients.push_back(Ingredient(price, ingredientName, unit, amount));
            }
        }
        const Recipe recipe{name, categoryText, ingredients, nutrition, 1};
        if (categoryText == "breakfast") {
            breakfasts.push_back(recipe);
        } else if (categoryText == "lunch") {
            lunches.push_back(recipe);
        } else if (categoryText == "dinner") {
            dinners.push_back(recipe);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Recipes& recipes) {
    os << "Breakfast:\n";
    for (const Recipe& rec: recipes.breakfasts) {
        os << rec << '\n';
    }
    os << "Lunch:\n";
    for (const Recipe& rec: recipes.lunches) {
        os << rec << '\n';
    }
    os << "Dinner:\n";
    for (const Recipe& rec: recipes.dinners) {
        os << rec << '\n';
    }
    return os;
}


std::vector<Recipe> Recipes::searchUserChoices(const UserChoices& choices) const {
    std::vector<Recipe> results;
    for (const Recipe& rec: breakfasts) {
        if (choices.matches(rec)) {
            results.push_back(rec);
        }
    }
    for (const Recipe& rec: lunches) {
        if (choices.matches(rec)) {
            results.push_back(rec);
        }
    }
    for (const Recipe& rec: dinners) {
        if (choices.matches(rec)) {
            results.push_back(rec);
        }
    }

    std::stable_sort(results.begin(), results.end(),[&choices](const Recipe& a, const Recipe& b) {
                         return choices.comesBefore(a, b);
                     });

    return results;
}

std::tuple<bool, bool, bool> reader(const std::string& input) {
    bool category = false;
    bool name = false;
    bool ingredient = false;

    std::stringstream stream(input);   // read the text piece by piece
    std::string word;
    while (std::getline(stream, word, ',')) {   // reads up to the next comma, into word
        if (word == "category") {
            category = true;
        } else if (word == "name") {
            name = true;
        } else if (word == "ingredient") {
            ingredient = true;
        } else {
            throw std::invalid_argument("Invalid input: " + word);
        }
    }
    if (!category && !name && !ingredient) {
        throw std::invalid_argument("No valid input provided");
    }

    return {category, name, ingredient};
}
