#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <csv.hpp>
#include "MealPlan.h"
#include "Recipes.h"
#include <iomanip>
#include "ShoppingList.h"

std::string exportQuestion() {
    std::cout << "Export to a file? (y/n): ";
    char choice = 'n';
    std::cin >> choice;
    if (choice != 'y') {
        return "";
    }
    std::cout << "File name: ";
    std::string fileName;
    std::cin >> fileName;
    return fileName;
}

void pantryQuestion(ShoppingList& list) {
    std::cout << "Do you have ingredients at home? (y/n): ";
    char answer = 'n';
    std::cin >> answer;
    if (answer != 'y') {
        return;
    }
    while (true) {
        std::cout << "Type them as name amount unit, separated by commas (oats 200 g, milk 500 ml): ";
        std::string line;
        if (!std::getline(std::cin >> std::ws, line)) {
            return;
        }
        try {
            const std::vector<std::string> notFound = list.subtractPantry(pantryReader(line));
            for (const std::string& name : notFound) {
                std::cout << name << " is not on your list\n";
            }
            return;
        } catch (const std::invalid_argument &e) {
            std::cout << e.what() << '\n';
        }
    }
}

void sortQuestion(ShoppingList& list) {
    std::cout << "Sort the list by price (1), name (2) or keep it as it is (0): ";
    int sortNumber = 0;
    std::cin >> sortNumber;
    if (sortNumber == 1) {
        list.sortBy(ListSort::price);
    } else if (sortNumber == 2) {
        list.sortBy(ListSort::name);
    }
}

void shoppingListQuestion(const MealPlan& plan) {
    std::cout << "Show the shopping list? (y/n): ";
    char answer = 'n';
    std::cin >> answer;
    if (answer != 'y') {
        return;
    }
    auto ingredients = plan.giveIngredients();
    ShoppingList list{ingredients};
    pantryQuestion(list);
    sortQuestion(list);
    std::cout << list;
    const std::string fileName = exportQuestion();
    if (!fileName.empty()) {
        list.exportList(fileName);
    }
}

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
        try {
            char choice = 'q';
            std::cout << "Plan a week (p), search a meal (s), catalog stats (c) or quit (q): ";
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

                const std::streamsize oldPrecision = std::cout.precision();
                std::cout << std::fixed << std::setprecision(2);
                std::cout << "Average kcal per day: " << newPlan.averageKcalPerDay()
                          << " kcal (target: " << caloriesPerDay << " kcal)\n";
                std::cout << "Week nutrition total:\n" << newPlan.weekNutritionTotal() << '\n';
                std::cout << "Budget left: " << newPlan.budgetLeft() << " lei\n";
                std::cout << "Cheapest day: Day " << newPlan.cheapestDay() << '\n';
                std::cout << "Most expensive day: Day " << newPlan.mostExpensiveDay() << '\n';
                std::cout << std::defaultfloat;
                std::cout.precision(oldPrecision);
                shoppingListQuestion(newPlan);

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
                    shoppingListQuestion(scaledPlan);
                    const std::string fileName = exportQuestion();
                    if (!fileName.empty()) {
                        newPlan.exportPlanAndScaled(fileName, scaledPlan);
                    }
                } else {
                    const std::string fileName = exportQuestion();
                    if (!fileName.empty()) {
                        newPlan.exportPlan(fileName);
                    }
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
                    const Recipe savedScaledRecipe = results[pick - 1].scaled(servings);
                    std::cout << savedScaledRecipe;
                    const std::string fileName = exportQuestion();
                    if (!fileName.empty()) {
                        results[pick-1].exportRecipeAndScaled(fileName, savedScaledRecipe);
                    }
                } else {
                    const std::string fileName = exportQuestion();
                    if (!fileName.empty()) {
                        results[pick-1].exportRecipe(fileName);
                    }
                }

            } else if (choice == 'c') {
                const std::vector<Category> categories = {
                    Category::breakfast,
                    Category::lunch,
                    Category::dinner
                };

                const std::streamsize oldPrecision = std::cout.precision();
                for (Category cat : categories) {
                    const auto& list = catalog.getRecipesFromCategory(cat);
                    std::cout << categoryToText(cat) << " (" << list.size() << " recipes):\n";
                    std::cout << std::fixed << std::setprecision(2);
                    std::cout << "Average price: " << catalog.averagePrice(cat) << " lei\n";
                    std::cout << "Average calories: " << catalog.averageKcal(cat) << " kcal\n";
                    std::cout << std::defaultfloat;
                    std::cout << "Cheapest: ";
                    catalog.cheapestRecipe(cat).printShort(std::cout);
                    std::cout << "Most expensive: ";
                    catalog.mostExpensiveRecipe(cat).printShort(std::cout);
                    std::cout << '\n';
                }
                std::cout.precision(oldPrecision);
            }
            else {
                std::cout << "Invalid choice. Type p, s, c or q." << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cout << e.what() << '\n';
        }
    }
    return 0;
}
