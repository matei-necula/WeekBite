//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_INGREDIENT_H
#define OOP_INGREDIENT_H

#include <vector>
#include <string>
#include <ostream>

// enum instead of string: a unit is always one of these 5 fixed values
enum class Unit { g, ml, tbsp, tsp, pieces };

class Ingredient {
    double ingredientPrice; // from ingredients.csv; 0 means the ingredient is missing from that file
    std::string name;
    Unit unit;
    double amount;
    std::vector<std::string> suitableTags; // from suitable_for: "vegan", "vegetarian"
    std::vector<std::string> containsTags; // from contains: "gluten", "lactose", "pork", "red meat"

public:
    // takes the unit as text so the text -> Unit conversion stays inside Ingredient
    Ingredient(double price, const std::string& name, const std::string& unitText,
               double amount, const std::vector<std::string>& suitableTags = {},
               const std::vector<std::string>& containsTags = {});

    double recipePrice() const;

    bool isSuitableFor(const std::string& diet) const; // is diet in suitableTags?

    bool hasTag(const std::string& tag) const; // is tag in containsTags?

    friend std::ostream& operator<<(std::ostream& os, const Ingredient& ingredient);

    void scale(int factor); // changes this ingredient; Recipe::scaled() calls it on its own copy
    // only getter: Recipe needs the name for the "price may vary" note (chosen over friend class Recipe)
    const std::string& getName() const;

    Ingredient& operator+=(const Ingredient& other);

    Ingredient& operator-=(const Ingredient& other);

    bool isEmpty() const;
};


#endif //OOP_INGREDIENT_H
