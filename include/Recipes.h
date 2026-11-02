//
// Created by Matei on 10/27/2026.
//

#ifndef OOP_RECIPES_H
#define OOP_RECIPES_H

#include <map>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>
#include "Nutrition.h"
#include "Recipe.h"
#include "UserChoices.h"

// one row of recipes.csv: name, category text, nutrition per serving
using RecipeRow = std::tuple<std::string, std::string, Nutrition>;
// one row of recipe_ingredients.csv, without the recipe name: ingredient, amount, unit
using IngredientRow = std::tuple<std::string, double, std::string>;

class Recipes {
    std::vector<Recipe> breakfasts;
    std::vector<Recipe> lunches;
    std::vector<Recipe> dinners;

public:
    Recipes(const std::map<std::string, double>& prices,
            const std::vector<RecipeRow>& recipeRows,
            const std::map<std::string, std::vector<IngredientRow>>& ingredientsByRecipe);
    const std::vector<Recipe>& getRecipesFromCategory(Category category) const;
    std::vector<Recipe> searchUserChoices(const UserChoices& choices) const;
    double averagePrice(Category category) const;
    double averageKcal(Category category) const;
    Recipe cheapestRecipe(Category category) const;
    Recipe mostExpensiveRecipe(Category category) const;
    friend std::ostream& operator<<(std::ostream& os, const Recipes& recipes);
};

std::tuple<bool, bool, bool> reader(const std::string& input);

#endif //OOP_RECIPES_H