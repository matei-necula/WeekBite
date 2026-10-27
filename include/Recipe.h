//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_RECIPE_H
#define OOP_RECIPE_H

#include <ostream>
#include <string>
#include <vector>
#include <Ingredient.h>
#include <Nutrition.h>

// same approach as Unit: fixed set of values, converted from text in the constructor
enum class Category {
    breakfast,
    lunch,
    dinner
};

class Recipe {
    std::string name;
    Category category;
    std::vector<Ingredient> ingredients; // Recipe owns its own copies (composition)
    Nutrition nutrition; // per serving, so it does not change when the recipe is scaled
    int servings; // 1 in recipes.csv; only a scaled() copy has more
public:
    // const& kept on purpose (clang-tidy suggests pass by value + std::move)
    Recipe(const std::string& name, const std::string& categoryText, const std::vector<Ingredient>& ingredients, const Nutrition& nutrition, int servings);
    // Tema 1 asks for these on one class; written by hand even though the generated ones would do the same
    Recipe(const Recipe &other);
    Recipe & operator=(const Recipe &other);
    ~Recipe();
    double recipePrice() const;
    // returns a scaled copy so the recipe in the catalog stays unchanged
    Recipe scaled(int newServings) const;
    friend std::ostream& operator<<(std::ostream& os, const Recipe& recipe);
};



#endif //OOP_RECIPE_H
