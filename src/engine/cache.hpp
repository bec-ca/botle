#pragma once

#include <array>
#include <limits>
#include <mutex>
#include <optional>
#include <random>
#include <unordered_map>
#include <vector>

#include "internal_string.hpp"

struct CacheKey {
  uint64_t lower_hash = 0;
  uint64_t upper_hash = 0;

  CacheKey& operator^=(const CacheKey& other)
  {
    lower_hash ^= other.lower_hash;
    upper_hash ^= other.upper_hash;
    return *this;
  }

  CacheKey operator^(const CacheKey& other) const
  {
    return CacheKey{
      .lower_hash = lower_hash ^ other.lower_hash,
      .upper_hash = upper_hash ^ other.upper_hash,
    };
  }

  bool operator==(const CacheKey& other) const
  {
    return lower_hash == other.lower_hash && upper_hash == other.upper_hash;
  }
  struct Hasher {
    size_t operator()(const CacheKey& k) const
    {
      return k.lower_hash ^ k.upper_hash;
    }
  };
};

struct CacheEntry {
 public:
  uint16_t lower_bound = 0;
  uint16_t upper_bound = std::numeric_limits<uint16_t>::max();
  InternalString lower_bound_word;
  InternalString upper_bound_word;
  bool lower_bound_optimal = false;
  bool upper_bound_optimal = false;

 private:
  void update(
    uint16_t num_guesses,
    uint16_t alpha,
    uint16_t beta,
    InternalString word,
    bool optimal);

  friend struct Cache;
};

struct Cache {
 public:
  Cache(size_t max_size);
  ~Cache();

  CacheKey min_search_key(const std::vector<InternalString>& words);
  CacheKey max_search_key(
    InternalString first_word, const std::vector<InternalString>& words);

  const CacheEntry find(int depth, const CacheKey& key);
  void update(
    int depth,
    const CacheKey& key,
    int num_guesses,
    int alpha,
    int beta,
    InternalString word,
    bool optimal);

 private:
  static const int max_depth = 40;
  CacheEntry& _find(int depth, const CacheKey& key, size_t segment);

  size_t _max_size;

  std::vector<CacheKey> _word_hashes;
  std::vector<CacheKey> _first_word_hashes;

  std::array<CacheKey, max_depth> _depth_hashes;

  constexpr static size_t _cache_segments = 1 << 20;

  std::array<
    std::unordered_map<CacheKey, CacheEntry, CacheKey::Hasher>,
    _cache_segments>
    _cache;

#ifdef _DESKTOP
  std::array<std::mutex, _cache_segments> _mutex;
#endif
};
