//
// Created by Matei on 11/03/2026.
//

#ifndef OOP_TEXTHELPER_H
#define OOP_TEXTHELPER_H

#include <string>
#include <vector>

std::string toLowercase(const std::string& text);
// "a;b;c" with ';' -> {"a", "b", "c"}; an empty text gives an empty list
std::vector<std::string> splitText(const std::string& text, char separator);

#endif //OOP_TEXTHELPER_H
