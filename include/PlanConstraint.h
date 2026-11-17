//
// Created by Matei on 11/17/2026.
//

#ifndef OOP_PLANCONSTRAINT_H
#define OOP_PLANCONSTRAINT_H

#include <vector>
#include "DayMeals.h"
#include "Recipe.h"
#include <memory>

class PlanConstraint {
    virtual bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                          const Recipe& wantToAdd) const = 0;

public:
    bool isValid(const std::vector<DayMeals>& previousDays,
                 const std::vector<Recipe>& todayRecipesSoFar,
                 const Recipe& wantToAdd) const;

    virtual ~PlanConstraint() = default;

    virtual std::unique_ptr<PlanConstraint> clone() const = 0;
};

#endif //OOP_PLANCONSTRAINT_H
