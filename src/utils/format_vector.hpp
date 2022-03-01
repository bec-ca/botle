#pragma once

#include "to_string.hpp"

#include <string>
#include <vector>

namespace fmt {

template <class T> struct to_string<std::vector<T>> {
  static std::string convert(const std::vector<T>& values)
  {
    return convert_container(values);
  }
};

}; // namespace fmt
