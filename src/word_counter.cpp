#include "word_counter.hpp"

#include <fstream>
#include <unordered_map>

#include "utils/command.hpp"

using namespace std;
using namespace fmt;

namespace {

bool validated_word(string& str)
{
  if (str.size() != 5) { return false; }

  for (char& c : str) {
    c = tolower(c);
    if (c < 'a' || c > 'z') return false;
  }

  return true;
}

OrError<Unit> count_words(const optional<string>& filename)
{
  if (!filename.has_value()) { return Error("Argument required"); }

  ifstream file(*filename);
  if (file.bad()) { return Error("Failed to open input file"); }

  string word;
  unordered_map<string, int> counts;
  size_t every = 0;
  size_t valid_words = 0;
  while (file >> word) {
    every++;
    if (every % (1 << 28) == 0) {
      print_line(
        "$ Gb, $ M words, $ M valid words, $ distinc valid words",
        double(file.tellg()) / 1024 / 1024 / 1024,
        double(every) / 1000000.0,
        double(valid_words) / 1000000.0,
        counts.size());
    }
    if (!validated_word(word)) continue;
    valid_words++;
    counts[word]++;
  }

  vector<pair<int, string>> count_to_word;
  for (const auto& w : counts) {
    count_to_word.emplace_back(w.second, w.first);
  }
  sort(
    count_to_word.begin(),
    count_to_word.end(),
    [](const auto& c1, const auto& c2) {
      if (c1.first != c2.first) return c2.first < c1.first;
      return c1.second < c2.second;
    });

  int rank = 0;
  for (const auto& c : count_to_word) {
    rank++;
    print_line("$ $ $", c.second, rank, c.first);
  }

  return unit;
}

} // namespace

Command WordCounter::command()
{
  auto builder = CommandBuilder("Counte word frequency from a file");
  auto filename = builder.anon(string_flag);
  return builder.run([=]() { return count_words(filename->value()); });
}
