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
#include <memory>
#include "Errors.h"
#include "DietRestrictionConstraint.h"
#include "MealBalanceConstraint.h"
#include "NutritionTargetConstraint.h"
#include "WeekdayPrepTimeConstraint.h"
#include "textHelper.h"
#include <optional>

// a line between what the user types and what the program shows
void printSeparator() {
    std::cout << "----------------------------------------\n";
}

// call it after a number could not be read: makes cin usable again and
// skips the typed word ("back" goes back quietly, anything else is reported)
void skipBadInput() {
    if (std::cin.eof()) {
        return;
    }
    std::cin.clear();
    std::string word;
    std::cin >> word;
    if (word != "back") {
        std::cout << "Not a number.\n";
    }
}

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

// asks until the user types a number >= 0; if the input ends, 0 (no target)
double readNonNegative(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        double value = 0;
        if (std::cin >> value) {
            if (value >= 0) {
                return value;
            }
            std::cout << "The number must be 0 or more.\n";
            continue;
        }
        if (std::cin.eof()) {
            return 0;
        }
        skipBadInput();
    }
}

std::vector<std::unique_ptr<PlanConstraint>> constraintsQuestion() {
    std::vector<std::unique_ptr<PlanConstraint>> constraints;
    std::cout << "Keep meals balanced (no meal more than 3x another)? (y/n): ";
    char choice = 'n';
    std::cin >> choice;
    if (choice == 'y') {
        constraints.push_back(std::make_unique<MealBalanceConstraint>());
    }
    std::cout << "Set daily nutrition targets? (y/n): ";
    std::cin >> choice;
    if (choice == 'y') {
        const double minProtein = readNonNegative("Minimum protein per day in g (0 = none): ");
        const double maxCarbs = readNonNegative("Maximum carbs per day in g (0 = no limit): ");
        const double maxFats = readNonNegative("Maximum fats per day in g (0 = no limit): ");
        const double maxSugar = readNonNegative("Maximum sugar per day in g (0 = no limit): ");
        constraints.push_back(std::make_unique<NutritionTargetConstraint>(
            Nutrition{0, minProtein, 0, 0, 0},
            Nutrition{0, 0, maxCarbs, maxFats, maxSugar}));
    }
    std::cout << "Limit the cooking time per meal? (y/n): ";
    std::cin >> choice;
    if (choice == 'y') {
        const double minutes = readNonNegative("Maximum minutes per meal: ");
        std::cout << "Only on weekdays (Monday-Friday)? (n = limit time in weekends too) (y/n): ";
        char weekdaysAnswer = 'n';
        std::cin >> weekdaysAnswer;
        constraints.push_back(
            std::make_unique<WeekdayPrepTimeConstraint>(static_cast<int>(minutes), weekdaysAnswer == 'y'));
    }
    std::cout << "Any diet restrictions? (y/n): ";
    std::cin >> choice;
    if (choice == 'y') {
        while (true) {
            std::cout << "Type them separated by commas, no spaces "
                         "(vegan,vegetarian,glutenfree,lactosefree,nopork,noredmeat): ";
            std::string line;
            std::cin >> line;
            try {
                std::vector<std::unique_ptr<PlanConstraint>> diets;
                for (const std::string& piece : splitText(line, ',')) {
                    diets.push_back(std::make_unique<DietRestrictionConstraint>(dietFromText(piece)));
                }
                for (std::unique_ptr<PlanConstraint>& diet : diets) {
                    constraints.push_back(std::move(diet));
                }
                break;
            } catch (const InputError& e) {
                std::cout << e.what() << '\n';
            }
        }
    }
    return constraints;
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
            for (const std::string& name: notFound) {
                std::cout << name << " is not on your list\n";
            }
            return;
        } catch (const WeekBiteError& e) {
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
    printSeparator();
    std::cout << list;
    printSeparator();
    const std::string fileName = exportQuestion();
    if (!fileName.empty()) {
        try{
            list.exportList(fileName);
        } catch (const FileError& e) {
            std::cout << "Could not save the shopping list: " << e.what() << '\n';
        }
    }
}

//builds the plan; on PlanImpossibleError prints why and returns no plan
std::optional<MealPlan> makePlan(double weeklyBudget, double caloriesPerDay, const Recipes& catalog,
                                 std::vector<std::unique_ptr<PlanConstraint>> constraints) {
    try {
        return MealPlan{weeklyBudget, caloriesPerDay, catalog, std::move(constraints)};
    } catch (const PlanImpossibleError& e) {
        std::cout << e.what() << std::endl;
        return std::nullopt;   // "no plan"
    }
}

// ---------- reading the CSV files (assets/ is copied next to the executable) ----------

// ingredients.csv  →  name,price,suitable_for,contains
std::map<std::string, IngredientInfo> readIngredientInfo() {
    std::map<std::string, IngredientInfo> ingredientInfo;
    csv::CSVReader ingredientsFile{"assets/ingredients.csv"};
    for (csv::CSVRow& row: ingredientsFile) {
        ingredientInfo[row["name"].get<std::string>()] = {
            row["price"].get<double>(), splitText(row["suitable_for"].get<std::string>(), ';'),
            splitText(row["contains"].get<std::string>(), ';')
        };
    }
    return ingredientInfo;
}

// recipes.csv  →  name,category,calories,protein,carbohydrates,fats,sugar,prep_time_min
std::vector<RecipeRow> readRecipeRows() {
    std::vector<RecipeRow> recipeRows;
    csv::CSVReader recipesFile{"assets/recipes.csv"};
    for (csv::CSVRow& row: recipesFile) {
        recipeRows.emplace_back(row["name"].get<std::string>(),
                                row["category"].get<std::string>(),
                                Nutrition{
                                    row["calories"].get<double>(),
                                    row["protein"].get<double>(),
                                    row["carbohydrates"].get<double>(),
                                    row["fats"].get<double>(),
                                    row["sugar"].get<double>()
                                },
                                row["prep_time_min"].get<int>());
    }
    return recipeRows;
}

// recipe_ingredients.csv  →  recipe,ingredient,amount,unit
std::map<std::string, std::vector<IngredientRow>> readIngredientsByRecipe() {
    std::map<std::string, std::vector<IngredientRow>> ingredientsByRecipe;
    csv::CSVReader recipeIngredientsFile{"assets/recipe_ingredients.csv"};
    for (csv::CSVRow& row: recipeIngredientsFile) {
        ingredientsByRecipe[row["recipe"].get<std::string>()].emplace_back(row["ingredient"].get<std::string>(),
                                                                           row["amount"].get<double>(),
                                                                           row["unit"].get<std::string>());
    }
    return ingredientsByRecipe;
}

// builds the catalog; if a file is missing or broken, prints why and returns no catalog
std::optional<Recipes> loadCatalog() {
    try {
        return Recipes{readIngredientInfo(), readRecipeRows(), readIngredientsByRecipe()};
    } catch (const std::runtime_error& e) {
        std::cout << "Could not read the recipe files: " << e.what() << '\n';
        return std::nullopt;
    }
}

// ---------- the 3 menu choices; "return" = back to the menu ----------

void planWeek(const Recipes& catalog) {
    double weeklyBudget = 0;
    std::cout << "Enter weekly budget (or back): ";
    if (!(std::cin >> weeklyBudget)) {
        skipBadInput();
        return;
    }
    if (weeklyBudget <= 0) {
        std::cout << "The budget must be a positive number." << std::endl;
        return;
    }

    double caloriesPerDay = 0;
    std::cout << "Enter calories per day (or back): ";
    if (!(std::cin >> caloriesPerDay)) {
        skipBadInput();
        return;
    }
    if (caloriesPerDay <= 0) {
        std::cout << "The calories must be a positive number." << std::endl;
        return;
    }

    const std::optional<MealPlan> newPlan = makePlan(weeklyBudget, caloriesPerDay, catalog, constraintsQuestion());
    if (!newPlan) {
        printSeparator();
        return;
    }
    printSeparator();
    std::cout << *newPlan;

    const std::streamsize oldPrecision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average kcal per day: " << newPlan->averageKcalPerDay()
            << " kcal (target: " << caloriesPerDay << " kcal)\n";
    std::cout << "Week nutrition total:\n" << newPlan->weekNutritionTotal() << '\n';
    std::cout << "Budget left: " << newPlan->budgetLeft() << " lei\n";
    std::cout << "Cheapest day: Day " << newPlan->cheapestDay() << '\n';
    std::cout << "Most expensive day: Day " << newPlan->mostExpensiveDay() << '\n';
    std::cout << std::defaultfloat;
    std::cout.precision(oldPrecision);
    printSeparator();
    shoppingListQuestion(*newPlan);

    try {
        std::cout << "Scale the servings? (y/n): ";
        char answer = 'n';
        std::cin >> answer;
        if (answer == 'y' || answer == 'Y') {
            int servings = 0;
            std::cout << "Enter number of servings (or back): ";
            if (!(std::cin >> servings)) {
                skipBadInput();
                return;
            }
            if (servings <= 0) {
                std::cout << "The servings must be a positive number." << std::endl;
                return;
            }
            const MealPlan scaledPlan = newPlan->scaled(servings);
            printSeparator();
            std::cout << scaledPlan;
            printSeparator();
            shoppingListQuestion(scaledPlan);
            const std::string fileName = exportQuestion();
            if (!fileName.empty()) {
                newPlan->exportPlanAndScaled(fileName, scaledPlan);
            }
        } else {
            const std::string fileName = exportQuestion();
            if (!fileName.empty()) {
                newPlan->exportPlan(fileName);
            }
        }
    }
    catch (const FileError& e) {
        std::cout << "Could not save the file: " << e.what() << '\n';
    }
}

void searchMeal(const Recipes& catalog) {
    bool categoryPicked = false;
    bool namePicked = false;
    bool ingredientPicked = false;
    bool goBack = false;
    while (true) {
        std::string criteria;
        std::cout << "Search by:\n"
                << "  category   - the meal of the day: breakfast, lunch or dinner\n"
                << "  name       - a word from the recipe's name\n"
                << "  ingredient - something the recipe contains\n"
                << "Type one or more, separated by commas, no spaces (e.g. category,ingredient), or back: ";
        if (!(std::cin >> criteria)) {
            return;
        }
        if (criteria == "back") {
            goBack = true;
            break;
        }
        try {
            auto [c, n, i] = reader(criteria);
            categoryPicked = c;
            namePicked = n;
            ingredientPicked = i;
            break;
        } catch (const InputError& e) {
            std::cout << e.what() << '\n';
        }
    }
    if (goBack) {
        return;
    }
    Category category = Category::breakfast; // used only ifcategoryPicked
    if (categoryPicked) {
        std::cout << "Which meal of the day? breakfast (1), lunch (2), dinner (3), or back: ";
        int categoryNumber = 0;
        if (!(std::cin >> categoryNumber)) {
            skipBadInput();
            return;
        }
        if (categoryNumber == 1) {
            category = Category::breakfast;
        } else if (categoryNumber == 2) {
            category = Category::lunch;
        } else if (categoryNumber == 3) {
            category = Category::dinner;
        } else {
            std::cout << "Invalid category number. Please choose 1, 2, or 3.\n";
            return;
        }
    }
    std::string nameText;
    if (namePicked) {
        std::cout << "Type a word from the recipe's name (or back): ";
        std::cin >> nameText;
        if (nameText == "back") {
            return;
        }
    }
    std::string ingredientText;
    if (ingredientPicked) {
        std::cout << "Type the ingredient (or back): ";
        std::cin >> ingredientText;
        if (ingredientText == "back") {
            return;
        }
    }
    // 3. sort choice
    std::cout << "Sort by calories (1) or price (2), or back: ";
    int sortNumber = 0;
    if (!(std::cin >> sortNumber)) {
        skipBadInput();
        return;
    }
    SortChoice sortChoice = SortChoice::calories;
    if (sortNumber == 1) {
        sortChoice = SortChoice::calories;
    } else if (sortNumber == 2) {
        sortChoice = SortChoice::price;
    } else {
        std::cout << "Invalid sort choice. Please choose 1 or 2.\n";
        return;
    }

    const UserChoices choices{
        categoryPicked, namePicked, ingredientPicked, category, nameText, ingredientText, sortChoice
    };
    const std::vector<Recipe> results = catalog.searchUserChoices(choices);
    if (results.empty()) {
        std::cout << "No recipes found.\n";
        return;
    }

    printSeparator();
    for (std::size_t i = 0; i < results.size(); ++i) {
        std::cout << i + 1 << ". ";
        results[i].printShort(std::cout);
    }
    printSeparator();

    std::cout << "Pick a recipe number (or back): ";
    int pick = 0;
    if (!(std::cin >> pick)) {
        skipBadInput();
        return;
    }
    if (pick < 1 || static_cast<std::size_t>(pick) > results.size()) {
        std::cout << "Invalid recipe number.\n";
        return;
    }
    printSeparator();
    std::cout << results[pick - 1];
    printSeparator();
    try {
        std::cout << "Do you want to scale the recipe? (y/n): ";
        char scaleChoice;
        std::cin >> scaleChoice;
        if (scaleChoice == 'y' || scaleChoice == 'Y') {
            std::cout << "Enter the number of servings (or back): ";
            int servings = 0;
            if (!(std::cin >> servings)) {
                skipBadInput();
                return;
            }
            if (servings <= 0) {
                std::cout << "The servings must be a positive number." << std::endl;
                return;
            }
            const Recipe savedScaledRecipe = results[pick - 1].scaled(servings);
            printSeparator();
            std::cout << savedScaledRecipe;
            printSeparator();
            const std::string fileName = exportQuestion();
            if (!fileName.empty()) {
                results[pick - 1].exportRecipeAndScaled(fileName, savedScaledRecipe);
            }
        } else {
            const std::string fileName = exportQuestion();
            if (!fileName.empty()) {
                results[pick - 1].exportRecipe(fileName);
            }
        }
    }
    catch (const FileError& e) {
        std::cout << "Could not save the file: " << e.what() << '\n';
    }
}

void showCatalogStats(const Recipes& catalog) {
    const std::vector<Category> categories = {
        Category::breakfast,
        Category::lunch,
        Category::dinner
    };

    const std::streamsize oldPrecision = std::cout.precision();
    printSeparator();
    for (Category cat: categories) {
        try {
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
        catch (const EmptyCategoryError& e) {
            std::cout << e.what() << '\n';
        }
    }
    std::cout.precision(oldPrecision);
    printSeparator();
}

int main() {
    const std::optional<Recipes> catalog = loadCatalog();
    if (!catalog) {
        return 1;
    }
    char showCatalog = 'n';
    std::cout << "Show catalog? (y/n): ";
    std::cin >> showCatalog;
    if (showCatalog == 'y') {
        printSeparator();
        std::cout << *catalog;
        printSeparator();
    }

    while (true) {
        char choice = 'q';
        std::cout << "Plan a week (p), search a meal (s), catalog stats (c) or quit (q): ";
        if (!(std::cin >> choice) || choice == 'q') {
            break;
        }
        if (choice == 'p') {
            planWeek(*catalog);
        } else if (choice == 's') {
            searchMeal(*catalog);
        } else if (choice == 'c') {
            showCatalogStats(*catalog);
        } else {
            std::cout << "Invalid choice. Type p, s, c or q." << std::endl;
        }
    }
    return 0;
}
