//
// Created by Matei on 10/27/2026.
//

#ifndef OOP_MEALPLAN_H
#define OOP_MEALPLAN_H

#include <ostream>
#include <vector>
#include "DayMeals.h"
#include "Recipes.h"
#include <string>

class MealPlan {
    std::vector<DayMeals> days; // 7 when the plan worked, fewer when it failed
    double weeklyBudget;
    double caloriesPerDay;

    // the greedy; private, only the constructor calls it
    void processPlan(const Recipes& catalog);

public:
    MealPlan(double weeklyBudget, double caloriesPerDay, const Recipes& catalog);

    bool weekCompleted() const;

    MealPlan scaled(int newServings) const;

    void exportPlan(const std::string& fileName) const;// plan only

    void exportPlanAndScaled(const std::string& fileName, const MealPlan& scaledPlan) const;  // plan + scaled copy

    friend std::ostream& operator<<(std::ostream& os, const MealPlan& plan);
};

#endif //OOP_MEALPLAN_H
