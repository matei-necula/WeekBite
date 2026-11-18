//
// Created by Matei on 11/17/2026.
//

#ifndef OOP_NUTRITIONTARGETCONSTRAINT_H
#define OOP_NUTRITIONTARGETCONSTRAINT_H

#include <memory>
#include "PlanConstraint.h"
#include "Nutrition.h"

class NutritionTargetConstraint : public PlanConstraint {
    Nutrition minim; // per day; 0 = no minimum
    Nutrition maxim; // per day; 0 = no limit

    bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                  const Recipe& wantToAdd) const override;
    void print_(std::ostream& os) const override;

public:
    NutritionTargetConstraint(const Nutrition& minima, const Nutrition& maxima);

    std::unique_ptr<PlanConstraint> clone() const override;
};

#endif //OOP_NUTRITIONTARGETCONSTRAINT_H
