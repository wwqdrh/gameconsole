#pragma once

#include <memory>
#include <regex>
#include <string>
#include <variant>
#include <vector>

#include "collection.h"

namespace types {

// Forward declarations
class BaseType;
class FilterType;
class BoolType;
class AnyType;
class StringType;
class RegexType;
class IntType;
class FloatType;
class FloatRangeType;
class IntRangeType;
class Vector2Type;
class Vector3Type;

// Check result enum
enum class CheckResult { Ok, Failed, Canceled };

// Base class for all types
class BaseType {
public:
  virtual ~BaseType() = default;
  virtual std::string toString() const { return name; }
  virtual CheckResult check(const Variant &value) { return CheckResult::Ok; }
  virtual Variant normalize(const Variant &value) { return value; }

protected:
  std::string name;
};

class AnyType : public BaseType {
public:
  AnyType() { name = "Any"; }

  //   Variant normalize(const Variant &value) override { return value; }
};

// Filter type implementation
class FilterType : public BaseType {
public:
  enum class Mode { Allow, Deny };

  FilterType() { name = "Filter"; }

  void initialize(const std::vector<Variant> &filterList,
                  Mode mode = Mode::Allow) {
    filter_list = filterList;
    filter_mode = mode;
  }

  CheckResult check(const Variant &value) override {
    bool found = std::find(filter_list.begin(), filter_list.end(), value) !=
                 filter_list.end();
    if ((filter_mode == Mode::Allow && found) ||
        (filter_mode == Mode::Deny && !found)) {
      return CheckResult::Ok;
    }
    return CheckResult::Canceled;
  }

private:
  std::vector<Variant> filter_list;
  Mode filter_mode;
};

// Bool type implementation
class BoolType : public BaseType {
public:
  BoolType() { name = "Bool"; }

  Variant normalize(const Variant &value) override {
    if (std::holds_alternative<std::string>(value)) {
      std::string v = std::get<std::string>(value);
      std::transform(v.begin(), v.end(), v.begin(), ::tolower);
      return v == "true" || v == "TRUE" || v == "1";
    }
    return false;
  }
};

// String type implementation
class StringType : public BaseType {
public:
  StringType() { name = "String"; }

  Variant normalize(const Variant &value) override {
    return std::visit(
        [](auto &&arg) -> Variant {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::string>) {
            return arg;
          } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
          } else if constexpr (std::is_integral_v<T> ||
                               std::is_floating_point_v<T>) {
            return std::to_string(arg);
          } else {
            throw std::runtime_error("Unsupported type");
          }
        },
        value);
  }
};

// Base regex type implementation
class RegexType : public BaseType {
public:
  RegexType() { name = "Regex"; }

  void initialize(const std::string &pattern_name, const std::string &pattern) {
    name = pattern_name;
    regex_pattern = std::regex(pattern);
  }

  CheckResult check(const Variant &value) override {
    if (auto str = std::get_if<std::string>(&value)) {
      return std::regex_match(*str, regex_pattern) ? CheckResult::Ok
                                                   : CheckResult::Failed;
    }
    return CheckResult::Failed;
  }

protected:
  std::regex regex_pattern;

  std::string extractMatch(const Variant &value) const {
    if (auto str = std::get_if<std::string>(&value)) {
      std::smatch match;
      if (std::regex_match(*str, match, regex_pattern)) {
        return match.str();
      }
    }
    return "";
  }
};

class IntType : public RegexType {
public:
  IntType() { RegexType::initialize("Int", "^[+-]?\\d+$"); }

  //   Variant normalize(const Variant &value) override { return value; }
  Variant normalize(const Variant &value) override {
    return std::stoi(RegexType::extractMatch(value));
  }
};

class FloatType : public RegexType {
public:
  FloatType() {
    RegexType::initialize("Float",
                          "^[+-]?([0-9]*[\\.\\,]?[0-9]+|[0-9]+[\\.\\,]?"
                          "[0-9]*)([eE][+-]?[0-9]+)?$");
  }

  //   Variant normalize(const Variant &value) override { return value; }
  Variant normalize(const Variant &value) override {
    std::string v = RegexType::extractMatch(value);
    return std::stof(v.replace(v.begin(), v.end(), ',', '.'));
  }
};

class Vector2Type : public RegexType {
private:
  std::vector<std::string> _normalized_value;

public:
  Vector2Type() {
    RegexType::initialize("Vector2",
                          "^[+-]?([0-9]*[\\.\\,]?[0-9]+|[0-9]+[\\.\\,"
                          "]?[0-9]*)([eE][+-]?[0-9]+)?$");
  }

  //   Variant normalize(const Variant &value) override { return value; }
  CheckResult check(const Variant &value) override {
    if (auto str = std::get_if<std::string>(&value)) {
      std::string curr = *str;
      std::vector<std::string> values = {};
      size_t pos = 0;
      while ((pos = curr.find(";") != std::string::npos)) {
        values.push_back(curr.substr(0, pos));
        curr.erase(0, pos + 1);
      }

      if (values.size() < 2) {
        if (values.size() == 1) {
          values.push_back("0");
        } else {
          return CheckResult::Failed;
        }
      }

      // 检查两个值是否通过了 super.check
      for (int i = 0; i < 2; i++) {
        if (RegexType::check(values[i]) == CheckResult::Failed) {
          return CheckResult::Failed;
        }
      }

      _normalized_value = values;

      return CheckResult::Ok;
    }
    return CheckResult::Failed;
  }
  Variant normalize(const Variant &value) override {
    std::string v = RegexType::extractMatch(value);
    return std::stof(v.replace(v.begin(), v.end(), ',', '.'));
  }
};

class Vector3Type : public RegexType {
private:
  std::vector<std::string> _normalized_value;

public:
  Vector3Type() {
    RegexType::initialize("Vector3",
                          "^[+-]?([0-9]*[\\.\\,]?[0-9]+|[0-9]+[\\.\\,"
                          "]?[0-9]*)([eE][+-]?[0-9]+)?$");
  }

  //   Variant normalize(const Variant &value) override { return value; }
  CheckResult check(const Variant &value) override {
    if (auto str = std::get_if<std::string>(&value)) {
      std::string curr = *str;
      std::vector<std::string> values = {};
      size_t pos = 0;
      while ((pos = curr.find(";") != std::string::npos)) {
        values.push_back(curr.substr(0, pos));
        curr.erase(0, pos + 1);
      }

      if (values.size() < 3) {
        if (values.size() == 1) {
          values.push_back("0");
          values.push_back("0");
        } else if (values.size() == 2) {
          values.push_back("0");
        } else {
          return CheckResult::Failed;
        }
      }

      // 检查两个值是否通过了 super.check
      for (int i = 0; i < 3; i++) {
        if (RegexType::check(values[i]) == CheckResult::Failed) {
          return CheckResult::Failed;
        }
      }

      _normalized_value = values;

      return CheckResult::Ok;
    }
    return CheckResult::Failed;
  }
  Variant normalize(const Variant &value) override {
    std::string v = RegexType::extractMatch(value);
    return std::stof(v.replace(v.begin(), v.end(), ',', '.'));
  }
};

// Factory for creating type instances
class TypeFactory {
public:
  static std::shared_ptr<BaseType> build(int type) {
    switch (type) {
    case 0:
      return std::make_shared<AnyType>();
    case 1:
      return std::make_shared<BoolType>();
    case 2:
      return std::make_shared<IntType>();
    case 3:
      return std::make_shared<FloatType>();
    case 4:
      return std::make_shared<StringType>();
    case 5:
      return std::make_shared<Vector2Type>();
    case 7:
      return std::make_shared<Vector3Type>();
    default:
      return std::make_shared<BaseType>();
    }
  }
};

} // namespace types