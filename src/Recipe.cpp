//
// Created by Matei on 10/25/2026.
//

#include "Recipe.h"
#include <stdexcept>
#include <string>
#include <iomanip>

namespace {
    Category categoryFromText(const std::string& categoryText) {
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

Recipe::Recipe(const std::string& name, const std::string& categoryText, const std::vector<Ingredient>& ingredients, const Nutrition& nutrition, int servings) : name(name), category(categoryFromText(categoryText)), ingredients(ingredients), nutrition(nutrition), servings(servings) {}

double Recipe::recipePrice() const {
    double total = 0;
    for (const Ingredient& ing : ingredients) {
        total += ing.recipePrice();
    }
    return total;
}

std::ostream& operator<<(std::ostream& os, const Recipe& recipe) {
    os << recipe.name << " (" << categoryToText(recipe.category) << ", " << recipe.servings << " servings)\n";
    for (const Ingredient& ing : recipe.ingredients) {
        os << ing << '\n';
    }
    const std::streamsize oldPrecision = os.precision();
    os << std::setprecision(2) << std::fixed << "Total price: " << recipe.recipePrice() << " lei\n";
    os << std::defaultfloat;
    os.precision(oldPrecision);
    os << recipe.nutrition << '\n';
    bool found = false;
    for (const Ingredient& ing : recipe.ingredients) {
        if (ing.recipePrice() == 0) {
            if (found==false) {
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
