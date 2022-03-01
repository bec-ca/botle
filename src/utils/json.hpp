#pragma once

#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "to_json.hpp"

namespace json {

struct null_t {};

constexpr null_t null;

struct JsonValue {
  JsonValue(const JsonValue& other) : _value(other._value) {}

  explicit JsonValue(JsonValue&& other) : _value(std::move(other._value)) {}

  explicit JsonValue(const std::string& value) : _value(value) {}
  explicit JsonValue(int value) : _value(value) {}
  explicit JsonValue(double value) : _value(value) {}
  explicit JsonValue(null_t value) : _value(value) {}
  explicit JsonValue(const std::vector<JsonValue>& value) : _value(value) {}
  explicit JsonValue(const std::map<std::string, JsonValue>& value)
      : _value(value)
  {}

  const std::map<std::string, JsonValue>& get_object() const;
  const std::vector<JsonValue>& get_list() const;
  const std::string& get_string() const;
  int get_int() const;
  double get_double() const;

  // template <class T>
  // JsonValue(const T& value) : _value(value) {}

  // template <class T>
  // JsonValue(T&& value) : _value(forward<T>(value)) {}

  using value_type = std::variant<
    std::string,
    int,
    double,
    null_t,
    std::vector<JsonValue>,
    std::map<std::string, JsonValue>>;

  value_type _value;

  std::string to_string() const;

  static JsonValue of_string(const std::string& str);

  std::string what_alternative() const;
};

template <> struct to_json_t<JsonValue> {
  static JsonValue convert(const JsonValue& value) { return JsonValue(value); }
  static JsonValue convert(JsonValue&& value)
  {
    return JsonValue(std::move(value));
  }
};

template <> struct to_json_t<std::string> {
  static JsonValue convert(const std::string& value)
  {
    return JsonValue(value);
  }
  static JsonValue convert(std::string&& value)
  {
    return JsonValue(std::move(value));
  }
};

template <> struct to_json_t<int> {
  static JsonValue convert(int value) { return JsonValue(value); }
};

template <> struct to_json_t<double> {
  static JsonValue convert(double value) { return JsonValue(value); }
};

template <> struct to_json_t<bool> {
  static JsonValue convert(bool value)
  {
    return JsonValue(value ? "true" : "false");
  }
};

template <class T> struct to_json_t<std::optional<T>> {
  static JsonValue convert(const std::optional<T>& value)
  {
    return value.has_value() ? to_json_t<T>::convert(value.value())
                             : JsonValue(null);
  }
};

template <class T> struct to_json_t<std::vector<T>> {
  static JsonValue convert(const std::vector<T>& values)
  {
    std::vector<JsonValue> json_values;
    for (const auto& v : values) {
      json_values.emplace_back(to_json_t<T>::convert(v));
    }
    return JsonValue(move(json_values));
  }

  static JsonValue convert(std::vector<T>&& values)
  {
    std::vector<JsonValue> json_values;
    for (auto& v : values) {
      json_values.emplace_back(to_json_t<T>::convert(move(v)));
    }
    return JsonValue(move(json_values));
  }
};

template <class T> struct to_json_t<std::map<std::string, T>> {
  static JsonValue convert(const std::map<std::string, T>& values)
  {
    std::map<std::string, JsonValue> json_values;
    for (const auto& v : values) {
      json_values.emplace(v.first, to_json_t<T>::convert(v.second));
    }
    return JsonValue(move(json_values));
  }

  static JsonValue convert(std::map<std::string, T>&& values)
  {
    std::map<std::string, JsonValue> json_values;
    for (auto& v : values) {
      json_values.emplace(
        move(v.first), to_json_t<T>::convert(std::move(v.second)));
    }
    return JsonValue(move(json_values));
  }
};

template <class T> JsonValue to_json(T&& value)
{
  using DT = std::decay_t<T>;
  return to_json_t<DT>::convert(std::forward<T>(value));
}

} // namespace json
