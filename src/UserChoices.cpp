//
// Created by Matei on 11/01/2026.
//

#include "UserChoices.h"

#include <cctype>      // std::tolower
#include <stdexcept>   // std::invalid_argument
#include <string>

namespace {
    // only this file can see it (same as categoryFromText in Recipe.cpp)
    std::string toLowercase(const std::string &text) {
        std::string copy = text;
        for (char& c : copy) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return copy;
    }
}

UserChoices::UserChoices(bool categoryPicked, bool namePicked, bool ingredientPicked, Category category,
                         const std::string &nameText, const std::string &ingredientText,
                         SortChoice sortChoice) : categoryPicked(categoryPicked), namePicked(namePicked),
                                                  ingredientPicked(ingredientPicked), category(category),
                                                  nameText(toLowercase(nameText)),
                                                  ingredientText(toLowercase(ingredientText)), sortChoice(sortChoice) {
    if (namePicked && nameText.empty()) {
        throw std::invalid_argument("Type the name");
    }
    if (ingredientPicked && ingredientText.empty()) {
        throw std::invalid_argument("Type the ingredient");
    }
}

bool UserChoices::matches(const Recipe &recipe) const {
    if (categoryPicked && !recipe.questionCategory(category)) return false;
    if (namePicked && !recipe.questionName(nameText)) return false;
    if (ingredientPicked && !recipe.questionIngredient(ingredientText)) return false;
    return true;
}

bool UserChoices::comesBefore(const Recipe &recipeA, const Recipe &recipeB) const { // we use stable sort but it doesn't know what to sort
    if (sortChoice == SortChoice::calories) {
        if (recipeA.getNutrition().getCalories() < recipeB.getNutrition().getCalories()) return true;
    }
    else {
        if (recipeA.recipePrice() < recipeB.recipePrice()) return true;
    }
    return false;
}


