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

std::ostream& operator<<(std::ostream& os, const Nutrition& nutrition) {
    os << "Calories: " << nutrition.calories << "kcal, Protein: " << nutrition.protein << "g, Carbohydrates: " << nutrition.carbohydrates << "g, Fats: " << nutrition.fats << "g, Sugar: " << nutrition.sugar <<"g";
    return os;
}
