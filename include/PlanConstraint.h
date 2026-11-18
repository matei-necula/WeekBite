//
// Created by Matei on 11/17/2026.
//

#ifndef OOP_PLANCONSTRAINT_H
#define OOP_PLANCONSTRAINT_H

#include <vector>
#include "DayMeals.h"
#include "Recipe.h"
#include <memory>
#include <ostream>
#include <string>

class PlanConstraint {
    std::string name; // what the rule is about, printed before ':' (e.g. "Diet")

    virtual bool isValid_(const std::vector<DayMeals>& previousDays, const std::vector<Recipe>& todayRecipesSoFar,
                          const Recipe& wantToAdd) const = 0;

    //writes this rule's details in one line, without '\n' (e.g. "vegan"; operator<< writes "Diet: " before it);
    // called only by operator<< below (NVI, like isValid -> isValid_)
    virtual void print_(std::ostream& os) const = 0;

    protected:
    // only the derived rules build a PlanConstraint, so the constructor is protected.
    explicit PlanConstraint(const std::string& name);

public:
    bool isValid(const std::vector<DayMeals>& previousDays,
                 const std::vector<Recipe>& todayRecipesSoFar,
                 const Recipe& wantToAdd) const;

    virtual ~PlanConstraint() = default;

    virtual std::unique_ptr<PlanConstraint> clone() const = 0;

    // the one way to print any constraint; the right print_ is picked at run time
    friend std::ostream& operator<<(std::ostream& os, const PlanConstraint& constraint);
};

#endif //OOP_PLANCONSTRAINT_H
