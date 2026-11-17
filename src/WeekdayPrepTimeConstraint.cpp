//
// Created by Matei on 11/17/2026.
//

#include "WeekdayPrepTimeConstraint.h"

WeekdayPrepTimeConstraint::WeekdayPrepTimeConstraint(int maxMinutes, bool weekdaysOnly)
    : maxMinutes(maxMinutes), weekdaysOnly(weekdaysOnly) {
}

bool WeekdayPrepTimeConstraint::isValid_(const std::vector<DayMeals>& previousDays,
                                         const std::vector<Recipe>& /*todayRecipesSoFar*/,
                                         const Recipe& wantToAdd) const {
    constexpr std::size_t weekdaysPerWeek = 5; // days 0-4 = Monday-Friday
    const std::size_t todayIndex = previousDays.size();
    if (weekdaysOnly && todayIndex >= weekdaysPerWeek) {
        return true;
    }
    return wantToAdd.takesAtMost(maxMinutes);
}

std::unique_ptr<PlanConstraint> WeekdayPrepTimeConstraint::clone() const {
    return std::make_unique<WeekdayPrepTimeConstraint>(*this);
}
