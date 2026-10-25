//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_INGREDIENT_H
#define OOP_INGREDIENT_H
#include <string>
#include <ostream>

enum class Unit { g, ml, tbsp, tsp, pieces };

class Ingredient {
    double ingredientPrice;
    std::string name;
    Unit unit;
    double amount;

public:
    Ingredient(double price, const std::string &name, const std::string& unitText, double amount);
    double recipePrice() const;
    friend std::ostream& operator<<(std::ostream& os, const Ingredient& ingredient);
};

#endif //OOP_INGREDIENT_H