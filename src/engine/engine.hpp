#pragma once

#include <map>
#include <set>

#include "cache.hpp"
#include "game_state.hpp"
#include "greedy.hpp"

struct MultiSearchContext;

struct CachePair {
  Cache min_cache;
  Cache max_cache;
  CachePair(size_t max_size);
  ~CachePair();
};

struct Engine {
 public:
  Engine(CachePair& cache, bool verbose);

  OrError<SearchResult> search(const GameState& game_state, int max_depth);

  void debug();

  void set_verbose(bool verbose);

  bool get_verbose() const;

 private:
  CachePair& _cache_pair;

  bool _verbose;
  bool _hard_mode;

  std::vector<int> rank_distribution;

  struct MaxSearchResult {
    MaxSearchResult(int num_guesses, bool is_optimal)
        : num_guesses(num_guesses), is_optimal(is_optimal)
    {}

    int num_guesses;
    bool is_optimal;
  };

  MaxSearchResult max_search(
    const std::vector<InternalString>& allowed_guesses,
    const std::vector<InternalString>& possible_secrets,
    const InternalString guess_candidate,
    int max_depth,
    int alpha,
    int beta);

  SearchResult min_search(
    const std::vector<InternalString>& allowed_guesses,
    const std::vector<InternalString>& possible_secrets,
    int max_depth,
    int alpha,
    int beta,
    bool is_root);

  void add_rank(int rank);
};
