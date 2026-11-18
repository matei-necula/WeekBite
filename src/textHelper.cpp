//
// Created by Matei on 11/03/2026.
//
#include "textHelper.h"
#include <cctype>
#include <sstream>

std::string toLowercase(const std::string& text) {
    std::string copy = text;
    for (char& c : copy) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return copy;
}

std::vector<std::string> splitText(const std::string& text, char separator) {
    std::vector<std::string> pieces;
    std::stringstream stream(text);
    std::string piece;
    while (std::getline(stream, piece, separator)) {
        if (!piece.empty()) {
            pieces.push_back(piece);
        }
    }
    return pieces;
}
