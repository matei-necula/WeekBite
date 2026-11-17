//
// Created by Matei on 10/25/2026.
//

#include "Nutrition.h"

Nutrition::Nutrition(double calories, double protein, double carbohydrates, double fats, double sugar)
    : calories(calories),
      protein(protein),
      carbohydrates(carbohydrates),
      fats(fats),
      sugar(sugar) {
}

Nutrition& Nutrition::operator+=(const Nutrition& other) {
    calories += other.calories;
    protein += other.protein;
    carbohydrates += other.carbohydrates;
    fats += other.fats;
    sugar += other.sugar;
    return *this;
}

double Nutrition::getCalories() const {
    return calories;
}

std::ostream& operator<<(std::ostream& os, const Nutrition& nutrition) {
    os << "Calories: " << nutrition.calories << "kcal, Protein: " << nutrition.protein << "g, Carbohydrates: " <<
            nutrition.carbohydrates << "g, Fats: " << nutrition.fats << "g, Sugar: " << nutrition.sugar << "g";
    return os;
}

std::array<double, 5> Nutrition::values() const {
    return {calories, protein, carbohydrates, fats, sugar};
}

bool Nutrition::fitsUnder(const Nutrition& maxim) const {
    const std::array<double, 5> mine = values();
    const std::array<double, 5> limits = maxim.values();
    for (std::size_t i = 0; i < mine.size(); i++) {
        if (limits[i] > 0 && mine[i] > limits[i]) {
            return false;
        }
    }
    return true;
}

bool Nutrition::reaches(const Nutrition& minim) const {
    const std::array<double, 5> mine = values();
    const std::array<double, 5> limits = minim.values();
    for (std::size_t i = 0; i < mine.size(); i++) {
        if (mine[i] < limits[i]) {
            return false;
        }
    }
    return true;
}
