#pragma once

#include <cassert>
#include <string>

#include "internal_string.hpp"
#include "utils/error.hpp"
#include "utils/to_json.hpp"

struct Match {
 public:
  bool operator==(const Match o) const { return _id == o._id; }
  bool operator!=(const Match o) const { return _id != o._id; }
  bool operator<(const Match o) const { return _id < o._id; }
  bool operator<=(const Match o) const { return _id <= o._id; }
  bool operator>(const Match o) const { return _id > o._id; }
  bool operator>=(const Match o) const { return _id >= o._id; }

  bool is_all_hit() const { return _id == 0; }

  uint8_t code() const { return _id; }

  std::string str() const;

  static OrError<Match> parse(const std::string& str);

  explicit Match(uint8_t id);

  static inline Match match(InternalString guess, InternalString secret)
  {
    return _cache[guess.id()][secret.id()];
  }

  static Match match_uncached(InternalString guess, InternalString secret_word);

  static void init_result_cache();

  static void clear_result_cache();

  std::vector<InternalString> eliminate_words(
    const std::vector<InternalString>& candidates, InternalString guess) const;

  static void debug();

 private:
  uint8_t _id;

  static std::vector<std::vector<Match>> _cache;
};

namespace fmt {
template <> struct to_string<Match> {
  static std::string convert(const Match value) { return value.str(); }
};
} // namespace fmt

namespace json {
template <> struct to_json_t<Match> {
  static JsonValue convert(const Match value);
};
} // namespace json
