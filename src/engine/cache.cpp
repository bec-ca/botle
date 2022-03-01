#include "cache.hpp"

#include "utils/format.hpp"

#include <mutex>
#include <unordered_map>

using namespace std;

namespace {

vector<CacheKey> init_word_hashes()
{
  vector<CacheKey> word_hashes;
  word_hashes.resize(InternalString::max_id());

  std::mt19937_64 rng(1);
  for (auto& h : word_hashes) {
    h.lower_hash = rng();
    h.upper_hash = rng();
  }
  return word_hashes;
}

array<CacheKey, 40> init_depth_hashes()
{
  array<CacheKey, 40> depth_hashes;
  std::mt19937_64 rng(42);
  for (auto& h : depth_hashes) {
    h.lower_hash = rng();
    h.upper_hash = rng();
  }
  return depth_hashes;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Cache
//

Cache::Cache(size_t max_size)
    : _max_size(max_size / _cache_segments),
      _word_hashes(init_word_hashes()),
      _first_word_hashes(init_word_hashes()),
      _depth_hashes(init_depth_hashes())
{}

Cache::~Cache() {}

CacheEntry& Cache::_find(int depth, const CacheKey& given_key, size_t segment)
{
  auto key = given_key ^ _depth_hashes[depth];
  auto& c = _cache[segment];
  auto it = c.find(key);
  if (it != c.end()) { return it->second; }
  if ((c.size() + 1) % (1 << 20) == 0) {
    fmt::print_line("Cache size: $", c.size());
  }
  if (c.size() >= _max_size) {
    size_t counter = 0;
    for (auto it = c.begin(); it != c.end();) {
      if ((counter++) % 2 == 0) {
        it = c.erase(it);
      } else {
        ++it;
      }
    }
  }
  return c.emplace(key, CacheEntry()).first->second;
}

const CacheEntry Cache::find(int depth, const CacheKey& key)
{
  assert(depth < max_depth);
  size_t segment = key.lower_hash % _cache_segments;
#ifdef _DESKTOP
  lock_guard<mutex> lock(_mutex[segment]);
#endif
  return _find(depth, key, segment);
}

void Cache::update(
  int depth,
  const CacheKey& key,
  int num_guesses,
  int alpha,
  int beta,
  InternalString word,
  bool optimal)
{
  assert(depth < max_depth);
  size_t segment = key.lower_hash % _cache_segments;
#ifdef _DESKTOP
  lock_guard<mutex> lock(_mutex[segment]);
#endif
  _find(depth, key, segment).update(num_guesses, alpha, beta, word, optimal);
}

////////////////////////////////////////////////////////////////////////////////
// CacheKey
//

CacheKey Cache::min_search_key(const std::vector<InternalString>& words)
{
  CacheKey hash;
  for (const auto w : words) { hash ^= _word_hashes.at(w.id()); }
  return hash;
}

CacheKey Cache::max_search_key(
  InternalString first_word, const std::vector<InternalString>& words)
{
  return min_search_key(words) ^ _first_word_hashes.at(first_word.id());
}

void CacheEntry::update(
  uint16_t num_guesses,
  uint16_t alpha,
  uint16_t beta,
  InternalString word,
  bool optimal)
{
  if (num_guesses >= beta) {
    lower_bound = max(lower_bound, beta);
    lower_bound_word = word;
    lower_bound_optimal = optimal;
  } else if (num_guesses <= alpha) {
    upper_bound = min(upper_bound, alpha);
    upper_bound_word = word;
    upper_bound_optimal = optimal;
  } else {
    lower_bound = upper_bound = num_guesses;
    lower_bound_word = upper_bound_word = word;
    lower_bound_optimal = upper_bound_optimal = optimal;
  }
}
