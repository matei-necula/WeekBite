//
// Created by Matei on 10/27/2026.
//

#include "MealPlan.h"
#include <algorithm>
#include <cmath>
#include <iomanip>

MealPlan::MealPlan(double weeklyBudget, double caloriesPerDay, const Recipes &catalog)
    : weeklyBudget(weeklyBudget), caloriesPerDay(caloriesPerDay) {
    // days starts empty; the greedy fills it
    processPlan(catalog);
}

void MealPlan::processPlan(const Recipes &catalog) {
    double dailyBudget = weeklyBudget / 7;
    std::vector<Recipe> copyBreakfast = catalog.getRecipesFromCategory(Category::breakfast);
    std::vector<Recipe> copyLunch = catalog.getRecipesFromCategory(Category::lunch);
    std::vector<Recipe> copyDinner = catalog.getRecipesFromCategory(Category::dinner);
    std::sort(copyLunch.begin(), copyLunch.end(), [](const Recipe &a, const Recipe &b) {
        return a.getNutrition().getCalories() > b.getNutrition().getCalories();
    });

    for (int day = 0; day < 7; day++) {
        bool dayFound = false;
        for (int i = 0; i < static_cast<int>(copyLunch.size()); i++) {
            const Recipe &lunch = copyLunch[i];
            if (lunch.recipePrice() > dailyBudget)
                continue;
            else {
                double half = (caloriesPerDay - lunch.getNutrition().getCalories()) / 2;
                double moneyForBreakfast = (dailyBudget - lunch.recipePrice()) / 2;
                double moneyForDinner = moneyForBreakfast;
                int bestBreakfast = -1;
                double bestDistanceBreakfast = 0;
                for (int j = 0; j < static_cast<int>(copyBreakfast.size()); j++) {
                    if (copyBreakfast[j].recipePrice() > moneyForBreakfast) {
                        continue;
                    }
                    double distance = std::abs(copyBreakfast[j].getNutrition().getCalories() - half);
                    if (bestBreakfast == -1 || distance < bestDistanceBreakfast) {
                        bestBreakfast = j;
                        bestDistanceBreakfast = distance;
                    }
                }
                if (bestBreakfast == -1) {
                    continue; // could not find a suitable breakfast , goes to next lunch
                }
                int bestDinner = -1; // same algorithm as breakfast but for dinner
                double bestDistanceDinner = 0;
                for (int j = 0; j < static_cast<int>(copyDinner.size()); j++) {
                    if (copyDinner[j].recipePrice() > moneyForDinner) {
                        continue;
                    }
                    double distance = std::abs(copyDinner[j].getNutrition().getCalories() - half);
                    if (bestDinner == -1 || distance < bestDistanceDinner) {
                        bestDinner = j;
                        bestDistanceDinner = distance;
                    }
                }
                if (bestDinner == -1) {
                    continue; // could not find a suitable dinner , goes to next lunch
                } else {
                    days.push_back(DayMeals(copyBreakfast[bestBreakfast], lunch, copyDinner[bestDinner]));
                    copyDinner.erase(copyDinner.begin() + bestDinner);
                    copyLunch.erase(copyLunch.begin() + i);
                    copyBreakfast.erase(copyBreakfast.begin() + bestBreakfast);
                    dayFound = true;
                    break;
                }
            }
        }
        if (!dayFound) {
            return;
        }
    }
}

bool MealPlan::weekCompleted() const {
    return (days.size() == 7);
}

// TODO: scaled: ask each DayMeals for its scaled copy (round 42)

std::ostream &operator<<(std::ostream &os, const MealPlan &plan) {
    if (!plan.weekCompleted()) {
        os << "cannot compute a meal plan , please change the budget/calorie target\n";
        return os;
    } else {
        double totalMoneySpent = 0;
        for (int i = 0; i < static_cast<int>(plan.days.size()); i++) {
            totalMoneySpent += plan.days[i].dayPrice();
            os << "Day " << i + 1 << "\n";
            os << plan.days[i];
            os << "\n";
        }
        const std::streamsize oldPrecision = os.precision();
        os << std::setprecision(2) << std::fixed << "Total money spent: " << totalMoneySpent << " lei";
        os << std::defaultfloat;
        os.precision(oldPrecision);
        os << "\n";
    }

    return os;
}
