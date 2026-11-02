//
// Created by Matei on 11/02/2026.
//

#ifndef OOP_SHOPPINGLIST_H
#define OOP_SHOPPINGLIST_H

#include "Ingredient.h"
#include <string>
#include <vector>

enum class ListSort {
    price,
    name
};

class ShoppingList {
    double total = 0;
    std::vector<Ingredient> ingredients;

    void computeTotal();

public:
    explicit ShoppingList(const std::vector<Ingredient>& allIngredients);
    void subtractPantry(const std::vector<Ingredient>& pantry);
    void sortBy(ListSort sortChoice);
    void exportList(const std::string& fileName) const;
    friend std::ostream& operator<<(std::ostream& os, const ShoppingList& list);
};

std::vector<Ingredient> pantryReader(const std::string& input);

#endif //OOP_SHOPPINGLIST_H
