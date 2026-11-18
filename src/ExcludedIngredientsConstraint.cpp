//
// Created by Matei on 11/18/2026.
//

#include "ExcludedIngredientsConstraint.h"
#include "Recipe.h"

ExcludedIngredientsConstraint::ExcludedIngredientsConstraint(const std::vector<std::string>& excluded)
    : PlanConstraint("Excluded ingredients"), excluded(excluded) {
}

bool ExcludedIngredientsConstraint::isValid_(const std::vector<DayMeals>& /*previousDays*/,
                                             const std::vector<Recipe>& /*todayRecipesSoFar*/,
                                             const Recipe& wantToAdd) const {
    for (const auto& ingredient : excluded)
        if (wantToAdd.questionIngredient(ingredient))
            return false;
    return true;
}

void ExcludedIngredientsConstraint::print_(std::ostream& os) const {
    for (size_t i = 0; i < excluded.size(); ++i) {
        os << excluded[i];
        if (i != excluded.size() - 1)
            os << ", ";
    }
}

std::unique_ptr<PlanConstraint> ExcludedIngredientsConstraint::clone() const {
    return std::make_unique<ExcludedIngredientsConstraint>(excluded);
}
