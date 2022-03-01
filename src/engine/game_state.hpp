#pragma once

#include "internal_string.hpp"
#include "match.hpp"
#include "utils/command.hpp"

#include <functional>
#include <vector>

struct Partition {
 public:
  bool is_empty() const { return _possible_secrets.empty(); }
  Match match() const { return _match; }

 private:
  WordList _allowed_guesses;
  WordList _possible_secrets;
  Match _match = Match(0);

  friend struct GameState;
};

struct GameState {
 public:
  GameState(
    const WordList& allowed_guesses,
    const WordList& possible_secrets,
    bool hard_mode);

  ~GameState();

  const WordList& allowed_guesses() const;
  const WordList& possible_secrets() const;

  OrError<Unit> make_guess(InternalString guess, Match match);

  void sort_guesses_by_greedy(bool possible_secrets_first);

  OrError<Unit> validate_words();

  static std::function<OrError<GameState>()> param(CommandBuilder& builder);

  std::string hash() const;

  bool is_hard_mode() const;

  std::array<Partition, 256> partition_by_pattern(InternalString guess) const;

  GameState state_from_partition(const Partition& partition) const;

 private:
  WordList _allowed_guesses;
  WordList _possible_secrets;
  bool _hard_mode;
};
