//
// Created by Matei on 11/02/2026.
//

#include "ShoppingList.h"
#include "fileHelper.h"
#include <algorithm>   // std::find_if
#include <iomanip>     // std::setprecision
#include <numeric>     // std::accumulate

ShoppingList::ShoppingList(const std::vector<Ingredient>& allIngredients) {
    for (const Ingredient& item : allIngredients) {
        auto it = std::find_if(ingredients.begin(), ingredients.end(),
                         [&item](const Ingredient& x) { return x.getName() == item.getName(); });
        if (it != ingredients.end()) {
            *it += item;
        } else {
            ingredients.push_back(item);
        }
    }
    total = std::accumulate(ingredients.begin(), ingredients.end(), 0.0,
                [](double sum, const Ingredient& x) { return sum + x.recipePrice(); });
}

std::ostream& operator<<(std::ostream& os, const ShoppingList& list) {
    // ingredients first: Ingredient's operator<< formats its own price
    for (const Ingredient& ingredient : list.ingredients) {
        os << ingredient << '\n';
    }
    // only the total gets 2 decimals, then the stream goes back to how the caller had it
    const std::streamsize oldPrecision = os.precision();
    os << "Total: " << std::fixed << std::setprecision(2) << list.total << " lei" << '\n';
    os << std::defaultfloat;
    os.precision(oldPrecision);
    return os;
}

void ShoppingList::exportList(const std::string& fileName) const {
    std::ofstream file = openFile(fileName);
    file << *this;
}
