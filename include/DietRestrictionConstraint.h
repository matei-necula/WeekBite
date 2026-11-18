//
// Created by Matei on 11/18/2026.
//

#ifndef OOP_DIETRESTRICTIONCONSTRAINT_H
#define OOP_DIETRESTRICTIONCONSTRAINT_H

#include <memory>
#include <string>
#include "PlanConstraint.h"

// one restriction the user can pick; vegan / vegetarian need every ingredient to be suitable,
// the others reject a recipe if any ingredient contains the tag
enum class Diet { vegan, vegetarian, glutenFree, lactoseFree, noPork, noRedMeat };
// throws InputError for any other text
Diet dietFromText(const std::string& text);

class DietRestrictionConstraint : public PlanConstraint {
    Diet diet;

    bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                  const Recipe& wantToAdd) const override;

public:
    explicit DietRestrictionConstraint(Diet diet);

    std::unique_ptr<PlanConstraint> clone() const override;
};

#endif //OOP_DIETRESTRICTIONCONSTRAINT_H
