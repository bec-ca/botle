
#include <cstring>
#include <iostream>
#include <map>
#include <queue>

#include "engine/game_state.hpp"
#include "engine/hash_game_state.hpp"
#include "engine/simulator.hpp"
#include "utils/format_vector.hpp"
#include "utils/json.hpp"

using namespace fmt;
using namespace std;

namespace fmt {

template <> struct to_string<json::JsonValue> {
  static std::string convert(const json::JsonValue& value)
  {
    return value.to_string();
  }
};

} // namespace fmt

namespace {

struct State {
 public:
  State(
    const vector<InternalString>& allowed_guesses,
    const vector<InternalString>& possible_secrets,
    bool hard_mode)
      : _game_state(allowed_guesses, possible_secrets, hard_mode),
        _cache_pair(1 << 24),
        _engine(_cache_pair, false),
        _simulator(_engine)
  {
    _game_state.validate_words().force();
    sort_guesses();

    _reset_thinking();
  }

  bool back()
  {
    if (_history.empty()) return false;
    _game_state = move(_history.back());
    _reset_thinking();
    _history.pop_back();
    return true;
  }

  void make_guess(InternalString guess, Match match)
  {
    assert(_is_allowed_guess(guess) && "Word is not an allowed guess");

    _history.push_back(_game_state);
    _game_state.make_guess(guess, match);

    sort_guesses();
    _reset_thinking();
  }

  OrError<optional<WordInfo>> compute_next_suggestion()
  {
    if (
      _thinking_words.size() < 100 &&
      _next_thinking_word < _game_state.allowed_guesses().size() &&
      (!_added_new_word_last || _thinking_words.empty())) {
      _thinking_words.push({
        .word = _game_state.allowed_guesses().at(_next_thinking_word),
        .max_depth = 1,
        .best_avg_guess = 0,
      });
      _next_thinking_word++;
      _added_new_word_last = true;
    } else {
      _added_new_word_last = false;
    }

    if (_thinking_words.empty()) { return nullopt; }

    WordState thinking_word = _thinking_words.top();
    _thinking_words.pop();

    bail(
      out,
      _simulator.simulate(
        _game_state, thinking_word.word, thinking_word.max_depth));

    if (!out.can_stop && thinking_word.max_depth < 16) {
      thinking_word.max_depth++;
      thinking_word.best_avg_guess =
        max(thinking_word.best_avg_guess, out.avg_guesses);
      _thinking_words.push(thinking_word);
    }

    return out;
  }

  string cache_key() { return _game_state.hash(); }

 private:
  struct WordState {
    InternalString word;
    int max_depth;
    double best_avg_guess;

    bool operator<(const WordState& other) const
    {
      return best_avg_guess > other.best_avg_guess;
    }
  };

  priority_queue<WordState> _thinking_words;
  size_t _next_thinking_word = 0;
  bool _added_new_word_last = false;

  GameState _game_state;
  CachePair _cache_pair;
  Engine _engine;
  Simulator _simulator;

  vector<GameState> _history;

  void sort_guesses()
  {
    _game_state.sort_guesses_by_greedy(
      _game_state.possible_secrets().size() <= 20);
  }

  bool _is_allowed_guess(InternalString word)
  {
    for (InternalString w : _game_state.allowed_guesses()) {
      if (w == word) return true;
    }
    return false;
  }

  void _reset_thinking()
  {
    while (!_thinking_words.empty()) { _thinking_words.pop(); }
    _next_thinking_word = 0;
  }
};

unique_ptr<State> state;

string return_string;

vector<InternalString> to_vector_internal_string(const json::JsonValue& values)
{
  vector<InternalString> out;
  const vector<json::JsonValue>& list = values.get_list();
  for (const json::JsonValue& v : list) {
    out.push_back(InternalString(v.get_string()));
  }
  return out;
}

const json::JsonValue& get_key(const string& key, const json::JsonValue& obj)
{
  const map<string, json::JsonValue>& m = obj.get_object();
  auto it = m.find(key);
  if (it == m.end()) {
    print_line("key $ not found in json", key);
    print_line("keys are:");
    for (const auto& kv : m) { print_line("'$'", kv.first); }
    assert(false && "failed to find key");
  }
  return it->second;
}

} // namespace

#ifndef _DESKTOP
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {

EMSCRIPTEN_KEEPALIVE
void load_dict(const char* dict_json)
{
  state = nullptr;
  InternalString::wipe_ids();
  Match::clear_result_cache();

  json::JsonValue dict = json::JsonValue::of_string(dict_json);

  const auto& allowed_guesses =
    to_vector_internal_string(get_key("allowed_guesses", dict));
  const auto& possible_secrets =
    to_vector_internal_string(get_key("possible_secrets", dict));
  bool hard_mode = get_key("hard_mode", dict).get_string() == "true";

  Match::init_result_cache();

  state = make_unique<State>(allowed_guesses, possible_secrets, hard_mode);
}

EMSCRIPTEN_KEEPALIVE
const char* compute_next_suggestion()
{
  auto out = state->compute_next_suggestion();
  return_string = json::to_json(out).to_string();
  return return_string.data();
}

EMSCRIPTEN_KEEPALIVE
void make_guess(const char* plain_args)
{
  auto pair = json::JsonValue::of_string(plain_args).get_list();
  auto guess = InternalString(pair[0].get_string());
  auto match = Match::parse(pair[1].get_string()).force();

  state->make_guess(guess, match);
}

EMSCRIPTEN_KEEPALIVE
bool back() { return state->back(); }

EMSCRIPTEN_KEEPALIVE
const char* cache_key()
{
  return_string = state->cache_key();
  return return_string.data();
}

// extern C
}
