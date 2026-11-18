//
// Created by Matei on 11/18/2026.
//

#include "DietRestrictionConstraint.h"
#include "Errors.h"
#include <stdexcept>
#include "textHelper.h"

Diet dietFromText(const std::string& text) {
    const std::string lower = toLowercase(text);
     if (lower == "vegan")
        return Diet::vegan;
     if (lower == "vegetarian")
        return Diet::vegetarian;
     if (lower == "glutenfree")
        return Diet::glutenFree;
     if (lower == "lactosefree")
        return Diet::lactoseFree;
     if (lower == "nopork")
        return Diet::noPork;
     if (lower == "noredmeat")
        return Diet::noRedMeat;
     throw InputError("Unknown diet: " + text);
}

std::string dietToText(Diet diet) {
    switch (diet) {
        case Diet::vegan:
            return "vegan";
        case Diet::vegetarian:
            return "vegetarian";
        case Diet::glutenFree:
            return "gluten-free";
        case Diet::lactoseFree:
            return "lactose-free";
        case Diet::noPork:
            return "no pork";
        case Diet::noRedMeat:
            return "no red meat";
    }
    throw InputError("Unknown diet");
}

DietRestrictionConstraint::DietRestrictionConstraint(Diet diet)
    : diet(diet) {
}

bool DietRestrictionConstraint::isValid_(const std::vector<DayMeals>& /*previousDays*/,
                                         const std::vector<Recipe>& /*todayRecipesSoFar*/,
                                         const Recipe& wantToAdd) const {
    return allows(wantToAdd);
}

bool DietRestrictionConstraint::allows(const Recipe& wantToAdd) const {
    switch (diet) {
        case Diet::vegan:
            return wantToAdd.allIngredientsSuitableFor("vegan");
        case Diet::vegetarian:
            return wantToAdd.allIngredientsSuitableFor("vegetarian");
        case Diet::glutenFree:
            return !wantToAdd.containsAny("gluten");
        case Diet::lactoseFree:
            return !wantToAdd.containsAny("lactose");
        case Diet::noPork:
            return !wantToAdd.containsAny("pork");
        case Diet::noRedMeat:
            return !wantToAdd.containsAny("red meat");
    }
    throw InputError("Unknown diet");
}

void DietRestrictionConstraint::print_(std::ostream& os) const {
    os << "Diet: " << dietToText(diet);
}

std::unique_ptr<PlanConstraint> DietRestrictionConstraint::clone() const {
    return std::make_unique<DietRestrictionConstraint>(*this);
}
