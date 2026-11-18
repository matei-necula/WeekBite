//
// Created by Matei on 11/17/2026.
//

#ifndef OOP_WEEKDAYPREPTIMECONSTRAINT_H
#define OOP_WEEKDAYPREPTIMECONSTRAINT_H

#include <memory>
#include "PlanConstraint.h"

class WeekdayPrepTimeConstraint : public PlanConstraint {
    int maxMinutes;    // per meal
    bool weekdaysOnly; // true: Monday-Friday only; false: every day

    bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                  const Recipe& wantToAdd) const override;
    void print_(std::ostream& os) const override;

public:
    WeekdayPrepTimeConstraint(int maxMinutes, bool weekdaysOnly);

    std::unique_ptr<PlanConstraint> clone() const override;
};

#endif //OOP_WEEKDAYPREPTIMECONSTRAINT_H
