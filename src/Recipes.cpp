//
// Created by Matei on 10/27/2026.
//

#include "Recipes.h"
#include <stdexcept>

const std::vector<Recipe> &Recipes::getRecipesFromCategory(Category category) const {
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

Recipes::Recipes(const std::map<std::string, double> &prices,
                 const std::vector<RecipeRow> &recipeRows,
                 const std::map<std::string, std::vector<IngredientRow> > &ingredientsByRecipe) {
    for (const auto &[name, categoryText, nutrition]: recipeRows) {
        std::vector<Ingredient> ingredients;
        const auto found = ingredientsByRecipe.find(name); // ask map
        if (found != ingredientsByRecipe.end()) { // check
            for (const auto& [ingredientName, amount, unit] : found->second ) { // use
                double price = 0;
                const auto priceFound = prices.find(ingredientName);
                if (priceFound != prices.end()) { price=priceFound->second;}
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
