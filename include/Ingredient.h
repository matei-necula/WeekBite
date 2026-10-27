//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_INGREDIENT_H
#define OOP_INGREDIENT_H
#include <string>
#include <ostream>

// enum instead of string: a unit is always one of these 5 fixed values
enum class Unit { g, ml, tbsp, tsp, pieces };

class Ingredient {
    double ingredientPrice; // from ingredients.csv; 0 means the ingredient is missing from that file
    std::string name;
    Unit unit;
    double amount;

public:
    // takes the unit as text so the text -> Unit conversion stays inside Ingredient
    Ingredient(double price, const std::string& name, const std::string& unitText, double amount);
    double recipePrice() const;
    friend std::ostream& operator<<(std::ostream& os, const Ingredient& ingredient);
    void scale(int factor); // changes this ingredient; Recipe::scaled() calls it on its own copy
    // only getter: Recipe needs the name for the "price may vary" note (chosen over friend class Recipe)
    const std::string& getName() const;
};


#endif //OOP_INGREDIENT_H