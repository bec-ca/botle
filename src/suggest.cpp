#include "suggest.hpp"

#include <fstream>

#include "engine/engine.hpp"
#include "engine/game_state.hpp"
#include "engine/match.hpp"
#include "engine/simulator.hpp"
#include "utils/command.hpp"
#include "utils/error.hpp"
#include "utils/format_optional.hpp"

using namespace std;
using namespace fmt;

namespace {

OrError<Unit> suggest_guess(
  GameState game_state,
  const string& guesses_filename,
  int max_depth,
  int initial_depth)
{
  ifstream guesses_file(guesses_filename);
  if (guesses_file.bad()) {
    return Error::format("Failed to open guesses file $", guesses_filename);
  }

  auto cache_pair = make_unique<CachePair>(1 << 24);
  Engine engine(*cache_pair, false);

  int num_guesses = 0;
  string guess_str, match_str;
  while (guesses_file >> guess_str >> match_str) {
    if (guess_str.size() != 5) {
      return Error("A guess word must have 5 characters");
    }
    if (match_str.size() != 5) {
      return Error("A match must have 5 characters");
    }

    auto guess = InternalString(guess_str);
    bail(match, Match::parse(match_str));

    bail_unit(game_state.make_guess(guess, match));
    num_guesses++;

    print_line(
      "$ $, remaining_secrets: $",
      guess,
      match.str(),
      game_state.possible_secrets().size());
  }

  print_line("Candidates left: $", game_state.possible_secrets().size());

  if (game_state.possible_secrets().empty()) { return Error("No solution"); }

  game_state.sort_guesses_by_greedy(false);

  for (InternalString first_guess : game_state.allowed_guesses()) {
    print_line("----------------------------------------");
    print_line("Running $", first_guess);
    for (int depth = initial_depth; depth <= max_depth; depth++) {
      print_line("Searching at depth $", depth);
      Simulator sim(engine);
      bail(info, sim.simulate(game_state, first_guess, depth));

      print_line(
        "Depth:$ Worst case num guesses: $, avg guesses: $",
        info.first_guess,
        info.worst_num_guesses,
        info.avg_guesses);
      if (info.can_stop) break;
    }
  }

  return unit;
}

} // namespace

Command Suggest::command()
{
  auto builder = CommandBuilder("Suggest a good guess");
  auto game_state_param = GameState::param(builder);
  auto guesses_file = builder.required("--guesses-file", string_flag);
  auto max_depth = builder.optional_with_default("--max-depth", int_flag, 16);
  auto initial_depth =
    builder.optional_with_default("--initial-depth", int_flag, 1);
  return builder.run([=]() -> OrError<Unit> {
    bail(game_state, game_state_param());
    return suggest_guess(
      move(game_state),
      guesses_file->value(),
      max_depth->value(),
      initial_depth->value());
  });
}
