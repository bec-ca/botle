#include "greedy.hpp"

#include "match.hpp"

#include <array>
#include <string.h>

using namespace std;

pair<int, InternalString> worst_remaining_possible_after_one_guess(
  const InternalString guess_candidate,
  const vector<InternalString>& possible_secrets,
  int beta)
{
  assert(possible_secrets.size() > 0);

  array<int, 256> buckets;
  for (auto& v : buckets) v = 0;
  int worst_remaining_secrets = 0;
  optional<InternalString> worst_secret;
  for (const auto& possible_secret : possible_secrets) {
    auto r = Match::match(guess_candidate, possible_secret);
    if (guess_candidate == possible_secret) continue;
    int& count = buckets.at(r.code());
    count++;
    if (count > worst_remaining_secrets) {
      worst_remaining_secrets = count;
      worst_secret = possible_secret;
      if (worst_remaining_secrets >= beta) break;
    }
  }
  return make_pair(worst_remaining_secrets, *worst_secret);
}

SearchResult pick_greedy_guess(
  const vector<InternalString>& allowed_guesses,
  const vector<InternalString>& possible_secrets,
  int beta)
{
  assert(possible_secrets.size() > 0);
  assert(allowed_guesses.size() > 0);

  if (possible_secrets.size() == 1) {
    return SearchResult{
      .best_guess = possible_secrets[0],
      .num_guesses = 1,
      .is_optimal = true,
    };
  }

  InternalString guess;
  int best_worst_remaining_secrets = beta;
  optional<Match> worst_match;
  optional<InternalString> worst_secret;
  for (const auto& guess_candidate : allowed_guesses) {
    auto worst_remaining_secrets = worst_remaining_possible_after_one_guess(
      guess_candidate, possible_secrets, best_worst_remaining_secrets);
    if (worst_remaining_secrets.first < best_worst_remaining_secrets) {
      guess = guess_candidate;
      best_worst_remaining_secrets = worst_remaining_secrets.first;
      worst_secret = worst_remaining_secrets.second;
      worst_match =
        Match::match(guess_candidate, worst_remaining_secrets.second);
      if (best_worst_remaining_secrets <= 1) { break; }
    }
  }
  int num_guesses = best_worst_remaining_secrets + 1;
  return SearchResult{
    .best_guess = guess,
    .num_guesses = num_guesses,
    .is_optimal = num_guesses <= 2,
  };
}
