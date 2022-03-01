#include "json.hpp"

#include <cassert>

#include "utils/format.hpp"

using namespace std;
using namespace fmt;

namespace {

using namespace json;

struct Reader {
 public:
  Reader(const string& str) : _str(str) {}

  char pop()
  {
    auto c = peek();
    _pos++;
    return c;
  }

  char peek()
  {
    assert(_pos < _str.size());
    return _str[_pos];
  }

  string next_chars(size_t count) const
  {
    if (_str.size() < count + _pos) {
      return _str.substr(_pos);
    } else {
      return _str.substr(_pos, _pos + count);
    }
  }

  bool eof() const { return _pos >= _str.size(); }

  string all_data() const { return _str; }

 private:
  string _str;
  size_t _pos = 0;
};

JsonValue parse_number(Reader& reader)
{
  int number = 0;
  while (!reader.eof() && isdigit(reader.peek())) {
    number = number * 10 + (reader.pop() - '0');
  }
  return JsonValue(number);
}

JsonValue parse_keyword(Reader& reader)
{
  string word;
  while (!reader.eof() && isalpha(reader.peek())) { word += reader.pop(); }
  return JsonValue(move(word));
}

string parse_string(Reader& reader)
{
  string str;
  if (reader.peek() != '"') {
    print_line(
      "Got unexpected char at beginning of string, next chars: '$'",
      reader.next_chars(20));
    assert(false && "Unexpected char at beginning of string");
  }
  reader.pop();
  while (true) {
    assert(!reader.eof() && "Unexpected end of string");
    char c = reader.pop();
    if (c == '"') { break; }

    str += c;
  }
  return str;
}

optional<JsonValue> parse_any_json(Reader& reader);

JsonValue parse_list(Reader& reader)
{
  vector<JsonValue> values;
  assert(reader.pop() == '[');
  while (true) {
    assert(!reader.eof() && "Unexpected end of list");

    if (reader.peek() == ']') {
      reader.pop();
      break;
    }

    auto element = parse_any_json(reader);
    assert(element.has_value() && "Invalid json list element");

    values.push_back(move(*element));

    assert(!reader.eof() && "Unexpected end of list");
    char c = reader.pop();
    if (c == ']') {
      break;
    } else if (c == ',') {
      // continue
    } else {
      assert("Unexpected character inside list");
    }
  }
  return JsonValue(move(values));
}

JsonValue parse_object(Reader& reader)
{
  map<string, JsonValue> values;
  assert(reader.pop() == '{');
  while (true) {
    assert(!reader.eof() && "Unexpected end of object");

    if (reader.peek() == '}') {
      reader.pop();
      break;
    }

    auto key = parse_string(reader);

    {
      assert(!reader.eof() && "Unexpected end of object");
      char c = reader.pop();
      assert(c == ':' && "Expectd colon after object key");
    }

    auto value = parse_any_json(reader);
    assert(value.has_value() && "Invalid json object value");
    values.emplace(move(key), move(*value));

    assert(!reader.eof() && "Unexpected end of object");
    {
      char c = reader.pop();
      if (c == '}') {
        break;
      } else if (c == ',') {
        // continue
      } else {
        assert("Unexpected character inside object");
      }
    }
  }
  return JsonValue(move(values));
}

optional<JsonValue> parse_any_json(Reader& reader)
{
  if (reader.eof()) { return nullopt; }

  char c = reader.peek();
  if (c == '{') {
    return parse_object(reader);
  } else if (c == '[') {
    return parse_list(reader);
  } else if (c == '"') {
    return JsonValue(parse_string(reader));
  } else if (isdigit(c)) {
    return parse_number(reader);
  } else if (isalpha(c)) {
    return parse_keyword(reader);
  } else {
    assert(false && "Unexpected charater");
  }
}

} // namespace

namespace json {

std::string JsonValue::to_string() const
{
  if (holds_alternative<string>(_value)) {
    return format("\"$\"", get<std::string>(_value));
  } else if (holds_alternative<int>(_value)) {
    return format("$", get<int>(_value));
  } else if (holds_alternative<double>(_value)) {
    return format("$", get<double>(_value));
  } else if (holds_alternative<null_t>(_value)) {
    return "null";
  } else if (holds_alternative<vector<JsonValue>>(_value)) {
    const auto& v = get<std::vector<JsonValue>>(_value);
    std::string out = "[";
    bool first = true;
    for (const auto& el : v) {
      if (!first) { out += ","; }
      first = false;
      out += el.to_string();
    }
    out += "]";
    return out;
  } else if (holds_alternative<map<string, JsonValue>>(_value)) {
    const auto& v = get<std::map<string, JsonValue>>(_value);
    std::string out = "{";
    bool first = true;
    for (const auto& el : v) {
      if (!first) { out += ","; }
      first = false;
      out += format("\"$\":", el.first);
      out += el.second.to_string();
    }
    out += "}";
    return out;
  } else {
    assert(false && "Bug!");
  }
}

JsonValue JsonValue::of_string(const string& str)
{
  Reader reader(str);
  auto value = parse_any_json(reader);
  assert(value.has_value() && "empty json");
  return move(*value);
}

string JsonValue::what_alternative() const
{
  if (holds_alternative<map<string, JsonValue>>(_value)) {
    return "object";
  } else if (holds_alternative<vector<JsonValue>>(_value)) {
    return "list";
  } else if (holds_alternative<string>(_value)) {
    return "string";
  } else if (holds_alternative<int>(_value)) {
    return "int";
  } else if (holds_alternative<double>(_value)) {
    return "double";
  } else {
    assert(false && "bug!");
  }
}

const std::map<std::string, JsonValue>& JsonValue::get_object() const
{
  assert((holds_alternative<map<string, JsonValue>>(_value)));
  return get<map<string, JsonValue>>(_value);
}

const std::vector<JsonValue>& JsonValue::get_list() const
{
  assert(holds_alternative<vector<JsonValue>>(_value));
  return get<vector<JsonValue>>(_value);
}

const std::string& JsonValue::get_string() const
{
  assert(holds_alternative<string>(_value));
  return get<string>(_value);
}

int JsonValue::get_int() const
{
  assert(holds_alternative<int>(_value));
  return get<int>(_value);
}

double JsonValue::get_double() const
{
  assert(holds_alternative<double>(_value));
  return get<double>(_value);
}

} // namespace json
