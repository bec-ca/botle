#pragma once

#include <string>
#include <vector>

#include "utils/to_string.hpp"

#include "utils/json.hpp"

struct InternalString {
 public:
  InternalString() : _id(0) {}

  explicit InternalString(const std::string& str) : _id(_get_id(str)) {}
  explicit InternalString(const char* str) : _id(_get_id(str)) {}

  bool operator==(const InternalString o) const { return _id == o._id; }
  bool operator!=(const InternalString o) const { return _id != o._id; }
  bool operator<(const InternalString o) const { return _id < o._id; }
  bool operator<=(const InternalString o) const { return _id <= o._id; }
  bool operator>(const InternalString o) const { return _id > o._id; }
  bool operator>=(const InternalString o) const { return _id >= o._id; }

  const std::string& str() const;

  uint16_t id() const { return _id; }
  static uint16_t max_id();
  static InternalString from_id(uint16_t id) { return InternalString(id); }

  static void debug();

  static void wipe_ids();

  bool is_empty() const { return _id == 0; }

  bool is_valid() const;

 private:
  uint16_t _id;

  explicit InternalString(uint16_t id) : _id(id) {}

  static uint16_t _get_id(const std::string& str);
};

using WordList = std::vector<InternalString>;

namespace fmt {
template <> struct to_string<InternalString> {
  static std::string convert(const InternalString value) { return value.str(); }
};
} // namespace fmt

namespace json {
template <> struct to_json_t<InternalString> {
  static JsonValue convert(const InternalString& value);
};
} // namespace json
