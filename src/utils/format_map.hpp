#pragma once

#include "to_string.hpp"

#include <map>
#include <string>

namespace fmt {
template <class T, class F> struct to_string<std::map<T, F>> {
  static std::string convert(const std::map<T, F>& values)
  {
    return convert_container(values);
  }
};
} // namespace fmt
