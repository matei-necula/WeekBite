//
// Created by Matei on 11/02/2026.
//

#ifndef OOP_SHOPPINGLIST_H
#define OOP_SHOPPINGLIST_H

#include "Ingredient.h"
#include <vector>

class ShoppingList {
    double total = 0;
    std::vector<Ingredient> ingredients;

public:
    explicit ShoppingList(const std::vector<Ingredient>& allIngredients);
    void exportList(const std::string& fileName) const;
    friend std::ostream& operator<<(std::ostream& os, const ShoppingList& list);
};

#endif //OOP_SHOPPINGLIST_H
