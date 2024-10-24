#pragma once
#include <memory>

#include "collection.h"

class History : public Collection {
private:
  std::function<void(std::string)> _out;
  int max_length = 10;

public:
  History() = default;
  History(std::function<void(std::string)> out, int max_length = 10)
      : _out(out), max_length(max_length) {}

  void push(const Variant &value) {
    int l = size();
    if (l > 0 && last().value() == value) {
      return;
    }
    if (l == max_length) {
      pop();
    }
    add(value);
    // last();
  }

  std::optional<Variant> pop() {
    std::optional<Variant> value = first();
    if (value.has_value()) {
      remove_by_index(0);
    }
    return value;
  }

  void print_all() {
    if (_out == nullptr) {
      return;
    }

    int i = 1;
    seek(0);
    while (has_next()) {
      std::string command_name = std::get<std::string>(last().value());
      _out("[b]" + std::to_string(i) + ".[/b] [color=#ffff66][url=" +
           command_name + "]" + command_name + "[/url][/color]");
      i++;
    }
  }
};