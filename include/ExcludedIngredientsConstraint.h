//
// Created by Matei on 11/18/2026.
//

#ifndef OOP_EXCLUDEDINGREDIENTSCONSTRAINT_H
#define OOP_EXCLUDEDINGREDIENTSCONSTRAINT_H

#include <memory>
#include <string>
#include <vector>
#include "PlanConstraint.h"

// keeps out every recipe that contains one of the ingredients the user doesn't want (e.g. "mushroom", "tuna")
class ExcludedIngredientsConstraint : public PlanConstraint {
    std::vector<std::string> excluded; // lowercase ingredient names

    bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                  const Recipe& wantToAdd) const override;
    void print_(std::ostream& os) const override;

public:
    explicit ExcludedIngredientsConstraint(const std::vector<std::string>& excluded);

    std::unique_ptr<PlanConstraint> clone() const override;
};

#endif //OOP_EXCLUDEDINGREDIENTSCONSTRAINT_H
