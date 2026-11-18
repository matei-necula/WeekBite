//
// Created by Matei on 11/17/2026.
//

#include "NutritionTargetConstraint.h"

NutritionTargetConstraint::NutritionTargetConstraint(const Nutrition& minim, const Nutrition& maxim)
    : minim(minim), maxim(maxim) {
}

bool NutritionTargetConstraint::isValid_(const std::vector<DayMeals> & /*previousDays*/,
                                         const std::vector<Recipe>& todayRecipesSoFar,
                                         const Recipe& wantToAdd) const {
    constexpr std::size_t mealsPerDay = 3;

    Nutrition dayTotal = wantToAdd.getNutrition();
    for (const Recipe& meal: todayRecipesSoFar) {
        dayTotal += meal.getNutrition();
    }
    if (!dayTotal.fitsUnder(maxim)) {
        return false;
    }
    if (todayRecipesSoFar.size() + 1 == mealsPerDay && !dayTotal.reaches(minim)) {
        return false;
    }
    return true;
}

void NutritionTargetConstraint::print_(std::ostream& os) const {
    os << "Daily nutrition: ";
    os << "minimum: ";
    minim.printNonZero(os);
    os << "; ";
    os << "maximum: ";
    maxim.printNonZero(os);
}

std::unique_ptr<PlanConstraint> NutritionTargetConstraint::clone() const {
    return std::make_unique<NutritionTargetConstraint>(minim, maxim);
}
