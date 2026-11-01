//
// Created by Matei on 11/01/2026.
//

#ifndef OOP_USERCHOICES_H
#define OOP_USERCHOICES_H

#include <string>
#include "Recipe.h"

// the user's sort choice for the search results
enum class SortChoice {
    calories,
    price
};

class UserChoices {
    bool categoryPicked;
    bool namePicked;
    bool ingredientPicked;

    Category category;
    std::string nameText;
    std::string ingredientText;

    SortChoice sortChoice;

public:
    UserChoices(bool categoryPicked, bool namePicked, bool ingredientPicked,
                Category category, const std::string &nameText, const std::string &ingredientText,
                SortChoice sortChoice);

    bool matches(const Recipe &recipe) const;

    bool comesBefore(const Recipe &recipeA, const Recipe &recipeB) const;
};

#endif //OOP_USERCHOICES_H
