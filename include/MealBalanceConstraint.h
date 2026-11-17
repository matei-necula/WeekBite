//
// Created by Matei on 11/17/2026.
//

#ifndef OOP_MEALBALANCECONSTRAINT_H
#define OOP_MEALBALANCECONSTRAINT_H

#include "PlanConstraint.h"
#include <memory>

class MealBalanceConstraint : public PlanConstraint {
    bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                  const Recipe& wantToAdd) const override;
public: std::unique_ptr<PlanConstraint> clone() const override;
};

#endif //OOP_MEALBALANCECONSTRAINT_H
