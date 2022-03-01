#include "util.hpp"

using namespace std;

string right_pad_string(string str, size_t length)
{
  while (str.size() < length) { str += ' '; }
  return str;
}
