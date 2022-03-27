#include "suggest.hpp"

#include <csignal>
#include <fstream>
#include <iostream>

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

bool sig_int_received = false;

void handle_sig_int(int) { sig_int_received = true; }

OrError<Unit> suggest_guess(
  GameState game_state,
  const optional<string>& guesses_filename,
  int max_depth,
  int initial_depth)
{
  auto cache_pair = make_unique<CachePair>(1 << 26);

  signal(SIGINT, handle_sig_int);

  int num_done = 0;

  int go_back_lines = 0;
  auto show_progress = [&](
                         const map<InternalString, WordInfo>& suggestions,
                         InternalString first_guess,
                         int depth) {
    vector<WordInfo> words;
    for (auto& info : suggestions) { words.push_back(info.second); }

    stable_sort(
      words.begin(), words.end(), [](const auto& el1, const auto& el2) {
        if (el1.avg_guesses != el2.avg_guesses) {
          return el1.avg_guesses < el2.avg_guesses;
        } else {
          return el1.first_guess.str() < el2.first_guess.str();
        }
      });

    vector<string> lines;

    lines.push_back(format(
      "\x1b[?25l\x1b[$A",
      go_back_lines)); // move cursor up and hide cursor
    lines.push_back(format("Updated $ depth:$", first_guess, depth));
    lines.push_back(
      format("Done $/$", num_done, game_state.allowed_guesses().size()));
    lines.push_back("Top words so far:");
    for (int i = 0; i < min<int>(32, words.size()); i++) {
      auto& info = words[i];
      lines.push_back(format(
        "Word:$ Worst case num guesses: $, avg guesses: $",
        info.first_guess,
        info.worst_num_guesses,
        info.avg_guesses));
    }
    lines.push_back("----------------------------------------\x1b[?25h");

    string output;
    for (auto& l : lines) {
      output += l;
      output += '\n';
    }
    cout << output << flush;
    go_back_lines = lines.size();
  };

  auto suggest = [&]() -> OrError<Unit> {
    if (game_state.possible_secrets().empty()) { return Error("No solution"); }

    game_state.sort_guesses_by_greedy(false);

    map<InternalString, WordInfo> suggestions;

    sig_int_received = false;

#pragma omp parallel for schedule(dynamic, 1)
    for (InternalString first_guess : game_state.allowed_guesses()) {
      if (sig_int_received) continue;
      optional<WordInfo> best_sol;
      Engine engine(*cache_pair, false);
      for (int depth = initial_depth; depth <= max_depth; depth++) {
        if (sig_int_received) break;
        Simulator sim(engine);
        auto info_or_error = sim.simulate(game_state, first_guess, depth);
        if (info_or_error.is_error()) {
          print_line("Search failed: $", info_or_error.error());
          break;
        }
        auto& info = info_or_error.value();

        if (!best_sol.has_value() || info.avg_guesses < best_sol->avg_guesses) {
          best_sol = info;
#pragma omp critical
          {
            suggestions[first_guess] = info;
            show_progress(suggestions, first_guess, depth);
          }
        }
        if (info.can_stop) break;
      }
#pragma omp critical
      {
        num_done++;
      }
    }

    if (sig_int_received) {
      print_line("Thinking interruped");
    } else {
      print_line("Done thinking");
    }

    return unit;
  };

  auto read_one = [&](istream& stream) -> OrError<bool> {
    string guess_str, match_str;
    if (!(stream >> guess_str)) return false;

    if (guess_str == "go") {
      bail_unit(suggest());
      return true;
    }

    if (!(stream >> match_str)) return false;

    if (guess_str.size() != 5) {
      return Error("A guess word must have 5 characters");
    }
    if (match_str.size() != 5) {
      return Error("A match must have 5 characters");
    }

    auto guess = InternalString(guess_str);
    bail(match, Match::parse(match_str));

    bail_unit(game_state.make_guess(guess, match));

    print_line(
      "$ $, remaining_secrets: $",
      guess,
      match.str(),
      game_state.possible_secrets().size());

    print_line("Sample remaining secrets:");
    for (int i = 0; i < min<int>(32, game_state.possible_secrets().size());
         i++) {
      print_line(game_state.possible_secrets()[i]);
    }

    return true;
  };

  if (guesses_filename.has_value()) {
    // offline mode
    ifstream guesses_file(*guesses_filename);
    if (guesses_file.bad()) {
      return Error::format("Failed to open guesses file $", *guesses_filename);
    }

    string guess_str, match_str;
    while (true) {
      bail(got_one, read_one(guesses_file));
      if (!got_one) break;
    }

    return suggest();
  } else {
    // interactive mode
    while (true) {
      auto got_one = read_one(cin);
      if (got_one.is_error()) {
        print_line(got_one.error());
      } else if (!got_one.value()) {
        break;
      }
    }

    return unit;
  }
}

} // namespace

Command Suggest::command()
{
  auto builder = CommandBuilder("Suggest a good guess");
  auto game_state_param = GameState::param(builder);
  auto guesses_file = builder.optional("--guesses-file", string_flag);
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
