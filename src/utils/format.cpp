#include "format.hpp"

#include <iostream>

using namespace std;

namespace fmt {

void print_str(string str)
{
  str += '\n';
  cout << str << flush;
}

void print_err_str(string str)
{
  str += '\n';
  cerr << str << flush;
}

} // namespace fmt
