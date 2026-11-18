//
// Created by Matei on 11/18/2026.
//

#include "Errors.h"

WeekBiteError::WeekBiteError(const std::string& message) : std::runtime_error(message) {}

FileError::FileError(const std::string& detail) : WeekBiteError("File error: " + detail) {}

InputError::InputError(const std::string& detail) : WeekBiteError("Input error: " + detail) {}

AmountError::AmountError(const std::string& detail) : WeekBiteError("Amount error: " + detail) {}

UnitError::UnitError(const std::string& detail) : WeekBiteError("Unit error: " + detail) {}

EmptyCategoryError::EmptyCategoryError(const std::string& detail) : WeekBiteError("Empty category error: " + detail) {}

PlanImpossibleError::PlanImpossibleError(const std::string& reason) : WeekBiteError("Plan impossible error: " + reason) {}
