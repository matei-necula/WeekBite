//
// Created by Matei on 11/02/2026.
//

#include "ShoppingList.h"
#include "fileHelper.h"
#include <algorithm>   // std::find_if
#include <iomanip>     // std::setprecision
#include <numeric>     // std::accumulate
#include <sstream>
#include <stdexcept>

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
    computeTotal();
}

void ShoppingList::computeTotal() {
    total = std::accumulate(ingredients.begin(), ingredients.end(), 0.0,
                [](double sum, const Ingredient& x) { return sum + x.recipePrice(); });
}

void ShoppingList::subtractPantry(const std::vector<Ingredient>& pantry) {
    std::vector<Ingredient> left = ingredients;
    for (const Ingredient& item : pantry) {
        auto it = std::find_if(left.begin(), left.end(),
                         [&item](const Ingredient& x) { return x.getName() == item.getName(); });
        if (it != left.end()) {
            *it -= item;
        }
    }
    left.erase(std::remove_if(left.begin(), left.end(),
                   [](const Ingredient& x) { return x.isEmpty(); }), left.end());
    ingredients = left;
    computeTotal();
}

void ShoppingList::sortBy(ListSort sortChoice) {
    if (sortChoice == ListSort::price) {
        std::stable_sort(ingredients.begin(), ingredients.end(), [](const Ingredient& a, const Ingredient& b) {
            return a.recipePrice() > b.recipePrice();
        });
    } else {
        std::stable_sort(ingredients.begin(), ingredients.end(), [](const Ingredient& a, const Ingredient& b) {
            return a.getName() < b.getName();
        });
    }
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

std::vector<Ingredient> pantryReader(const std::string& input) {
    std::vector<Ingredient> pantry;
    std::stringstream stream(input);
    std::string piece;
    while (std::getline(stream, piece, ',')) {
        std::stringstream pieceStream(piece);
        std::string name;
        double amount = 0;
        std::string unit;
        std::string extra;
        if (!(pieceStream >> name >> amount >> unit) || pieceStream >> extra) {
            throw std::invalid_argument("Invalid item: " + piece);
        }
        if (amount <= 0) {
            throw std::invalid_argument("Invalid amount: " + piece);
        }
        pantry.push_back(Ingredient(0, name, unit, amount));
    }
    if (pantry.empty()) {
        throw std::invalid_argument("No items provided");
    }
    return pantry;
}
