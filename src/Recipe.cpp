//
// Created by Matei on 10/25/2026.
//

#include "Recipe.h"
#include "Errors.h"
#include <string>
#include <iomanip>
#include "fileHelper.h"

namespace {
    Category categoryFromText(const std::string& categoryText) {
        if (categoryText == "breakfast")
            return Category::breakfast;
        if (categoryText == "lunch")
            return Category::lunch;
        if (categoryText == "dinner")
            return Category::dinner;
        throw InputError("Invalid category text: " + categoryText);
    }
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
    throw InputError("Invalid category");
}

Recipe::Recipe(const std::string& name, const std::string& categoryText, const std::vector<Ingredient>& ingredients,
               const Nutrition& nutrition, int servings, int prepTime) : name(name), category(categoryFromText(categoryText)),
                                                           ingredients(ingredients), nutrition(nutrition),
                                                           servings(servings), prepMinutes(prepTime) {
}

double Recipe::recipePrice() const {
    double total = 0;
    for (const Ingredient& ing: ingredients) {
        total += ing.recipePrice();
    }
    return total;
}

bool Recipe::takesAtMost(int minutes) const {
    return prepMinutes <= minutes;
}

std::ostream& operator<<(std::ostream& os, const Recipe& recipe) {
    os << recipe.name << " (" << categoryToText(recipe.category) << ", " << recipe.servings << " servings)\n";
    for (const Ingredient& ing: recipe.ingredients) {
        os << ing << '\n';
    }
    const std::streamsize oldPrecision = os.precision();
    os << std::setprecision(2) << std::fixed << "Total price: " << recipe.recipePrice() << " lei\n";
    os << std::defaultfloat;
    os.precision(oldPrecision);
    os << recipe.nutrition << '\n';
    // the note is printed once, at the end of the recipe, listing every unpriced ingredient
    bool found = false;
    for (const Ingredient& ing: recipe.ingredients) {
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

Recipe::Recipe(const Recipe& other)
    : name(other.name),
      category(other.category),
      ingredients(other.ingredients),
      nutrition(other.nutrition),
      servings(other.servings),
      prepMinutes(other.prepMinutes) {
}

Recipe& Recipe::operator=(const Recipe& other) {
    // self-assignment (r = r) has nothing to copy
    if (this == &other) {
        return *this;
    }
    name = other.name;
    category = other.category;
    ingredients = other.ingredients;
    nutrition = other.nutrition;
    servings = other.servings;
    prepMinutes = other.prepMinutes;
    return *this;
}

// empty: string and vector free their own memory, Recipe owns nothing else
Recipe::~Recipe() {
}

Recipe Recipe::scaled(int newServings) const {
    if (newServings <= 0) {
        throw AmountError("Invalid number of servings");
    }
    // copy first, so *this (the catalog recipe) is never changed
    Recipe scaledRecipe = *this;
    scaledRecipe.servings = newServings;
    for (Ingredient& ing: scaledRecipe.ingredients) {
        // factor = newServings works because a recipe is always written for 1 serving;
        // calling scaled() on an already scaled copy would multiply again
        ing.scale(newServings);
    }
    return scaledRecipe;
}

const Nutrition& Recipe::getNutrition() const {
    return nutrition;
}

const std::string& Recipe::getName() const {
    return name;
}

bool Recipe::questionCategory(Category searchedCategory) const {
    return (category == searchedCategory);
    }

bool Recipe::questionName(const std::string& text) const {
    return (this->name.find(text) != std::string::npos);
    // return true if this recipe's name member contains it}
} 
bool Recipe::questionIngredient(const std::string& ingredient) const {
    for (const Ingredient& ing : ingredients) {
        if (ing.getName().find(ingredient) != std::string::npos)
            return true;
    }
    return false;
}

void Recipe::printShort(std::ostream& os) const {
    const std::streamsize oldPrecision = os.precision();
    os << std::setprecision(2) << std::fixed << "Name: " << name << ", kcal: " << nutrition.getCalories() << ", price: " << recipePrice() << " lei\n";
    os << std::defaultfloat;
    os.precision(oldPrecision);
    }

void Recipe::exportRecipe(const std::string& fileName) const {
    std::ofstream file = openFile(fileName);
    file << *this;
}

void Recipe::exportRecipeAndScaled(const std::string& fileName, const Recipe& scaledRecipe) const {
    std::ofstream file = openFile(fileName);
    file << *this << "\n\nScaled recipe:\n" << scaledRecipe;
}

const std::vector<Ingredient>& Recipe::giveIngredients() const {
    return ingredients;
}

bool Recipe::allIngredientsSuitableFor(const std::string& diet) const {
    for (const Ingredient& ing : ingredients) {
        if (!ing.isSuitableFor(diet))
            return false;
    }
    return true;
}

bool Recipe::containsAny(const std::string& tag) const {
    for (const Ingredient& ing : ingredients) {
        if (ing.hasTag(tag))
            return true;
    }
    return false;
}