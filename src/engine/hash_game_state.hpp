#pragma once

#include "internal_string.hpp"

#include <string>
#include <vector>

std::string hash_game_state(
  const std::vector<InternalString>& allowed_guesses,
  const std::vector<InternalString>& remaining_secrets,
  bool hard_mode);
