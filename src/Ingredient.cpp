//
// Created by Matei on 10/25/2026.
//

#include "Ingredient.h"
#include <stdexcept>
#include <iomanip>

namespace {
    Unit unitFromText(const std::string& unitText) {
        if (unitText == "g")
            return Unit::g;
        if (unitText == "ml")
            return Unit::ml;
        if (unitText == "tbsp")
            return Unit::tbsp;
        if (unitText == "tsp")
            return Unit::tsp;
        if (unitText == "pieces")
            return Unit::pieces;
        throw std::invalid_argument("Invalid unit text: " + unitText);
    }

    std::string unitToText(Unit unit) {
        switch (unit) {
            case Unit::g:
                return "g";
            case Unit::ml:
                return "ml";
            case Unit::tbsp:
                return "tbsp";
            case Unit::tsp:
                return "tsp";
            case Unit::pieces:
                return "pieces";
        }
        throw std::invalid_argument("Invalid unit");
    }
}

Ingredient::Ingredient(double price, const std::string &name, const std::string& unitText, double amount)
        : ingredientPrice(price),
        name(name),
        unit(unitFromText(unitText)),
        amount(amount) {
}

double Ingredient::recipePrice() const {
    switch (unit) {
        case Unit::g:
        case Unit::ml:
            return (ingredientPrice/100) * amount;
        case Unit::tbsp:
        case Unit::tsp:
            return 0;
        case Unit::pieces:
            return ingredientPrice*amount;
    }
    throw std::invalid_argument("Invalid unit");
}

std::ostream& operator<<(std::ostream& os, const Ingredient& ingredient) {
    os << ingredient.name << " (" << ingredient.amount << " - " << unitToText(ingredient.unit) << ")";
    if (ingredient.unit != Unit::tbsp && ingredient.unit != Unit::tsp) {
        const std::streamsize oldPrecision = os.precision();
        os << " (" << std::fixed << std::setprecision(2) << ingredient.recipePrice() << " lei)";
        os << std::defaultfloat;
        os.precision(oldPrecision);
    }
    return os;
}