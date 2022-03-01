#pragma once

#include "to_string.hpp"

#include <set>
#include <string>

namespace fmt {
template <class T> struct to_string<std::set<T>> {
  static std::string convert(const std::set<T>& values)
  {
    return convert_container(values);
  }
};
} // namespace fmt
