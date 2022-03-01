#include "engine.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "cache.hpp"
#include "greedy.hpp"
#include "match.hpp"
#include "utils/format_map.hpp"
#include "utils/format_optional.hpp"
#include "utils/format_set.hpp"
#include "utils/format_vector.hpp"

using namespace std;
using namespace fmt;

namespace {

struct Bucket {
  vector<InternalString> allowed_guesses;
  vector<InternalString> possible_secrets;
  Match match = Match(0);
};

} // namespace

Engine::Engine(CachePair& cache_pair, bool verbose)
    : _cache_pair(cache_pair), _verbose(verbose)
{}

Engine::MaxSearchResult Engine::max_search(
  const vector<InternalString>& allowed_guesses,
  const vector<InternalString>& possible_secrets,
  const InternalString guess_candidate,
  int max_depth,
  int alpha,
  int beta)
{
  auto cache_key =
    _cache_pair.max_cache.max_search_key(guess_candidate, possible_secrets);
  {
    const auto cache_entry = _cache_pair.max_cache.find(max_depth, cache_key);

    if (cache_entry.lower_bound >= beta) {
      return MaxSearchResult(
        cache_entry.lower_bound, cache_entry.lower_bound_optimal);
    }
    if (cache_entry.upper_bound <= alpha) {
      return MaxSearchResult(
        cache_entry.upper_bound, cache_entry.upper_bound_optimal);
    }
  }

  auto do_it = [&]() -> MaxSearchResult {
    array<Bucket, 256> buckets;
    for (int i = 0; i < 256; i++) { buckets[i].match = Match(i); }
    size_t largest = 0;
    for (const auto possible_secret : possible_secrets) {
      auto r = Match::match(guess_candidate, possible_secret);
      auto& b = buckets[r.code()];
      b.possible_secrets.push_back(possible_secret);
      if (b.possible_secrets.size() > largest) {
        largest = b.possible_secrets.size();
      }
    }
    if (_hard_mode) {
      for (InternalString allowed_guess : allowed_guesses) {
        auto r = Match::match(guess_candidate, allowed_guess);
        buckets[r.code()].allowed_guesses.push_back(allowed_guess);
      }
    }
    if (largest == possible_secrets.size()) {
      return MaxSearchResult(beta, true);
    }
    if (int(largest) <= alpha) { return MaxSearchResult(alpha, true); }
    if (largest <= 2) { return MaxSearchResult(largest, true); }
    if (beta <= 2) { return MaxSearchResult(beta, true); }
    sort(buckets.begin(), buckets.end(), [](const auto& b1, const auto& b2) {
      return b1.possible_secrets.size() > b2.possible_secrets.size();
    });
    int best_num_guesses = -1;
    bool is_optimal = true;
    assert(!buckets[0].possible_secrets.empty());
    for (const auto& b : buckets) {
      if (
        b.possible_secrets.empty() ||
        int(b.possible_secrets.size()) <= best_num_guesses) {
        break;
      }
      auto res = min_search(
        _hard_mode ? b.allowed_guesses : allowed_guesses,
        b.possible_secrets,
        max_depth - 1,
        best_num_guesses,
        beta,
        false);
      if (res.num_guesses > best_num_guesses) {
        best_num_guesses = res.num_guesses;
        is_optimal = res.is_optimal;
        if (best_num_guesses >= beta) {
          return MaxSearchResult(beta, is_optimal);
        }
      }
    }
    assert(best_num_guesses > 0);
    return MaxSearchResult(best_num_guesses, is_optimal);
  };

  auto ret = do_it();
  _cache_pair.max_cache.update(
    max_depth,
    cache_key,
    ret.num_guesses,
    alpha,
    beta,
    InternalString(),
    ret.is_optimal);

  return ret;
}

int one_letter_diff(InternalString w1, InternalString w2)
{
  int diff_letter = -1;
  int count_diff_letter = 0;
  const auto& s1 = w1.str();
  const auto& s2 = w2.str();
  for (int i = 0; i < 5; i++) {
    if (s1[i] != s2[i]) {
      diff_letter = i;
      count_diff_letter++;
      if (count_diff_letter > 1) { return -1; }
    }
  }
  return diff_letter;
}

bool are_all_same_one_letter_diff(const vector<InternalString>& words)
{
  if (words.size() <= 1) { return false; }
  int diff_letter = one_letter_diff(words[0], words[1]);
  if (diff_letter == -1) { return false; }
  for (size_t i = 2; i < words.size(); i++) {
    if (one_letter_diff(words[0], words[i]) != diff_letter) { return false; }
  }
  return true;
}

SearchResult Engine::min_search(
  const vector<InternalString>& allowed_guesses,
  const vector<InternalString>& possible_secrets,
  const int max_depth,
  const int alpha,
  const int beta,
  const bool is_root)
{
  assert(possible_secrets.size() > 0);

  if (possible_secrets.size() <= 2 || beta <= 2) {
    return SearchResult{
      .best_guess = possible_secrets[0],
      .num_guesses = min<int>(2, possible_secrets.size()),
      .is_optimal = true,
    };
  }

  if (alpha >= int(possible_secrets.size())) {
    return SearchResult{
      .best_guess = possible_secrets[0],
      .num_guesses = alpha,
      .is_optimal = true,
    };
  }

  if (max_depth <= 0) {
    return SearchResult{
      .best_guess = possible_secrets[0],
      .num_guesses = int(possible_secrets.size()),
      .is_optimal = false,
    };
  }

  CacheKey cache_key = _cache_pair.min_cache.min_search_key(possible_secrets);
  auto cache_entry = _cache_pair.min_cache.find(max_depth, cache_key);

  if (
    cache_entry.lower_bound >= beta &&
    (!is_root || !cache_entry.lower_bound_word.is_empty())) {
    return SearchResult{
      .best_guess = cache_entry.lower_bound_word,
      .num_guesses = cache_entry.lower_bound,
      .is_optimal = cache_entry.lower_bound_optimal,
    };
  }

  if (
    (cache_entry.upper_bound <= alpha ||
     cache_entry.upper_bound == cache_entry.lower_bound) &&
    (!cache_entry.upper_bound_word.is_empty())) {
    return SearchResult{
      .best_guess = cache_entry.upper_bound_word,
      .num_guesses = cache_entry.upper_bound,
      .is_optimal = cache_entry.upper_bound_optimal,
    };
  }

  auto result = [&]() {
    auto shallow_cache_entry =
      _cache_pair.min_cache.find(max_depth - 1, cache_key);
    auto previous_best_word = shallow_cache_entry.upper_bound_word;
    if (
      shallow_cache_entry.lower_bound == shallow_cache_entry.upper_bound &&
      shallow_cache_entry.lower_bound_optimal) {
      return SearchResult{
        .best_guess = shallow_cache_entry.lower_bound_word,
        .num_guesses = shallow_cache_entry.lower_bound,
        .is_optimal = true,
      };
    }

    if (max_depth <= 1) {
      return pick_greedy_guess(allowed_guesses, possible_secrets, beta);
    }

    vector<pair<int, InternalString>> sorted_candidates;
    {
      const int how_many_to_try =
        _hard_mode
          ? max(20, min<int>(80, 20 + (allowed_guesses.size() + 1) / 2))
          : 100;
      sorted_candidates.reserve(how_many_to_try + 1);

      int min_score = 10000000;

      int beta_remaining = possible_secrets.size();
      for (const auto& guess_candidate : allowed_guesses) {
        auto s = worst_remaining_possible_after_one_guess(
                   guess_candidate, possible_secrets, beta_remaining)
                   .first;
        if (previous_best_word == guess_candidate) { s = -10; }
        if (s >= beta_remaining) continue;
        min_score = min(s, min_score);
        sorted_candidates.emplace_back(s, guess_candidate);
        push_heap(sorted_candidates.begin(), sorted_candidates.end());
        if (int(sorted_candidates.size()) > how_many_to_try) {
          pop_heap(sorted_candidates.begin(), sorted_candidates.end());
          beta_remaining = min(beta_remaining, sorted_candidates.back().first);
          sorted_candidates.pop_back();
        }
        if (min_score <= 1) { break; }
      }

      sort_heap(sorted_candidates.begin(), sorted_candidates.end());
    }

    assert(!sorted_candidates.empty());

    bool is_optimal = true;
    int tried = 0;
    int best_rank = -1;
    InternalString best_guess;
    int best_num_guesses = possible_secrets.size();
    for (const auto& guess_candidate_pair : sorted_candidates) {
      const InternalString guess_candidate = guess_candidate_pair.second;
      assert(
        guess_candidate.is_valid() &&
        "Got invalid candidate, how did this even happen?");
      auto res = max_search(
        allowed_guesses,
        possible_secrets,
        guess_candidate,
        max_depth,
        max(1, alpha - 1),
        max(1, best_num_guesses - 1));
      assert(res.num_guesses > 0);
      int num_guesses = res.num_guesses + 1;
      if (num_guesses < best_num_guesses || best_guess.is_empty()) {
        best_rank = tried + 1;
        best_guess = guess_candidate;
        best_num_guesses = num_guesses;
        is_optimal = res.is_optimal;
        if (is_root && _verbose) {
          print_line(
            "guess:$ best_num_guesses:$ "
            "rank:$",
            best_guess,
            best_num_guesses,
            tried + 1);
          print_line("-------------------------------");
        }
        if (best_num_guesses <= alpha) { break; }
        if (best_num_guesses <= 2) { break; }
      }
      tried++;
    }

    assert(!best_guess.is_empty());

    if (best_rank >= 0) { add_rank(best_rank); }
    assert(best_num_guesses > 0);
    return SearchResult{
      .best_guess = best_guess,
      .num_guesses = best_num_guesses,
      .is_optimal = is_optimal,
    };
  }();

  assert(result.num_guesses > 0);

  _cache_pair.min_cache.update(
    max_depth,
    cache_key,
    result.num_guesses,
    alpha,
    beta,
    result.best_guess,
    result.is_optimal);

  return result;
}

OrError<SearchResult> Engine::search(const GameState& game_state, int max_depth)
{
  assert(max_depth > 0);
  _hard_mode = game_state.is_hard_mode();
  auto result = min_search(
    game_state.allowed_guesses(),
    game_state.possible_secrets(),
    max_depth,
    0,
    game_state.possible_secrets().size(),
    true);
  if (result.best_guess.is_empty()) {
    return Error::format(
      "Engine failed to find a word $ $",
      result.best_guess,
      result.best_guess.id());
  }
  return result;
}

void Engine::add_rank(int rank)
{
  if (rank_distribution.size() <= size_t(rank)) {
    rank_distribution.resize(rank + 1, 0);
  }
  rank_distribution.at(rank)++;
}

void Engine::set_verbose(bool verbose) { _verbose = verbose; }
bool Engine::get_verbose() const { return _verbose; }

void Engine::debug()
{
  map<int, int> v;
  int total = 0;
  for (size_t i = 0; i < rank_distribution.size(); i++) {
    if (rank_distribution[i] == 0) continue;
    v.emplace(i, rank_distribution[i]);
    total += rank_distribution[i];
  }
  map<int, double> d;
  int acc = 0;
  for (size_t i = 0; i < rank_distribution.size(); i++) {
    if (rank_distribution[i] == 0) continue;
    acc += rank_distribution[i];
    d.emplace(i, double(acc) * 100.0 / total);
  }
  fmt::print_line("Distribution of best word rank: $", v);
  fmt::print_line("Distribution of best word rank: $", d);
}

////////////////////////////////////////////////////////////////////////////////
// CachePair
//

CachePair::CachePair(size_t max_size) : min_cache(max_size), max_cache(max_size)
{}

CachePair::~CachePair() {}
