#pragma once

#include "collection.h"
#include "commands.h"
#include "history.h"
#include "types.h"
#include <algorithm>
#include <memory>
#include <regex>
#include <string>
#include <variant>
#include <vector>

class Console {
private:
  History _history;
  Commands _commands;
  // std::regex _erase_bb_tags_regex;

public:
  Console(std::function<void(const std::string &)> out)
      : _history(History(out)) {
    // _erase_bb_tags_regex =
    //     std::regex("\\[[\\/]?[a-z0-9\\=\\#\\ \\_\\-\\,\\.\\;]+\\]");
  }

  std::optional<std::shared_ptr<Command>> get_command(const std::string &name) {
    std::optional<Variant> val = _commands.get_value(name);
    if (val.has_value()) {
      return std::get<std::shared_ptr<Command>>(val.value());
    } else {
      return std::nullopt;
    }
  }
  std::shared_ptr<Collection> get_commands(const std::string &name) {
    return _commands.find(name);
  }

  void add_command(const std::string &name,
                   const std::function<Variant(std::vector<Variant>)> &target,
                   const std::vector<std::shared_ptr<Argument>> &arguments,
                   const std::string &description) {
    _commands.set_value(
        name, std::make_shared<Command>(name, target, arguments, description));
  }

  bool register_command(const std::string &name, std::shared_ptr<Command> cmd) {
    if (!_commands.contains_key(name)) {
      _commands.set_value(name, cmd);
      return true;
    } else {
      return false;
    }
  }

  void remove_command(const std::string &name) { _commands.remove(name); }

  std::string autocomplete(const std::string &command_name) {
    auto commands = _commands.find(command_name);
    if (commands->size() == 1) {
      if (commands->first().has_value()) {
        auto val =
            std::get<std::shared_ptr<Command>>(commands->first().value());
        return val->name;
      }
    }

    std::vector<std::string> all_prefix_commands = {};
    while (commands->has_next()) {
      auto val = commands->next();
      if (!val.has_value()) {
        continue;
      }

      std::string cur_name =
          std::get<std::shared_ptr<Command>>(val.value())->name;
      if (command_name == cur_name) {
        continue;
      }
      all_prefix_commands.push_back(cur_name);
    }
    if (all_prefix_commands.size() == 0) {
      return command_name;
    }

    int idx = -1;
    std::string cur_prefix = "";
    std::sort(all_prefix_commands.begin(), all_prefix_commands.end());
    while (true) {
      char cur = 0;
      idx += 1;
      for (auto item : all_prefix_commands) {
        if (idx >= item.length()) {
          return cur_prefix;
        }

        if (!cur) {
          cur = item[idx];
        } else if (cur != item[idx]) {
          return cur_prefix;
        }
      }
      cur_prefix += all_prefix_commands[0][idx];
    }
    return cur_prefix;
  }
};