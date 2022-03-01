#include "hash_game_state.hpp"

#include "utils/format.hpp"
#include "utils/hash.hpp"

#include <algorithm>

using namespace std;

string hash_game_state(
  const std::vector<InternalString>& allowed_guesses,
  const std::vector<InternalString>& remaining_secrets,
  bool hard_mode)
{
  Hash hash;
  auto feed_strings = [&](const vector<InternalString>& input) {
    vector<string> v;
    v.reserve(input.size());
    for (InternalString w : input) { v.push_back(w.str()); }

    sort(v.begin(), v.end());
    for (const string& s : v) {
      hash.feed_string(s);
      hash.feed_char('|');
    }
    hash.feed_char('#');
  };

  feed_strings(allowed_guesses);
  feed_strings(remaining_secrets);

  hash.feed_char(hard_mode ? 'h' : 'e');

  return hash.finalize().hex();
}
