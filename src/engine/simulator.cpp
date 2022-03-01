#include "simulator.hpp"

#include "utils/format.hpp"
#include "utils/format_vector.hpp"

#include <array>

using namespace fmt;
using namespace std;

namespace {

struct SecretInfo {
  InternalString secret;
  int num_guesses;
};

struct SimContext {
 public:
  SimContext(Engine& engine, int max_depth)
      : _engine(engine), _max_depth(max_depth)
  {}

  OrError<Unit> simulate_rec(
    InternalString guess, const GameState& game_state, int num_guesses)
  {
    num_guesses++;
    array<Partition, 256> buckets = game_state.partition_by_pattern(guess);

    for (const auto& b : buckets) {
      if (b.is_empty()) continue;

      if (num_guesses == 1) { _all_matches.insert(b.match()); }

      GameState next_state = game_state.state_from_partition(b);

      if (next_state.possible_secrets().size() == 1) {
        InternalString only_secret = next_state.possible_secrets()[0];

        int ng = num_guesses;
        if (guess != only_secret) { ng++; }

        _output.emplace_back(
          SecretInfo{.secret = only_secret, .num_guesses = ng});
        continue;
      }

      bail(search_result, _engine.search(next_state, _max_depth));
      if (!search_result.is_optimal) { _can_stop = false; }
      bail_unit(
        simulate_rec(search_result.best_guess, next_state, num_guesses));
    }
    return unit;
  }

  const vector<SecretInfo>& output() const { return _output; }

  const set<Match>& all_matches() const { return _all_matches; }

  bool can_stop() const { return _can_stop; }

 private:
  Engine& _engine;
  vector<SecretInfo> _output;
  set<Match> _all_matches;
  const int _max_depth;
  bool _can_stop = true;
};

} // namespace

Simulator::Simulator(Engine& engine) : _engine(engine) {}

Simulator::~Simulator() {}

OrError<WordInfo> Simulator::simulate(
  const GameState& game_state, InternalString first_guess, int max_depth)
{
  assert(max_depth > 0);
  SimContext sim_ctx(_engine, max_depth);
  bail_unit(sim_ctx.simulate_rec(first_guess, game_state, 0));

  int worst_num_guesses = 0;
  int sum_num_guesses = 0;
  map<int, int> distribution;
  optional<InternalString> most_difficult_secret;

  for (const auto& info : sim_ctx.output()) {
    int num_guesses = info.num_guesses;
    if (num_guesses > worst_num_guesses) {
      worst_num_guesses = num_guesses;
      most_difficult_secret = info.secret;
    }
    sum_num_guesses += num_guesses;
    distribution[num_guesses]++;
  }

  map<int, int> cumulative_distribution;
  int sum = 0;
  for (const auto& d : distribution) {
    sum += d.second;
    cumulative_distribution.emplace(d.first, sum);
  }

  vector<Match> all_matches_v;
  for (const auto m : sim_ctx.all_matches()) { all_matches_v.push_back(m); }

  return WordInfo{
    .first_guess = first_guess,
    .sum_num_guesses = sum_num_guesses,
    .avg_guesses =
      double(sum_num_guesses) / game_state.possible_secrets().size(),
    .worst_num_guesses = worst_num_guesses,
    .guess_distribution = distribution,
    .cumulative_guess_distribution = cumulative_distribution,
    .most_difficult_secret = *most_difficult_secret,
    .max_depth = max_depth,
    .all_matches = move(all_matches_v),
    .can_stop = sim_ctx.can_stop(),
  };
}

////////////////////////////////////////////////////////////////////////////////
// WordInfo
//

WordInfo::~WordInfo() {}

namespace json {

JsonValue to_json_t<WordInfo>::convert(const WordInfo& value)
{
  map<string, JsonValue> obj{
    {"guess", to_json(value.first_guess)},
    {"avg_guesses", to_json(value.avg_guesses)},
    {"worst_num_guesses", to_json(value.worst_num_guesses)},
    {"max_depth", to_json(value.max_depth)},
    {"all_matches", to_json(value.all_matches)},
  };
  return to_json(obj);
}

} // namespace json
