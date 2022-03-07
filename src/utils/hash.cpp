#include "hash.hpp"

#include "small_hash.hpp"

#include <cassert>

using namespace std;

namespace {

struct Block {
 public:
  Block() {}

  void clear()
  {
    _size = 0;
    for (uint64_t& v : _data) { v = 0; }
  }

  bool is_full() { return _size >= 4 * 8; }

  void add_char(char c)
  {
    assert(!is_full());
    size_t idx = _size / 8;
    _data[idx] = _data[idx] * 256 + c;
    ++_size;
  }

  uint64_t get(size_t idx) const
  {
    assert(idx <= 4);
    return _data[idx];
  }

  size_t size() const { return _size; }

 private:
  array<uint64_t, 4> _data{0, 0, 0, 0};
  size_t _size = 0;
};

char hex_char(int value)
{
  if (value >= 0 && value < 10) return value + '0';
  if (value >= 10 && value < 16) return value - 10 + 'a';
  assert(false && "Invalid hex value");
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Hash::State
//

struct Hash::State {
 public:
  State()
      : partials{
          0x1d8dbe1a34ef9c58ull,
          0x69ea132c47819762ull,
          0x1636679a015ee2ecull,
          0xc6531e9958c7a1aeull,
        }
  {}

  void feed_char(const char c)
  {
    if (block.is_full()) { process_block(); }
    block.add_char(c);
  }

  HashResult finalize()
  {
    if (block.size() > 0) {
      process_block();
      while (!block.is_full()) { block.add_char(7); }
    }
    return HashResult(partials);
  }

 private:
  array<uint64_t, 4> partials;

  Block block;

  void mix(uint64_t& v1, uint64_t& v2)
  {
    uint64_t o1 = hash64(v1 ^ (v2 >> 32));
    uint64_t o2 = hash64(v2 ^ (v1 >> 32));

    v1 = o1;
    v2 = o2;
  }

  void process_block()
  {
    for (int i = 0; i < 4; i++) {
      partials[i] ^= block.get(i);
      partials[i] = hash64(partials[i]);
    }

    mix(partials[0], partials[1]);
    mix(partials[2], partials[3]);

    mix(partials[0], partials[2]);
    mix(partials[1], partials[3]);

    block.clear();
  }
};

////////////////////////////////////////////////////////////////////////////////
// Hash
//

Hash::Hash() : _state(make_unique<State>()) {}

Hash::~Hash() {}

void Hash::feed_string(const string& data)
{
  for (char c : data) { _state->feed_char(c); };
}

void Hash::feed_char(char c) { _state->feed_char(c); }

HashResult Hash::finalize() { return _state->finalize(); }

////////////////////////////////////////////////////////////////////////////////
// HashResult
//

HashResult::HashResult(array<uint64_t, 4> bits) : _bits(bits) {}

HashResult::~HashResult() {}

string HashResult::hex() const
{
  string output;
  for (uint64_t b : _bits) {
    for (int i = 0; i < 16; i++) {
      output += hex_char(b % 16);
      b /= 16;
    }
  }
  return output;
}
