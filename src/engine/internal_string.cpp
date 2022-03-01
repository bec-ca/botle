#include "internal_string.hpp"

#include <unordered_map>
#include <vector>

#include "utils/format.hpp"
#include "utils/format_unordered_map.hpp"
#include "utils/format_vector.hpp"
#include "utils/json.hpp"

using namespace std;

namespace {

unordered_map<string, uint16_t> ids{{"", 0}};
vector<string> strs = {""};

} // namespace

uint16_t InternalString::_get_id(const string& str)
{
  auto it = ids.find(str);
  if (it != ids.end()) { return it->second; }
  uint16_t id = strs.size();
  ids.emplace(str, id);
  strs.emplace_back(str);
  return id;
}

const string& InternalString::str() const { return strs.at(_id); }

uint16_t InternalString::max_id() { return strs.size(); }

void InternalString::debug()
{
  fmt::print_line(strs);
  fmt::print_line(ids);
}

bool InternalString::is_valid() const { return _id < strs.size(); }

void InternalString::wipe_ids()
{
  strs.clear();
  ids.clear();
  // First id is reserved for the empty string
  _get_id("");
}

namespace json {
JsonValue to_json_t<InternalString>::convert(const InternalString& value)
{
  return JsonValue(value.str());
}
} // namespace json
