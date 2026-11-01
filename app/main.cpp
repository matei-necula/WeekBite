#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <csv.hpp>
#include "MealPlan.h"
#include "Recipes.h"

int main() {
    // ---------- 1. read the 3 CSV files (assets/ is copied next to the executable) ----------

    // ingredients.csv  →  name,price
    std::map<std::string, double> prices;
    csv::CSVReader ingredientsFile{"assets/ingredients.csv"};
    for (csv::CSVRow &row: ingredientsFile) {
        prices[row["name"].get<std::string>()] = row["price"].get<double>();
    }

    // recipes.csv  →  name,category,calories,protein,carbohydrates,fats,sugar
    std::vector<RecipeRow> recipeRows;
    csv::CSVReader recipesFile{"assets/recipes.csv"};
    for (csv::CSVRow &row: recipesFile) {
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
    for (csv::CSVRow &row: recipeIngredientsFile) {
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


    while (true) {
        char choice = 'q';
        std::cout << "Plan a week (p), search a meal (s) or quit (q): ";
        if (!(std::cin >> choice) || choice == 'q') {
            break;
        }
        if (choice == 'p') {
            double weeklyBudget;
            std::cout << "Enter weekly budget: ";
            if (!(std::cin >> weeklyBudget) || weeklyBudget <= 0) {
                std::cout << "Invalid input. Please enter a positive number." << std::endl;
                break;
            }

            double caloriesPerDay;
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
        } else if (choice == 's') {
            bool categoryPicked = false;
            bool namePicked = false;
            bool ingredientPicked = false;
            while (true) {
                std::string criteria;
                std::cout << "Choose criteria to search by (category,name,ingredient), no spaces: ";
                if (!(std::cin >> criteria)) {
                    return 0;
                }
                try {
                    auto [c, n, i] = reader(criteria);
                    categoryPicked = c;
                    namePicked = n;
                    ingredientPicked = i;
                    break;
                } catch (const std::invalid_argument &e) {
                    std::cout << e.what() << '\n';
                }
            }
            Category category = Category::breakfast; // used only ifcategoryPicked
            if (categoryPicked) {
                std::cout << "Choose category: breakfast-1, lunch-2, dinner-3: ";
                int categoryNumber;
                std::cin >> categoryNumber;
                if (categoryNumber == 1) {
                    category = Category::breakfast;
                } else if (categoryNumber == 2) {
                    category = Category::lunch;
                } else if (categoryNumber == 3) {
                    category = Category::dinner;
                } else {
                    std::cout << "Invalid category number. Please choose 1, 2, or 3.\n";
                    break;
                }
            }
            std::string nameText;
            if (namePicked) {
                std::cout << "Type the name: ";
                std::cin >> nameText;
            }
            std::string ingredientText;
            if (ingredientPicked) {
                std::cout << "Type the ingredient: ";
                std::cin >> ingredientText;
            }
            // 3. sort choice
            std::cout << "Sort by calories (1) or price (2): ";
            int sortNumber;
            std::cin >> sortNumber;
            SortChoice sortChoice = SortChoice::calories;
            if (sortNumber == 1) {
                sortChoice = SortChoice::calories;
            } else if (sortNumber == 2) {
                sortChoice = SortChoice::price;
            } else {
                std::cout << "Invalid sort choice. Please choose 1 or 2.\n";
                break;
            }

            const UserChoices choices{
                categoryPicked, namePicked, ingredientPicked, category, nameText, ingredientText, sortChoice
            };
            const std::vector<Recipe> results = catalog.searchUserChoices(choices);
            if (results.empty()) {
                std::cout << "No recipes found.\n";
                continue;
            }

            for (std::size_t i = 0; i < results.size(); ++i) {
                std::cout << i + 1 << ". ";
                results[i].printShort(std::cout);
            }

            std::cout << "Pick a recipe number: ";
            int pick;
            std::cin >> pick;
            if (pick < 1 || static_cast<std::size_t>(pick) > results.size()) {
                std::cout << "Invalid recipe number.\n";
                break;
            }
            std::cout << results[pick - 1];
            std::cout << "Do you want to scale the recipe? (y/n): ";
            char scaleChoice;
            std::cin >> scaleChoice;
            if (scaleChoice == 'y' || scaleChoice == 'Y') {
                std::cout << "Enter the number of servings: ";
                int servings;
                if (!(std::cin >> servings) || servings <= 0) {
                    std::cout << "Invalid input. Please enter a positive number." << std::endl;
                    break;
                }
                std::cout << results[pick - 1].scaled(servings);
            }
        } else {
            std::cout << "Invalid choice. Type p, s or q." << std::endl;
        }
    }
    return 0;
}
