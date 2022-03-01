#include "command.hpp"

#include "error.hpp"
#include "util.hpp"

#include <deque>
#include <stdexcept>
#include <vector>

using namespace std;
using namespace fmt;

namespace {

const string& flag_name(const Flag& flag)
{
  return visit([&](auto flag) -> const string& { return flag->name(); }, flag);
}

OrError<Flag> find_flag(const vector<Flag>& flags, const string& name)
{
  for (const auto& flag : flags) {
    if (name == flag_name(flag)) { return flag; }
  }
  return Error::format("Unknown flag '$'", name);
}

OrError<Unit> parse_args(
  const vector<Flag>& named_flags,
  const vector<AnonFlag::ptr>& anon_flags,
  const deque<string>& args)
{
  size_t anon_flag_index = 0;
  for (size_t i = 0; i < args.size();) {
    const string& arg = args.at(i++);
    if (arg.front() == '-') {
      bail(flag, find_flag(named_flags, arg));
      bail_unit(visit(
        [&](auto flag) -> OrError<Unit> {
          using T = decay_t<decltype(flag)>;
          if constexpr (is_same_v<T, ValueFlag::ptr>) {
            if (i >= args.size()) {
              return Error::format("No arguments for flag $", arg);
            }
            auto value = args.at(i++);
            auto err = flag->parse_value(value);
            if (err.is_error()) {
              return Error::format(
                "Failed to parse flag $ with value '$': $",
                arg,
                value,
                err.error());
            }
            return unit;
          } else if constexpr (is_same_v<T, BooleanFlag::ptr>) {
            flag->set();
            return unit;
          } else {
            static_assert(always_false_v<T> && "visit is not exhaustive");
          }
        },
        flag));
    } else {
      if (anon_flag_index >= anon_flags.size()) {
        return Error::format("Unexpected anonymous argument '$'", arg);
      }
      auto err = anon_flags[anon_flag_index]->parse_value(arg);
      if (err.is_error()) {
        return Error::format(
          "Failed to parse anon flag with value '$': $", arg, err.error());
      }
      anon_flag_index++;
    }
  }
  for (const auto& flag : named_flags) {
    bail_unit(visit(
      [&](auto flag) -> OrError<Unit> {
        using T = decay_t<decltype(flag)>;
        if constexpr (is_same_v<T, ValueFlag::ptr>) {
          return flag->finish_parsing();
        } else if constexpr (is_same_v<T, BooleanFlag::ptr>) {
          return unit;
        } else {
          static_assert(always_false_v<T> && "visit is not exhaustive");
        }
      },
      flag));
  }
  return unit;
}
} // namespace

NamedFlag::NamedFlag(const string& name) : _name(name) {}
NamedFlag::~NamedFlag() {}

const string& NamedFlag::name() const { return _name; }

BooleanFlag::BooleanFlag(const string& name) : NamedFlag(name), _value(false) {}

BooleanFlag::~BooleanFlag() {}

void BooleanFlag::set() { _value = true; }
bool BooleanFlag::value() const { return _value; }

BooleanFlag::ptr BooleanFlag::create(const string& name)
{
  return ptr(new BooleanFlag(name));
}

ValueFlag::ValueFlag(const string& name) : NamedFlag(name) {}

ValueFlag::~ValueFlag() {}

Command::Command(
  const string& description,
  const vector<Flag>& flags,
  const vector<AnonFlag::ptr>& anon_flags,
  handler_type handler)
    : _description(description),
      _handler(handler),
      _flags(flags),
      _anon_flags(anon_flags)
{}

int Command::run(const deque<string>& args) const
{
  {
    auto err = parse_args(_flags, _anon_flags, args);
    if (err.is_error()) {
      print_line(err.error());
      print_help();
      return 1;
    }
  }
  auto err = _handler();
  if (err.is_error()) {
    print_line(err.error().msg());
    return 1;
  }

  return 0;
}

void Command::print_help() const
{
  print_line("Accepted flags:");
  if (!_flags.empty()) {
    for (const auto& flag : _flags) { print_line("    $", flag_name(flag)); }
  }
}

const string& Command::description() const { return _description; }

// Flags

const OrError<string> StringFlag::operator()(const string& value)
{
  return value;
}

StringFlag string_flag;

const OrError<int> IntFlag::operator()(const string& value)
{
  try {
    return stoi(value);
  } catch (invalid_argument&) {
    return Error("Not a numerical value");
  } catch (out_of_range&) {
    return Error("Numerical overflow");
  }
}

IntFlag int_flag;

const OrError<double> FloatFlag::operator()(const string& value)
{
  try {
    return stod(value);
  } catch (invalid_argument&) {
    return Error("Not a numerical value");
  } catch (out_of_range&) {
    return Error("Numerical overflow");
  }
}

FloatFlag float_flag;

////////////////////////////////////////////////////////////////////////////////
// CommandBuilder
//

CommandBuilder::CommandBuilder(const string& description)
    : _description(description)
{}

BooleanFlag::ptr CommandBuilder::no_arg(const std::string& name)
{
  auto flag = BooleanFlag::create(name);
  _flags.push_back(flag);
  return flag;
}

////////////////////////////////////////////////////////////////////////////////
// CommandGroup
//

CommandGroup::CommandGroup(map<string, Command> handlers)
    : _handlers(move(handlers))
{
  _add_cmd("help", {"Prints this help", {}, {}, [&] {
                      this->print_help();
                      return unit;
                    }});
}

void CommandGroup::_add_cmd(const string& name, Command command)
{
  _handlers.emplace(name, move(command));
}

void CommandGroup::print_help()
{
  print_line("Available comands:");

  size_t longest_name = 0;
  for (const auto& cmd : _handlers) {
    longest_name = max(longest_name, cmd.first.size());
  }

  for (const auto& cmd : _handlers) {
    print_line(
      "  $  $",
      right_pad_string(cmd.first, longest_name),
      cmd.second.description());
  }
}

int CommandGroup::run(int argc, char* argv[])
{
  deque<string> args;
  for (int i = 1; i < argc; i++) { args.push_back(argv[i]); }

  if (args.empty()) {
    print_line("No arguments given");
    print_help();
    return 1;
  }

  string cmd = args[0];

  auto it = _handlers.find(cmd);
  if (it == _handlers.end()) {
    print_line("Unknown command: $", cmd);
    print_help();
    return 1;
  } else {
    args.pop_front();
    return it->second.run(args);
  }
}

////////////////////////////////////////////////////////////////////////////////
// CommandGroupBuilder
//

CommandGroupBuilder::CommandGroupBuilder() {}

CommandGroupBuilder& CommandGroupBuilder::cmd(
  const std::string& name, Command command)
{
  _handlers.emplace(name, move(command));
  return *this;
}

CommandGroup CommandGroupBuilder::build()
{
  return CommandGroup(move(_handlers));
}
