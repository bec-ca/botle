#include "evaluate.hpp"

#include <chrono>
#include <fstream>
#include <set>

#include "engine/dictionary.hpp"
#include "engine/engine.hpp"
#include "engine/game_state.hpp"
#include "engine/hash_game_state.hpp"
#include "engine/match.hpp"
#include "engine/simulator.hpp"
#include "utils/command.hpp"
#include "utils/error.hpp"
#include "utils/format_map.hpp"
#include "utils/format_optional.hpp"
#include "utils/format_vector.hpp"

using namespace std;
using namespace fmt;

namespace {

void print_word_info(const WordInfo& info, int idx, int max_words)
{
  print_line("---------------------------------");
  print_line("First guess: $ ($/$)", info.first_guess, idx, max_words);
  print_line("Search depth: $", info.max_depth);
  print_line("Average guesses: $", info.avg_guesses);
  print_line("Worst guesses: $", info.worst_num_guesses);
  print_line("Guesses distribution: $", info.guess_distribution);
  print_line(
    "Guesses cumulative distribution: $", info.cumulative_guess_distribution);
  print_line("Most difficult secret: $", info.most_difficult_secret);
};

bool is_better_than(const WordInfo& info1, const WordInfo& info2)
{
  if (info1.avg_guesses != info2.avg_guesses) {
    return info1.avg_guesses < info2.avg_guesses;
  } else if (info1.worst_num_guesses != info2.worst_num_guesses) {
    return info1.worst_num_guesses < info2.worst_num_guesses;
  } else if (info1.max_depth != info2.max_depth) {
    return info1.max_depth > info2.max_depth;
  } else if (info1.first_guess != info2.first_guess) {
    return info1.first_guess.str() < info2.first_guess.str();
  } else {
    return false;
  }
}

OrError<WordInfo> run_word(
  CachePair& cache_pair,
  const InternalString first_guess,
  const GameState& game_state,
  bool verbose,
  int idx,
  int max_words)
{
  Engine engine(cache_pair, verbose);

  Simulator simulator(engine);

  optional<WordInfo> best_strategy;

  auto update_strategy = [&](WordInfo&& info) {
    if (!best_strategy.has_value() || is_better_than(info, *best_strategy)) {
      best_strategy = move(info);
    }
  };

  bool should_stop = false;
  for (int max_depth = 1; max_depth < 16 && !should_stop; ++max_depth) {
    if (verbose) { print_line("Running depth $", max_depth); }
    bail(info, simulator.simulate(game_state, first_guess, max_depth));

    if (info.can_stop) {
      if (verbose) { print_line("can stop!"); }
      should_stop = true;
    }

    update_strategy(move(info));
  }

#pragma omp critical
  print_word_info(*best_strategy, idx, max_words);

  return *best_strategy;
}

OrError<Unit> evaluate(
  GameState game_state,
  bool verbose,
  int max_words,
  const string& solutions_cache_dir,
  const optional<int>& solutions_max_words,
  int cache_max_size)

{
  game_state.sort_guesses_by_greedy(false);

  max_words = min<int>(max_words, game_state.allowed_guesses().size());

  vector<optional<OrError<WordInfo>>> best_strategies_per_word(
    max_words, nullopt);

  auto cache_pair = make_unique<CachePair>(cache_max_size);

  auto cache_key = game_state.hash();

  auto write_snapshot = [&](bool show_top_strats) {
    vector<WordInfo> best_strategies;
    for (const auto& w_or_error : best_strategies_per_word) {
      if (!w_or_error.has_value()) continue;
      if (w_or_error.value().is_error()) continue;
      best_strategies.push_back(w_or_error.value().value());
    }
    sort(best_strategies.begin(), best_strategies.end(), is_better_than);
    if (show_top_strats) {
      print_line("========================================");
      print_line("Top best strategies by average guesses");
      int idx = 0;
      for (const auto& info : best_strategies) {
        print_line("--------------------------------");
        print_word_info(info, idx++, max_words);
        if (idx > 5) { break; }
      }
    }

    if (
      solutions_max_words.has_value() &&
      best_strategies.size() > size_t(*solutions_max_words)) {
      best_strategies.resize(*solutions_max_words);
    }
    auto json = json::to_json(best_strategies).to_string();

    string path = format("$/$.json", solutions_cache_dir, cache_key);
    ofstream f(path, ios::out);
    f.write(json.data(), json.size());
    assert(f.good() && "Failed to write cache");
    print_line("Wrote cache to file $", path);
  };

  auto last_wrote_snapshot = chrono::system_clock::now();

#pragma omp parallel for schedule(dynamic, 1)
  for (int idx = 0; idx < max_words; idx++) {
    const auto word = game_state.allowed_guesses()[idx];
    auto result =
      run_word(*cache_pair, word, game_state, verbose, idx, max_words);
    if (result.is_error()) { print_line("Word failed: $", result.error()); }
    best_strategies_per_word[idx] = move(result);

#pragma omp critical
    {
      auto now = chrono::system_clock::now();
      auto since_last_write = now - last_wrote_snapshot;
      if (since_last_write > 1min) {
        write_snapshot(false);
        last_wrote_snapshot = now;
      }
    }
  }

  write_snapshot(true);

  return unit;
} // namespace

} // namespace

Command Evaluate::command()
{
  auto builder = CommandBuilder("Evaluate performance of the bot");
  auto game_state_param = GameState::param(builder);
  auto verbose = builder.no_arg("--verbose");
  auto solutions_cache_dir =
    builder.required("--write-solutions-dir", string_flag);
  auto solutions_max_words =
    builder.optional("--solutions-max-words", int_flag);
  auto max_words = builder.optional_with_default("--max-words", int_flag, 100);
  auto cache_max_size =
    builder.optional_with_default("--cache-max-size", int_flag, 1 << 26);
  return builder.run([=]() -> OrError<Unit> {
    bail(game_state, game_state_param());
    return evaluate(
      move(game_state),
      verbose->value(),
      max_words->value(),
      solutions_cache_dir->value(),
      solutions_max_words->value(),
      cache_max_size->value());
  });
}
