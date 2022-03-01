#include "match.hpp"

#include <array>

#include "internal_string.hpp"
#include "utils/format.hpp"
#include "utils/format_vector.hpp"

using namespace std;

namespace {

enum class LetterResult : int8_t {
  Hit,
  Wrong_place,
  Miss,
};

char letter_result_to_char(LetterResult r)
{
  switch (r) {
  case LetterResult::Hit:
    return '!';
  case LetterResult::Wrong_place:
    return '?';
  case LetterResult::Miss:
    return 'X';
  default:
    assert(false);
  }
}

OrError<LetterResult> char_to_letter_result(char c)
{
  switch (c) {
  case '!':
    return LetterResult::Hit;
  case '?':
    return LetterResult::Wrong_place;
  case 'X':
    return LetterResult::Miss;
  default:
    return Error::format("Invalid result char: $", c);
  }
}

using Result = std::array<LetterResult, 5>;

std::string result_to_string(const Result& r)
{
  std::string output;
  for (auto l : r) { output += letter_result_to_char(l); }
  return output;
}

static LetterResult int_to_letter_result(int i)
{
  switch (i) {
  case 0:
    return LetterResult::Hit;
  case 1:
    return LetterResult::Wrong_place;
  case 2:
    return LetterResult::Miss;
  default:
    assert(false);
  }
}

Result decode(uint8_t code)
{
  Result output;
  for (int i = 0; i < 5; i++) {
    output[4 - i] = int_to_letter_result(code % 3);
    code /= 3;
  }
  return output;
}

static int _letter_result_to_int(LetterResult r)
{
  switch (r) {
  case LetterResult::Hit:
    return 0;
  case LetterResult::Wrong_place:
    return 1;
  case LetterResult::Miss:
    return 2;
  default:
    assert(false);
  }
}

static uint8_t encode(const Result& r)
{
  uint8_t code = 0;
  for (int i = 0; i < 5; i++) { code = code * 3 + _letter_result_to_int(r[i]); }
  return code;
}

Match compute_match(
  const InternalString i_guess, const InternalString i_secret_word)
{
  const string& guess = i_guess.str();
  const string& secret_word = i_secret_word.str();
  if (guess.size() != 5 || secret_word.size() != 5) { return Match(0); }
  assert(guess.size() == 5);
  assert(secret_word.size() == 5);
  Result output;
  vector<bool> used(5, false);
  for (int i = 0; i < 5; i++) {
    auto match_letter = [&]() {
      if (guess[i] == secret_word[i]) {
        return LetterResult::Hit;
      } else {
        for (int j = 0; j < 5; j++) {
          if (
            guess[i] == secret_word[j] && secret_word[j] != guess[j] &&
            !used[j]) {
            used[j] = true;
            return LetterResult::Wrong_place;
          }
        }
        return LetterResult::Miss;
      }
    };
    output[i] = match_letter();
  }
  return Match(encode(output));
}

} // namespace

Match::Match(uint8_t id) : _id(id) {}

vector<vector<Match>> Match::_cache;

std::string Match::str() const { return result_to_string(decode(_id)); }

OrError<Match> Match::parse(const std::string& str)
{
  if (str.size() != 5) { return Error("result string must have 5 characters"); }
  Result output;
  for (int i = 0; i < 5; i++) {
    bail_assign(output[i], char_to_letter_result(str[i]));
  }
  return Match(encode(output));
}

void Match::debug()
{
  fmt::print_line("------------------------------------");
  for (auto& r : _cache) { fmt::print_line(r); }
  fmt::print_line("------------------------------------");
}

void Match::clear_result_cache() { _cache.clear(); }

void Match::init_result_cache()
{
  size_t max_id = InternalString::max_id();

  _cache.clear();
  _cache.reserve(max_id);

  for (size_t i = 0; i < max_id; i++) {
    vector<Match> row;
    row.reserve(max_id);
    for (size_t j = 0; j < max_id; j++) {
      row.push_back(
        compute_match(InternalString::from_id(i), InternalString::from_id(j)));
    }
    _cache.emplace_back(move(row));
  }
}

Match Match::match_uncached(InternalString guess, InternalString secret)
{
  return compute_match(guess, secret);
}

vector<InternalString> Match::eliminate_words(
  const vector<InternalString>& candidates, InternalString guess) const
{
  vector<InternalString> output;
  for (const auto& word : candidates) {
    if (Match::match(guess, word) == *this) output.push_back(word);
  }
  return output;
}

namespace json {
JsonValue to_json_t<Match>::convert(const Match value)
{
  return JsonValue(value.str());
}
} // namespace json
