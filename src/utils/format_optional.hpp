#pragma once

#include "to_string.hpp"

#include <optional>
#include <string>

namespace fmt {

template <class T> struct to_string<std::optional<T>> {
  static std::string convert(const std::optional<T>& value)
  {
    if (value.has_value()) {
      return format("($)", value.value());
    } else {
      return "()";
    }
  }
};

} // namespace fmt
