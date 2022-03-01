#pragma once

#include "to_string.hpp"

#include <string>
#include <unordered_map>

namespace fmt {
template <class T, class F> struct to_string<std::unordered_map<T, F>> {
  static std::string convert(const std::unordered_map<T, F>& values)
  {
    return convert_container(values);
  }
};
} // namespace fmt
