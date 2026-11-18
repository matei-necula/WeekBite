//
// Created by Matei on 10/27/2026.
//

#include "MealPlan.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "Errors.h"
#include <fstream>
#include <numeric>
#include <utility>
#include "fileHelper.h"
#include "PlanConstraint.h"

MealPlan::MealPlan(double weeklyBudget, double caloriesPerDay, const Recipes& catalog,
                   std::vector<std::unique_ptr<PlanConstraint>> constraints)
    : weeklyBudget(weeklyBudget), caloriesPerDay(caloriesPerDay), constraints(std::move(constraints)) {
    // days starts empty; the greedy fills it
    if (processPlan(catalog, true))
        return;
    days.clear();
    if (!this->constraints.empty() && processPlan(catalog, false)) {
        throw PlanImpossibleError("The restrictions leave too few recipes");
    } else {
        throw PlanImpossibleError("The budget is too small for 7 days");
    }
}

bool MealPlan::processPlan(const Recipes& catalog, bool useConstraints) {
    constexpr double lunchCalorieRatio = 0.4;
    const double targetLunchCalories = caloriesPerDay * lunchCalorieRatio;
    double dailyBudget = weeklyBudget / 7;
    std::vector<Recipe> copyBreakfast = catalog.getRecipesFromCategory(Category::breakfast);
    std::vector<Recipe> copyLunch = catalog.getRecipesFromCategory(Category::lunch);
    std::vector<Recipe> copyDinner = catalog.getRecipesFromCategory(Category::dinner);
    std::stable_sort(copyLunch.begin(), copyLunch.end(), [targetLunchCalories](const Recipe& a, const Recipe& b) {
        double distA = std::abs(a.getNutrition().getCalories() - targetLunchCalories);
        double distB = std::abs(b.getNutrition().getCalories() - targetLunchCalories);
        return distA < distB;
    });

    for (int day = 0; day < 7; day++) {
        bool dayFound = false;
        for (int i = 0; i < static_cast<int>(copyLunch.size()); i++) {
            const Recipe& lunch = copyLunch[i];
            if (lunch.recipePrice() > dailyBudget)
                continue;
            else {
                if (useConstraints && !passesConstraints({}, lunch))
                    continue;
                const std::vector<Recipe> todayAfterLunch{lunch};
                double half = (caloriesPerDay - lunch.getNutrition().getCalories()) / 2;
                double moneyForBreakfast = (dailyBudget - lunch.recipePrice()) / 2;
                double moneyForDinner = moneyForBreakfast;
                int bestBreakfast = -1;
                double bestDistanceBreakfast = 0;
                for (int j = 0; j < static_cast<int>(copyBreakfast.size()); j++) {
                    if (copyBreakfast[j].recipePrice() > moneyForBreakfast) {
                        continue;
                    }
                    if (useConstraints && !passesConstraints(todayAfterLunch, copyBreakfast[j])) {
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
                const std::vector<Recipe> todayAfterBreakfast{lunch, copyBreakfast[bestBreakfast]};
                int bestDinner = -1; // same algorithm as breakfast but for dinner
                double bestDistanceDinner = 0;
                for (int j = 0; j < static_cast<int>(copyDinner.size()); j++) {
                    if (copyDinner[j].recipePrice() > moneyForDinner) {
                        continue;
                    }
                    if (useConstraints && !passesConstraints(todayAfterBreakfast, copyDinner[j])) {
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
            return false;
        }
    }
    return true;
}

MealPlan MealPlan::scaled(int newServings) const {
    if (newServings <= 0) {
        throw AmountError("Invalid number of servings");
    }
    MealPlan scaledPlan = *this;
    scaledPlan.days.clear();
    for (const auto& day: days) {
        scaledPlan.days.push_back(day.scaled(newServings));
    }
    return scaledPlan;
}

std::ostream& operator<<(std::ostream& os, const MealPlan& plan) {
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
    return os;
}

void MealPlan::exportPlan(const std::string& fileName) const {
    std::ofstream file = openFile(fileName);
    file << *this;
}

void MealPlan::exportPlanAndScaled(const std::string& fileName, const MealPlan& scaledPlan) const {
    std::ofstream file = openFile(fileName);
    file << *this << "\n\nScaled plan:\n" << scaledPlan;
}

Nutrition MealPlan::weekNutritionTotal() const {
    Nutrition total{0.0, 0.0, 0.0, 0.0, 0.0};
    for (const auto& day: days) {
        total += day.dayNutrition();
    }
    return total;
}

double MealPlan::averageKcalPerDay() const {
    return weekNutritionTotal().getCalories() / static_cast<double>(days.size());
}

double MealPlan::budgetLeft() const {
    double spent = std::accumulate(days.begin(), days.end(), 0.0, [](double sum, const DayMeals& day) {
        return sum + day.dayPrice();
    });
    return weeklyBudget - spent;
}

int MealPlan::cheapestDay() const {
    auto it = std::min_element(days.begin(), days.end(), [](const DayMeals& a, const DayMeals& b) {
        return a.dayPrice() < b.dayPrice();
    });
    return static_cast<int>(it - days.begin()) + 1;
}

int MealPlan::mostExpensiveDay() const {
    auto it = std::max_element(days.begin(), days.end(), [](const DayMeals& a, const DayMeals& b) {
        return a.dayPrice() < b.dayPrice();
    });
    return static_cast<int>(it - days.begin()) + 1;
}

std::vector<Ingredient> MealPlan::giveIngredients() const {
    std::vector<Ingredient> result;
    for (const DayMeals& day: days) {
        const std::vector<Ingredient> dayList = day.giveIngredients();
        result.insert(result.end(), dayList.begin(), dayList.end());
    }
    return result;
}

MealPlan::MealPlan(const MealPlan& other)
    : days(other.days), weeklyBudget(other.weeklyBudget), caloriesPerDay(other.caloriesPerDay) {
    for (const auto& constraint: other.constraints) {
        constraints.push_back(constraint->clone());
    }
}

MealPlan& MealPlan::operator=(MealPlan other) {
    swap(*this, other);
    return *this;
}

void swap(MealPlan& first, MealPlan& second) noexcept {
    using std::swap;
    swap(first.days, second.days);
    swap(first.weeklyBudget, second.weeklyBudget);
    swap(first.caloriesPerDay, second.caloriesPerDay);
    swap(first.constraints, second.constraints);
}

bool MealPlan::passesConstraints(const std::vector<Recipe>& todayRecipesSoFar, const Recipe& candidate) const {
    for (const auto& constraint : constraints) {
        if (!constraint->isValid(days, todayRecipesSoFar, candidate)) {
            return false;
        }
    }
    return true;
}
