#pragma once

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "error.hpp"

struct AnonFlag {
 public:
  using ptr = std::shared_ptr<AnonFlag>;

  explicit AnonFlag(){};

  virtual ~AnonFlag(){};

  virtual OrError<Unit> parse_value(const std::string& value) = 0;
};

template <class P> struct AnonFlagTemplate : public AnonFlag {
 public:
  using value_type = typename P::value_type;
  using ptr = std::shared_ptr<AnonFlagTemplate>;

  virtual ~AnonFlagTemplate() {}

  static ptr create(const P& spec) { return ptr(new AnonFlagTemplate(spec)); }

  const std::optional<value_type>& value() const { return _value; }

  virtual OrError<Unit> parse_value(const std::string& value)
  {
    bail(parsed_value, _spec(value));
    _value = parsed_value;
    return unit;
  };

 private:
  explicit AnonFlagTemplate(const P& spec) : _spec(spec) {}

  std::optional<value_type> _value;
  P _spec;
};

struct NamedFlag {
 public:
  explicit NamedFlag(const std::string& name);

  virtual ~NamedFlag();

  const std::string& name() const;

 private:
  std::string _name;
};

struct BooleanFlag : public NamedFlag {
 public:
  using ptr = std::shared_ptr<BooleanFlag>;

  explicit BooleanFlag(const std::string& name);

  static ptr create(const std::string& name);

  virtual ~BooleanFlag();

  void set();

  bool value() const;

 private:
  bool _value;
};

struct ValueFlag : public NamedFlag {
 public:
  using ptr = std::shared_ptr<ValueFlag>;

  explicit ValueFlag(const std::string& name);

  virtual ~ValueFlag();

  virtual OrError<Unit> parse_value(const std::string& value) = 0;

  virtual OrError<Unit> finish_parsing() const = 0;
};

template <class P> struct FlagTemplate : public ValueFlag {
 public:
  using value_type = typename P::value_type;
  using ptr = std::shared_ptr<FlagTemplate>;

  static ptr create(const std::string& name, const P& spec)
  {
    return ptr(new FlagTemplate(name, spec));
  }

  const std::optional<value_type>& value() const { return _value; }

  virtual OrError<Unit> parse_value(const std::string& value)
  {
    bail(parsed_value, _spec(value));
    _value = std::move(parsed_value);
    return unit;
  };

  virtual OrError<Unit> finish_parsing() const { return unit; }

 protected:
  explicit FlagTemplate(
    const std::string& name,
    const P& spec,
    const std::optional<value_type>& def = std::nullopt)
      : ValueFlag(name), _spec(spec), _value(def)
  {}

 private:
  P _spec;
  std::optional<value_type> _value;
};

template <class P> struct RequiredFlagTemplate : public FlagTemplate<P> {
 public:
  using ptr = std::shared_ptr<RequiredFlagTemplate>;
  using value_type = typename P::value_type;
  static ptr create(
    const std::string& name,
    const P& spec,
    const std::optional<value_type>& def = std::nullopt)
  {
    return ptr(new RequiredFlagTemplate(name, spec, def));
  }

  virtual OrError<Unit> finish_parsing() const
  {
    if (!FlagTemplate<P>::value().has_value()) {
      return Error::format(
        "Flag $ is required, but not provided", this->name());
    }
    return unit;
  }

  const value_type& value() const { return *FlagTemplate<P>::value(); }

 private:
  explicit RequiredFlagTemplate(
    const std::string& name,
    const P& spec,
    const std::optional<value_type>& def)
      : FlagTemplate<P>(name, spec, def)
  {}
};

struct StringFlag {
  using value_type = std::string;
  const OrError<value_type> operator()(const std::string& value);
} extern string_flag;

struct IntFlag {
  using value_type = int;
  const OrError<value_type> operator()(const std::string& value);
} extern int_flag;

struct FloatFlag {
  using value_type = double;
  const OrError<value_type> operator()(const std::string& value);
} extern float_flag;

using Flag = std::variant<ValueFlag::ptr, BooleanFlag::ptr>;

struct Command {
 public:
  using handler_type = std::function<OrError<Unit>(void)>;

  Command(
    const std::string& description,
    const std::vector<Flag>& flags,
    const std::vector<AnonFlag::ptr>& anon_flags,
    handler_type handler);

  Command(Command&& other) = default;
  Command(const Command& other) = delete;
  Command& operator=(const Command& other) = delete;

  const std::string& description() const;

  int run(const std::deque<std::string>& flags) const;

  void print_help() const;

 private:
  std::string _description;
  handler_type _handler;
  std::vector<Flag> _flags;
  std::vector<AnonFlag::ptr> _anon_flags;
};

struct CommandBuilder {
 public:
  CommandBuilder(const std::string& description);

  BooleanFlag::ptr no_arg(const std::string& name);

  template <class S> auto optional(const std::string& name, const S& spec)
  {
    auto flag = FlagTemplate<S>::create(name, spec);
    _flags.push_back(flag);
    return flag;
  }

  template <class S>
  auto optional_with_default(
    const std::string& name, const S& spec, const typename S::value_type& def)
  {
    auto flag = RequiredFlagTemplate<S>::create(name, spec, def);
    _flags.push_back(flag);
    return flag;
  }

  template <class S> auto required(const std::string& name, const S& spec)
  {
    auto flag = RequiredFlagTemplate<S>::create(name, spec);
    _flags.push_back(flag);
    return flag;
  }

  template <class S> auto anon(const S& spec)
  {
    auto flag = AnonFlagTemplate<S>::create(spec);
    _anon_flags.push_back(flag);
    return flag;
  }

  template <class F> Command run(F handler)
  {
    return Command(_description, _flags, _anon_flags, handler);
  }

 private:
  std::string _description;
  std::vector<Flag> _flags;

  std::vector<AnonFlag::ptr> _anon_flags;
};

struct CommandGroup {
 public:
  CommandGroup();
  CommandGroup(std::map<std::string, Command> _handlers);

  void print_help();
  int run(int argc, char* argv[]);

 private:
  void _add_cmd(const std::string& name, Command command);

  std::map<std::string, Command> _handlers;
};

struct CommandGroupBuilder {
 public:
  CommandGroupBuilder();

  CommandGroupBuilder& cmd(const std::string& name, Command command);

  CommandGroup build();

 private:
  void _add_cmd(const std::string& name, Command command);

  std::map<std::string, Command> _handlers;
};
