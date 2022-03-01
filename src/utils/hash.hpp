#pragma once

#include <array>
#include <memory>
#include <string>

struct HashResult {
 public:
  HashResult(std::array<uint64_t, 4> bits);
  ~HashResult();

  std::string hex() const;

 private:
  std::array<uint64_t, 4> _bits;
};

struct Hash {
 public:
  Hash();
  ~Hash();

  void feed_string(const std::string& data);
  void feed_char(char c);

  HashResult finalize();

 private:
  struct State;

  std::unique_ptr<State> _state;
};
