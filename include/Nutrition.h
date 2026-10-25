//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_NUTRITION_H
#define OOP_NUTRITION_H

#include <ostream>

class Nutrition {
    double calories;
    double protein;
    double carbohydrates;
    double fats;
    double sugar;

public:
    Nutrition(double calories, double protein, double carbohydrates, double fats, double sugar);
    friend std::ostream& operator<<(std::ostream& os, const Nutrition& nutrition);
};


#endif //OOP_NUTRITION_H
