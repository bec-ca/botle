#pragma once

#include "internal_string.hpp"
#include "utils/error.hpp"

#include <string>
#include <vector>

struct Dictionary {
  static OrError<std::vector<InternalString>> load_words(
    const std::string& filename);
};
