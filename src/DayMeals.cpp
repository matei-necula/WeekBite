//
// Created by Matei on 10/27/2026.
//

#include "DayMeals.h"
#include <iomanip>

DayMeals::DayMeals(const Recipe& breakfast, const Recipe& lunch, const Recipe& dinner)
    : breakfast(breakfast), lunch(lunch), dinner(dinner) {
}

double DayMeals::dayPrice() const {
    return breakfast.recipePrice() + lunch.recipePrice() + dinner.recipePrice();
}

Nutrition DayMeals::dayNutrition() const {
    Nutrition totalNutrition = breakfast.getNutrition();
    totalNutrition += lunch.getNutrition();
    totalNutrition += dinner.getNutrition();
    return totalNutrition;
}

std::ostream& operator<<(std::ostream& os, const DayMeals& day) {
    os << day.breakfast << "\n" << day.lunch << "\n" << day.dinner << "\n";
    const std::streamsize oldPrecision = os.precision();
    os << std::setprecision(2) << std::fixed << "Day price: " << day.dayPrice() << " lei\n";
    os << std::defaultfloat;
    os.precision(oldPrecision);
    os << "Total nutrition: " << day.dayNutrition();
    os << "\n";

    return os;
}
