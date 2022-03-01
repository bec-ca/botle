#pragma once

#include <map>
#include <vector>

#include "engine.hpp"
#include "engine/game_state.hpp"
#include "internal_string.hpp"
#include "utils/to_json.hpp"

struct WordInfo {
  InternalString first_guess;
  int sum_num_guesses;
  double avg_guesses;
  int worst_num_guesses;
  std::map<int, int> guess_distribution;
  std::map<int, int> cumulative_guess_distribution;
  InternalString most_difficult_secret;
  int max_depth;

  std::vector<Match> all_matches;

  bool can_stop;

  ~WordInfo();
};

struct Simulator {
 public:
  Simulator(Engine& engine);

  ~Simulator();

  OrError<WordInfo> simulate(
    const GameState& game_state, InternalString first_guess, int max_depth);

 private:
  Engine& _engine;
};

namespace json {

template <> struct to_json_t<WordInfo> {
  static JsonValue convert(const WordInfo& value);
};

} // namespace json
