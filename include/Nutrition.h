//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_NUTRITION_H
#define OOP_NUTRITION_H

#include <ostream>

// the 5 values from recipes.csv, per serving; a small class instead of 5 loose fields in Recipe
class Nutrition {
    double calories;
    double protein;
    double carbohydrates;
    double fats;
    double sugar;

public:
    Nutrition(double calories, double protein, double carbohydrates, double fats, double sugar);

    Nutrition& operator+=(const Nutrition& other);

    double getCalories() const;
    friend std::ostream& operator<<(std::ostream& os, const Nutrition& nutrition);
};


#endif //OOP_NUTRITION_H
