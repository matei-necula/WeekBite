//
// Created by Matei on 11/17/2026.
//

#include "MealBalanceConstraint.h"

bool MealBalanceConstraint::isValid_(const std::vector<DayMeals>& /*previousDays*/,
                                      const std::vector<Recipe>& todayRecipesSoFar,
                                      const Recipe& wantToAdd) const {
    const double nextRecipeCalories = wantToAdd.getNutrition().getCalories();

    for (const Recipe& meal : todayRecipesSoFar) {
        const double mealCalories = meal.getNutrition().getCalories();
        if (mealCalories > nextRecipeCalories) {
            if (mealCalories > maxRatio * nextRecipeCalories) {
                return false;
            }
        } else {
            if (nextRecipeCalories > maxRatio * mealCalories) {
                return false;
            }
        }
    }
    return true;
}

void MealBalanceConstraint::print_(std::ostream& os) const {
    os << "Balanced meals: no meal more than " << maxRatio << "x the calories of another";
}

std::unique_ptr<PlanConstraint> MealBalanceConstraint::clone() const {
    return std::make_unique<MealBalanceConstraint>(*this);
}
