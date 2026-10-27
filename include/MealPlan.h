//
// Created by Matei on 10/27/2026.
//

#ifndef OOP_MEALPLAN_H
#define OOP_MEALPLAN_H

#include <ostream>
#include <vector>
#include "DayMeals.h"
#include "Recipes.h"

class MealPlan {
    std::vector<DayMeals> days; // 7 when the plan worked, fewer when it failed
    double weeklyBudget;
    double caloriesPerDay;

    // the greedy; private, only the constructor calls it
    void processPlan(const Recipes& catalog);

public:
    MealPlan(double weeklyBudget, double caloriesPerDay, const Recipes& catalog);

    bool weekCompleted() const;

    // TODO: scaled (compare with Recipe::scaled)

    friend std::ostream& operator<<(std::ostream& os, const MealPlan& plan);
};

#endif //OOP_MEALPLAN_H
