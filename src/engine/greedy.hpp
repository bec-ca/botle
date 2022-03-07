#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "internal_string.hpp"
#include "match.hpp"

struct SearchResult {
  InternalString best_guess;
  int num_guesses;
  bool is_optimal;
};

std::pair<int, InternalString> worst_remaining_possible_after_one_guess(
  const InternalString guess_candidate,
  const std::vector<InternalString>& possible_secrets,
  int beta);

SearchResult pick_greedy_guess(
  const std::vector<InternalString>& allowed_guesses,
  const std::vector<InternalString>& possible_secrets,
  int beta);

uint32_t hash_remaining_secrets(
  const InternalString guess_candidate,
  const std::vector<InternalString>& possible_secrets);
