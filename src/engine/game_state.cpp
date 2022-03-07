#include "game_state.hpp"

#include "dictionary.hpp"
#include "greedy.hpp"
#include "hash_game_state.hpp"
#include "utils/command.hpp"

#include <algorithm>
#include <array>
#include <set>
#include <vector>

using namespace std;
using namespace fmt;

namespace {

vector<InternalString> sort_words_by_greedy(
  const vector<InternalString>& allowed_guesses,
  const vector<InternalString>& possible_secrets,
  bool possible_secrets_first)
{
  vector<InternalString> sorted_candidates;
  sorted_candidates.reserve(allowed_guesses.size());

  vector<int> score(InternalString::max_id(), 0);
  for (const auto& guess_candidate : allowed_guesses) {
    auto s = worst_remaining_possible_after_one_guess(
               guess_candidate, possible_secrets, int(possible_secrets.size()))
               .first;
    score.at(guess_candidate.id()) = s;
    sorted_candidates.push_back(guess_candidate);
  }
  if (possible_secrets_first) {
    for (const auto& possible_secret : possible_secrets) {
      score.at(possible_secret.id()) -= 1000000;
    }
  }
  sort(
    sorted_candidates.begin(),
    sorted_candidates.end(),
    [&](const auto& c1, const auto& c2) {
      return score.at(c1.id()) < score.at(c2.id());
    });

  return sorted_candidates;
}

OrError<vector<InternalString>> get_possible_secrets(
  const optional<string>& possible_secrets_file,
  const vector<InternalString>& allowed_guesses)
{
  if (possible_secrets_file.has_value()) {
    bail(secrets, Dictionary::load_words(*possible_secrets_file));
    print_line("Dictionary of secret words contains $ words", secrets.size());
    return secrets;
  } else {
    print_line("No dictionary of secret words provided, using the entire "
               "list of allowed guesses");
    return allowed_guesses;
  }
}

} // namespace

GameState::GameState(
  const WordList& allowed_guesses,
  const WordList& possible_secrets,
  bool hard_mode)
    : _allowed_guesses(allowed_guesses),
      _possible_secrets(possible_secrets),
      _hard_mode(hard_mode)
{
  if (_possible_secrets.empty()) { _possible_secrets = allowed_guesses; }
}

GameState::~GameState() {}

const WordList& GameState::allowed_guesses() const { return _allowed_guesses; }
const WordList& GameState::possible_secrets() const
{
  return _possible_secrets;
}

void GameState::drop_useless_guesses()
{
  if (!_hard_mode) {
    set<uint32_t> seen_hashes;

    vector<InternalString> filtered;
    for (InternalString word : _allowed_guesses) {
      auto hash = hash_remaining_secrets(word, _possible_secrets);
      if (seen_hashes.find(hash) != seen_hashes.end()) { continue; }
      seen_hashes.insert(hash);
      filtered.push_back(word);
    }
    _allowed_guesses = move(filtered);
  }
}

OrError<Unit> GameState::make_guess(InternalString guess, Match match)
{
  bool has_guess = false;
  for (auto allowed_guess : _allowed_guesses) {
    if (allowed_guess == guess) {
      has_guess = true;
      break;
    }
  }
  if (!has_guess) {
    return Error::format("The word $ is not an allowed guess", guess);
  }

  auto possible_secrets = match.eliminate_words(_possible_secrets, guess);
  if (possible_secrets.empty()) {
    return Error("There would be no remaing secrets with this guess");
  }
  _possible_secrets = move(possible_secrets);
  if (_hard_mode) {
    _allowed_guesses = match.eliminate_words(_allowed_guesses, guess);
  }
  drop_useless_guesses();

  return unit;
}

void GameState::sort_guesses_by_greedy(bool possible_secrets_first)
{
  _allowed_guesses = sort_words_by_greedy(
    _allowed_guesses, _possible_secrets, possible_secrets_first);
}

OrError<Unit> GameState::validate_words()
{
  set<InternalString> g(_allowed_guesses.begin(), _allowed_guesses.end());
  for (const auto& s : _possible_secrets) {
    if (g.find(s) == g.end()) {
      return Error::format("Secret missing from set of allowed guesses: $", s);
    }
  }

  for (InternalString w : _allowed_guesses) {
    if (w.str().size() != 5) {
      return Error::format("Allowed guess must have 5 letters, got '$'", w);
    }
  }

  for (InternalString w : _possible_secrets) {
    if (w.str().size() != 5) {
      return Error::format("Secret must have 5 letters, got '$'", w);
    }
  }
  return unit;
}

function<OrError<GameState>()> GameState::param(CommandBuilder& builder)
{
  auto allowed_guesses_file =
    builder.required("--allowed-guesses-file", string_flag);
  auto possible_secrets_file =
    builder.optional("--possible-secrets-file", string_flag);
  auto hard_mode = builder.no_arg("--hard");

  return [=]() -> OrError<GameState> {
    bail(
      allowed_guesses, Dictionary::load_words(allowed_guesses_file->value()));
    print_line("Allowed guesses has $ words", allowed_guesses.size());

    bail(
      possible_secrets,
      get_possible_secrets(possible_secrets_file->value(), allowed_guesses));
    GameState game_state(allowed_guesses, possible_secrets, hard_mode->value());
    bail_unit(game_state.validate_words());
    return game_state;
  };
}

string GameState::hash() const
{
  return hash_game_state(_allowed_guesses, _possible_secrets, _hard_mode);
}

bool GameState::is_hard_mode() const { return _hard_mode; }

array<Partition, 256> GameState::partition_by_pattern(
  InternalString guess) const
{
  array<Partition, 256> buckets;
  for (int i = 0; i < 256; i++) { buckets[i]._match = Match(i); }
  for (InternalString possible_secret : _possible_secrets) {
    auto r = Match::match(guess, possible_secret);
    auto& b = buckets.at(r.code());
    b._possible_secrets.push_back(possible_secret);
  }

  if (_hard_mode) {
    for (InternalString allowed_guess : _allowed_guesses) {
      auto r = Match::match(guess, allowed_guess);
      buckets[r.code()]._allowed_guesses.push_back(allowed_guess);
    }
  }
  return buckets;
}

GameState GameState::state_from_partition(const Partition& partition) const
{
  auto state = GameState(
    _hard_mode ? partition._allowed_guesses : _allowed_guesses,
    partition._possible_secrets,
    _hard_mode);
  state.drop_useless_guesses();
  return state;
}
