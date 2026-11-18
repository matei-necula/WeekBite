//
// Created by Matei on 10/25/2026.
//

#ifndef OOP_NUTRITION_H
#define OOP_NUTRITION_H

#include <ostream>
#include <array>

// the 5 values from recipes.csv, per serving; a small class instead of 5 loose fields in Recipe
class Nutrition {
    double calories;
    double protein;
    double carbohydrates;
    double fats;
    double sugar;

    std::array<double, 5> values() const;
public:
    Nutrition(double calories, double protein, double carbohydrates, double fats, double sugar);

    Nutrition& operator+=(const Nutrition& other);

    // true if no value is above its limit; a limit of 0 means "no limit"
    bool fitsUnder(const Nutrition& maxim) const;

    // true if every value is at least its minimum; a minimum of 0 is always reached
    bool reaches(const Nutrition& minim) const;

    double getCalories() const;

    // only the values above 0, e.g. "protein 100 g, sugar 50 g"; "none" when all are 0
    // (used by NutritionTargetConstraint to show its limits)
    void printNonZero(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Nutrition& nutrition);
};


#endif //OOP_NUTRITION_H
