//
// Created by Matei on 11/02/2026.
//

#include "fileHelper.h"
  #include <stdexcept>

  std::ofstream openFile(const std::string& fileName) {
      std::ofstream file(fileName);
      if (!file) {
          throw std::runtime_error("File could not be opened");
      }
      return file;
  }
