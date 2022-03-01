#include "error.hpp"

#include "format.hpp"

using namespace fmt;
using namespace std;

Error::Error(const string& msg) : _msg(msg) {}
Error::Error(string&& msg) : _msg(move(msg)) {}

const string& Error::msg() const { return _msg; }
