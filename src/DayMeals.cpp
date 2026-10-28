//
// Created by Matei on 10/27/2026.
//

#include "DayMeals.h"
#include <iomanip>
#include <stdexcept>

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

DayMeals DayMeals::scaled(int newServings) const {
    if (newServings <= 0) {
        throw std::invalid_argument("Invalid number of servings");
    }
    DayMeals scaledDay(breakfast.scaled(newServings), lunch.scaled(newServings), dinner.scaled(newServings));
    return scaledDay;
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
