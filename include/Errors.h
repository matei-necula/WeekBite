//
// Created by Matei on 11/18/2026.
//

#ifndef OOP_ERRORS_H
#define OOP_ERRORS_H

#include <stdexcept>
#include <string>

// base of every WeekBite error: catch (const WeekBiteError& e) catches all of them
class WeekBiteError : public std::runtime_error {
public:
    explicit WeekBiteError(const std::string& message);
};

// a file could not be opened (fileHelper)
class FileError : public WeekBiteError {
public:
    explicit FileError(const std::string& detail);
};

// text we don't understand: category, diet, search words, pantry line, empty name
class InputError : public WeekBiteError {
public:
    explicit InputError(const std::string& detail);
};

// a number that must be > 0: servings, scale factor, pantry amount
class AmountError : public WeekBiteError {
public:
    explicit AmountError(const std::string& detail);
};

// unknown unit, or adding/subtracting ingredients with different names or units
class UnitError : public WeekBiteError {
public:
    explicit UnitError(const std::string& detail);
};

// stats (average, cheapest, most expensive) asked on a category with no recipes
class EmptyCategoryError : public WeekBiteError {
public:
    explicit EmptyCategoryError(const std::string& detail);
};

// the greedy could not fill all 7 days; reason says why
class PlanImpossibleError : public WeekBiteError {
public:
    explicit PlanImpossibleError(const std::string& reason);
};

#endif //OOP_ERRORS_H
