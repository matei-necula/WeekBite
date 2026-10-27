//
// Created by Matei on 10/25/2026.
//

#include "Recipe.h"
#include <stdexcept>
#include <string>
#include <iomanip>

namespace {
    Category categoryFromText(const std::string &categoryText) {
        if (categoryText == "breakfast")
            return Category::breakfast;
        if (categoryText == "lunch")
            return Category::lunch;
        if (categoryText == "dinner")
            return Category::dinner;
        throw std::invalid_argument("Invalid category text: " + categoryText);
    }

    std::string categoryToText(Category category) {
        switch (category) {
            case Category::breakfast:
                return "breakfast";
            case Category::lunch:
                return "lunch";
            case Category::dinner:
                return "dinner";
        }
        throw std::invalid_argument("Invalid category");
    }
}

Recipe::Recipe(const std::string &name, const std::string &categoryText, const std::vector<Ingredient> &ingredients,
               const Nutrition &nutrition, int servings) : name(name), category(categoryFromText(categoryText)),
                                                           ingredients(ingredients), nutrition(nutrition),
                                                           servings(servings) {
}

double Recipe::recipePrice() const {
    double total = 0;
    for (const Ingredient &ing: ingredients) {
        total += ing.recipePrice();
    }
    return total;
}

std::ostream &operator<<(std::ostream &os, const Recipe &recipe) {
    os << recipe.name << " (" << categoryToText(recipe.category) << ", " << recipe.servings << " servings)\n";
    for (const Ingredient &ing: recipe.ingredients) {
        os << ing << '\n';
    }
    const std::streamsize oldPrecision = os.precision();
    os << std::setprecision(2) << std::fixed << "Total price: " << recipe.recipePrice() << " lei\n";
    os << std::defaultfloat;
    os.precision(oldPrecision);
    os << recipe.nutrition << '\n';
    // the note is printed once, at the end of the recipe, listing every unpriced ingredient
    bool found = false;
    for (const Ingredient &ing: recipe.ingredients) {
        // == 0 is safe here: missing price and tbsp/tsp both return exactly 0
        if (ing.recipePrice() == 0) {
            if (found == false) {
                os << "(price may vary because of " << ing.getName();
            } else {
                os << ", " << ing.getName();
            }
            found = true;
        }
    }
    if (found) {
        os << ")\n";
    }
    return os;
}

Recipe::Recipe(const Recipe &other)
    : name(other.name),
      category(other.category),
      ingredients(other.ingredients),
      nutrition(other.nutrition),
      servings(other.servings) {
}

Recipe &Recipe::operator=(const Recipe &other) {
    // self-assignment (r = r) has nothing to copy
    if (this == &other) {
        return *this;
    }
    name = other.name;
    category = other.category;
    ingredients = other.ingredients;
    nutrition = other.nutrition;
    servings = other.servings;
    return *this;
}

// empty: string and vector free their own memory, Recipe owns nothing else
Recipe::~Recipe() {
}

Recipe Recipe::scaled(int newServings) const {
    if (newServings <= 0) {
        throw std::invalid_argument("Invalid number of servings");
    }
    // copy first, so *this (the catalog recipe) is never changed
    Recipe scaledRecipe = *this;
    scaledRecipe.servings = newServings;
    for (Ingredient &ing: scaledRecipe.ingredients) {
        // factor = newServings works because a recipe is always written for 1 serving;
        // calling scaled() on an already scaled copy would multiply again
        ing.scale(newServings);
    }
    return scaledRecipe;
}

const Nutrition &Recipe::getNutrition() const {
    return nutrition;
}

const std::string &Recipe::getName() const {
    return name;
}
