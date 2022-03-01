#include "dictionary.hpp"

#include <fstream>

#include "match.hpp"

using namespace std;

OrError<vector<InternalString>> Dictionary::load_words(const string& filename)
{
  vector<InternalString> words;

  ifstream word_file(filename);
  if (word_file.bad()) {
    return Error::format("Failed to open file $", filename);
  }

  string word;
  while (word_file >> word) {
    if (word.size() != 5) {
      return Error::format("All word must have 5 words, got $", word);
    }
    words.push_back(InternalString(word));
  }

  Match::init_result_cache();

  return words;
}
