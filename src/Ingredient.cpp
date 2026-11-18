//
// Created by Matei on 10/25/2026.
//

#include "Ingredient.h"
#include "Errors.h"
#include <algorithm>
#include <iomanip>

namespace {
    // prices in ingredients.csv are per 100 g / 100 ml
    constexpr double priceReferenceAmount = 100;

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
        // files are assumed correct, so an unknown unit is an error, not something to skip
        throw InputError("Invalid unit text: " + unitText);
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
        // not reachable (every Unit is handled above); without it the compiler warns about a missing return
        throw UnitError("Invalid unit");
    }
}

Ingredient::Ingredient(double price, const std::string& name, const std::string& unitText, double amount,
                       const std::vector<std::string>& suitableTags, const std::vector<std::string>& containsTags)
    : ingredientPrice(price),
      name(name),
      unit(unitFromText(unitText)),
      amount(amount),
      suitableTags(suitableTags),
      containsTags(containsTags) {
}

double Ingredient::recipePrice() const {
    switch (unit) {
        case Unit::g:
        case Unit::ml:
            return (ingredientPrice / priceReferenceAmount) * amount;
        case Unit::tbsp:
        case Unit::tsp:
            // tbsp/tsp have no price; 0 makes Recipe print the "price may vary" note
            return 0;
        case Unit::pieces:
            return ingredientPrice * amount;
    }
    throw UnitError("Invalid unit");
}

std::ostream& operator<<(std::ostream& os, const Ingredient& ingredient) {
    os << ingredient.name << " (" << ingredient.amount << " - " << unitToText(ingredient.unit) << ")";
    // no price shown on tbsp/tsp lines, they are not priced
    if (ingredient.unit != Unit::tbsp && ingredient.unit != Unit::tsp) {
        // 2 decimals for lei, then put the stream back the way the caller had it
        const std::streamsize oldPrecision = os.precision();
        os << " (" << std::fixed << std::setprecision(2) << ingredient.recipePrice() << " lei)";
        os << std::defaultfloat;
        os.precision(oldPrecision);
    }
    return os;
}

void Ingredient::scale(int factor) {
    // checked before changing amount, so a bad factor leaves the ingredient untouched
    if (factor <= 0)
        throw AmountError("Invalid factor");
    amount *= factor;
}

const std::string& Ingredient::getName() const {
    return name;
}

Ingredient& Ingredient::operator+=(const Ingredient& other) {
    if (name != other.name || unit != other.unit)
        throw UnitError("Cannot add ingredients with different names or units");
    amount += other.amount;
    return *this;
}

Ingredient& Ingredient::operator-=(const Ingredient& other) {
    if (name != other.name || unit != other.unit)
        throw UnitError("Cannot subtract " + other.name + ": the unit is different");
    amount -= other.amount;
    if (amount < 0)
        amount = 0;
    return *this;
}

bool Ingredient::isEmpty() const {
    return amount <= 0;
}

bool Ingredient::isSuitableFor(const std::string& diet) const {
    if (std::find(suitableTags.begin(), suitableTags.end(), diet) != suitableTags.end())
        return true;
    return false;
}

bool Ingredient::hasTag(const std::string& tag) const {
    if (std::find(containsTags.begin(), containsTags.end(), tag) != containsTags.end())
        return true;
    return false;
}