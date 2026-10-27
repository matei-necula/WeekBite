//
// Created by Matei on 10/27/2026.
//

#ifndef OOP_DAYMEALS_H
#define OOP_DAYMEALS_H

#include <ostream>
#include "Nutrition.h"
#include "Recipe.h"

class DayMeals {
    Recipe breakfast;
    Recipe lunch;
    Recipe dinner;

public:
    DayMeals(const Recipe& breakfast, const Recipe& lunch, const Recipe& dinner);

    double dayPrice() const;

    Nutrition dayNutrition() const;

    friend std::ostream& operator<<(std::ostream& os, const DayMeals& day);
};

#endif //OOP_DAYMEALS_H
