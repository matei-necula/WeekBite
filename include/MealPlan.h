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
#include <memory>
#include "PlanConstraint.h"

class MealPlan {
    std::vector<DayMeals> days; // always 7 , constructor throws PlanImpossibleError otherwise
    double weeklyBudget;
    double caloriesPerDay;

    // the greedy; private, only the constructor calls it
    // true when all 7 days were filled; useConstraints = false ignores the constraints
    bool processPlan(const Recipes& catalog, bool useConstraints);

    std::vector<std::unique_ptr<PlanConstraint>> constraints;

    bool passesConstraints(const std::vector<Recipe>& todayRecipesSoFar, const Recipe& candidate) const;

    // which meal the diet rules leave short of 7 different recipes, e.g. "only 3 breakfast recipes fit your diets";
    // "" when there are no diet rules or every meal has enough
    std::string dietShortage(const Recipes& catalog) const;

public:
    MealPlan(double weeklyBudget, double caloriesPerDay, const Recipes& catalog,
             std::vector<std::unique_ptr<PlanConstraint>> constraints = {});

    MealPlan(const MealPlan& other);

    MealPlan& operator=(MealPlan other);

    friend void swap(MealPlan& first, MealPlan& second) noexcept;

    MealPlan scaled(int newServings) const;

    Nutrition weekNutritionTotal() const;

    double averageKcalPerDay() const;

    double budgetLeft() const;

    int cheapestDay() const;

    int mostExpensiveDay() const;

    void exportPlan(const std::string& fileName) const; // plan only

    void exportPlanAndScaled(const std::string& fileName, const MealPlan& scaledPlan) const; // plan + scaled copy

    std::vector<Ingredient> giveIngredients() const;

    friend std::ostream& operator<<(std::ostream& os, const MealPlan& plan);
};

#endif //OOP_MEALPLAN_H
