//
// Created by Matei on 11/17/2026.
//

#include "PlanConstraint.h"

bool PlanConstraint::isValid(const std::vector<DayMeals>& previousDays,
                              const std::vector<Recipe>& todayRecipesSoFar,
                              const Recipe& wantToAdd) const {
    return isValid_(previousDays, todayRecipesSoFar, wantToAdd);
}

std::ostream& operator<<(std::ostream& os, const PlanConstraint& constraint) {
    constraint.print_(os);
    return os;
}
