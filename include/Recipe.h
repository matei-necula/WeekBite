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

enum class Category {
    breakfast,
    lunch,
    dinner
};

class Recipe {
    std::string name;
    Category category;
    std::vector<Ingredient> ingredients;
    Nutrition nutrition;
    int servings;
public:
    Recipe(const std::string& name, const std::string& categoryText, const std::vector<Ingredient>& ingredients, const Nutrition& nutrition, int servings);
    Recipe(const Recipe &other);
    Recipe & operator=(const Recipe &other);
    ~Recipe();
    double recipePrice() const;
    Recipe scaled(int newServings) const;
    friend std::ostream& operator<<(std::ostream& os, const Recipe& recipe);
};



#endif //OOP_RECIPE_H
