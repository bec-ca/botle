#pragma once

#include <cassert>
#include <string>
#include <variant>
#include <vector>

#include "error.hpp"
#include "format.hpp"
#include "json.hpp"
#include "to_json.hpp"

struct Error {
  explicit Error(const std::string& msg);
  explicit Error(std::string&& msg);

  Error(const Error& error) = default;
  Error(Error&& error) = default;

  Error& operator=(const Error& error) = default;
  Error& operator=(Error&& error) = default;

  template <class... Ts> static Error format(const char* format, Ts... args)
  {
    return Error(fmt::format(format, args...));
  }

  const std::string& msg() const;

 private:
  std::string _msg;
};

namespace fmt {
template <> struct to_string<Error> {
  static std::string convert(const Error& value) { return value.msg(); }
};
} // namespace fmt

struct Unit {};

static const Unit unit;

template <class T> struct OrError {
 public:
  using value_type = T;

  OrError(const T& value) : _value(std::in_place_index<0>, value) {}
  OrError(T&& value) : _value(std::in_place_index<0>, std::move(value)) {}

  OrError(const Error& msg) : _value(std::in_place_index<1>, msg) {}
  OrError(Error&& msg) : _value(std::in_place_index<1>, std::move(msg)) {}

  template <class U>
  OrError(const OrError<U>& other)
      : _value(
          other.is_error() ? std::variant<T, Error>(other.error())
                           : std::variant<T, Error>(other.value()))
  {}

  template <class U>
  OrError(const U& value) : _value(std::in_place_index<0>, value)
  {}

  OrError(const OrError& other) = default;
  OrError(OrError&& other) = default;

  template <class U>
  OrError(OrError<U>&& other)
      : _value(
          other.is_error()
            ? std::variant<T, Error>(
                std::in_place_type<Error>, std::move(other.error()))
            : std::variant<T, Error>(
                std::in_place_type<T>, std::move(other.value())))
  {}

  OrError& operator=(const OrError& other) = default;
  OrError& operator=(OrError&& other) = default;

  bool is_error() const { return std::holds_alternative<Error>(_value); }

  const Error& error() const& { return std::get<Error>(_value); }
  Error& error() & { return std::get<Error>(_value); }
  Error&& error() && { return std::get<Error>(_value); }

  const T& value() const& { return std::get<T>(_value); }
  T& value() & { return std::get<T>(_value); }
  T&& value() && { return std::get<T>(_value); }

  T value_default(T def)
  {
    if (is_error()) { return def; }
    return value();
  }

  T move_value_default(T def)
  {
    if (is_error()) { return def; }
    return std::move(value());
  }

  void assert_not_error() const
  {
    if (is_error()) {
      fmt::print_err_line("Called on error: $", error());
      assert(false);
    }
  }

  const T& force() const&
  {
    assert_not_error();
    return value();
  }

  T&& force() &&
  {
    assert_not_error();
    return std::move(value());
  }

  template <class F>
  OrError<typename std::invoke_result<F(T)>::type::value_type> bind(
    F&& f) const&
  {
    if (!is_error()) {
      return std::forward<F>(f)(value());
    } else {
      return error();
    }
  }

  template <class F>
  OrError<typename std::invoke_result<F(T)>::type::value_type> bind(F&& f) &&
  {
    if (!is_error()) {
      return std::forward<F>(f)(std::move(value()));
    } else {
      return std::move(error());
    }
  }

  template <class F>
  OrError<typename std::invoke_result<F(T)>::type> map(F&& f) const&
  {
    if (!is_error()) {
      return std::forward<F>(f)(value());
    } else {
      return error();
    }
  }

  template <class F>
  OrError<typename std::invoke_result<F(T)>::type> map(F&& f) &&
  {
    if (!is_error()) {
      return std::forward<F>(f)(std::move(value()));
    } else {
      return std::move(error());
    }
  }

 private:
  std::variant<T, Error> _value;
};

namespace fmt {
template <class T> struct to_string<OrError<T>> {
  static std::string convert(const OrError<T>& value)
  {
    if (value.is_error()) {
      return fmt::format("Error($)", value.error());
    } else {
      return fmt::format("Ok($)", value.value());
    }
  }
};
} // namespace fmt

namespace json {

template <class T> struct to_json_t<OrError<T>> {
  static JsonValue convert(const OrError<T>& value)
  {
    if (value.is_error()) {
      return to_json(std::map<std::string, JsonValue>{
        {"error", to_json(value.error().msg())}});
    } else {
      return to_json(
        std::map<std::string, JsonValue>{{"ok", to_json(value.value())}});
    }
  }
};

} // namespace json

#define bail(var, or_error)                                                    \
  auto __var##var = (or_error);                                                \
  if ((__var##var).is_error()) { return std::move((__var##var).error()); };    \
  auto var = std::move((__var##var).value());

#define bail_assign(var, or_error)                                             \
  do {                                                                         \
    auto __tmp__ = (or_error);                                                 \
    if (__tmp__.is_error()) { return __tmp__.error(); };                       \
    var = std::move(__tmp__.value());                                          \
  } while (false)

#define bail_unit(or_error)                                                    \
  do {                                                                         \
    auto __var = (or_error);                                                   \
    if ((__var).is_error()) { return std::move((__var).error()); };            \
  } while (false)
