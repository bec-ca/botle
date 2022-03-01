#pragma once

#include <string>

template <class> constexpr bool always_false_v = false;

std::string right_pad_string(std::string str, size_t length);
