//
// Created by Matei on 11/03/2026.
//
#include "textHelper.h"
#include <cctype>

std::string toLowercase(const std::string& text) {
    std::string copy = text;
    for (char& c : copy) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return copy;
}
