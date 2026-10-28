#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <csv.hpp>

#include "MealPlan.h"
#include "Recipes.h"

int main() {
    // ---------- 1. read the 3 CSV files (assets/ is copied next to the executable) ----------

    // ingredients.csv  →  name,price
    std::map<std::string, double> prices;
    csv::CSVReader ingredientsFile{"assets/ingredients.csv"};
    for (csv::CSVRow& row : ingredientsFile) {
        prices[row["name"].get<std::string>()] = row["price"].get<double>();
    }

    // recipes.csv  →  name,category,calories,protein,carbohydrates,fats,sugar
    std::vector<RecipeRow> recipeRows;
    csv::CSVReader recipesFile{"assets/recipes.csv"};
    for (csv::CSVRow& row : recipesFile) {
        recipeRows.emplace_back(row["name"].get<std::string>(),
                                row["category"].get<std::string>(),
                                Nutrition{
                                    row["calories"].get<double>(),
                                    row["protein"].get<double>(),
                                    row["carbohydrates"].get<double>(),
                                    row["fats"].get<double>(),
                                    row["sugar"].get<double>()
                                });
    }

    // recipe_ingredients.csv  →  recipe,ingredient,amount,unit
    std::map<std::string, std::vector<IngredientRow> > ingredientsByRecipe;
    csv::CSVReader recipeIngredientsFile{"assets/recipe_ingredients.csv"};
    for (csv::CSVRow& row : recipeIngredientsFile) {
        ingredientsByRecipe[row["recipe"].get<std::string>()].emplace_back(row["ingredient"].get<std::string>(),
                                                                           row["amount"].get<double>(),
                                                                           row["unit"].get<std::string>());
    }

    const Recipes catalog{prices, recipeRows, ingredientsByRecipe};

    char showCatalog = 'n';
    std::cout << "Show catalog? (y/n): ";
    std::cin >> showCatalog;
    if (showCatalog == 'y') {
        std::cout << catalog;
    }


    double weeklyBudget;
    std::cout << "Enter weekly budget: ";
    if (!(std::cin >> weeklyBudget) || weeklyBudget <= 0) {
        std::cout << "Invalid input. Please enter a positive number." << std::endl;
        return 0;
    }

    double caloriesPerDay;
    std::cout << "Enter calories per day: ";
    if (!(std::cin >> caloriesPerDay) || caloriesPerDay <= 0) {
        std::cout << "Invalid input. Please enter a positive number." << std::endl;
        return 0;
    }

    const MealPlan plan{weeklyBudget, caloriesPerDay, catalog};
    std::cout << plan;
    if (plan.weekCompleted()) {
        char answer = 'n';
        std::cout << "Scale the servings? (y/n): ";
        std::cin >> answer;
        if (answer == 'y') {
            int servings;
            std::cout << "Enter number of servings: ";
            if (!(std::cin >> servings) || servings <= 0) {
                std::cout << "Invalid input. Please enter a positive number." << std::endl;
                return 0;
            }
            const MealPlan scaledPlan = plan.scaled(servings);
            std::cout << scaledPlan;
        }
    }
    char again = 'y';
    while (true) {
        std::cout << "Plan another week? (y/n): ";
        if (!(std::cin >> again) || (again != 'y')) {
            break;
        }

        std::cout << "Enter weekly budget: ";
        if (!(std::cin >> weeklyBudget) || weeklyBudget <= 0) {
            std::cout << "Invalid input. Please enter a positive number." << std::endl;
            break;
        }

        std::cout << "Enter calories per day: ";
        if (!(std::cin >> caloriesPerDay) || caloriesPerDay <= 0) {
            std::cout << "Invalid input. Please enter a positive number." << std::endl;
            break;
        }

        const MealPlan newPlan{weeklyBudget, caloriesPerDay, catalog};
        std::cout << newPlan;
        if (!newPlan.weekCompleted()) continue;

        std::cout << "Scale the servings? (y/n): ";
        char answer = 'n';
        std::cin >> answer;
        if (answer == 'y' || answer == 'Y') {
            int servings;
            std::cout << "Enter number of servings: ";
            if (!(std::cin >> servings) || servings <= 0) {
                std::cout << "Invalid input. Please enter a positive number." << std::endl;
                break;
            }
            const MealPlan scaledPlan = newPlan.scaled(servings);
            std::cout << scaledPlan;
        }
    }
    return 0;
}
