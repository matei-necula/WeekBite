//
// Created by Matei on 11/02/2026.
//

#include "fileHelper.h"
#include "Errors.h"

  std::ofstream openFile(const std::string& fileName) {
      std::ofstream file(fileName);
      if (!file) {
          throw FileError("File could not be opened");
      }
      return file;
  }
