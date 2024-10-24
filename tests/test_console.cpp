#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <variant>

#include "argument.h"
#include "console.h"
#include "types.h"

TEST(ConsoleTest, TestConsoleComplete) {
  auto console = Console(
      [](const std::string &val) -> void { std::cout << val << std::endl; });

  console.add_command(
      "command1",
      [](std::vector<Variant> args) {
        return Variant(std::string("command1"));
      },
      {std::make_shared<Argument>("name", ValueType::String,
                                  "a name' bool argument")},
      "test command1");
  console.add_command(
      "command1_void",
      [](std::vector<Variant> args) {
        return Variant(std::string("command1"));
      },
      {}, "test command1 no argument");

  ASSERT_EQ(console.autocomplete("command"), "command1");
  ASSERT_EQ(console.autocomplete("command1"), "command1_void");
  ASSERT_EQ(console.autocomplete("command1_"), "command1_void");
  ASSERT_EQ(console.autocomplete("command2"), "command2");
  ASSERT_EQ(console.get_commands("")->size(), 2);
  ASSERT_EQ(console.get_commands("command1_")->size(), 1);
}

TEST(ConsoleTest, TestConsoleExecute) {
  auto console = Console(
      [](const std::string &val) -> void { std::cout << val << std::endl; });

  int value = 0;

  console.add_command(
      "add",
      [&value](std::vector<Variant> args) {
        if (args.size() == 1) {
          if (std::holds_alternative<int>(args[0])) {
            value += std::get<int>(args[0]);
          }
        } else if (args.size() == 0) {
          value += 1;
        }
        return Variant(std::string("command1"));
      },
      {std::make_shared<Argument>("value", ValueType::Int,
                                  "a value' int argument")},
      "add operator");

  console.execute("add(1)");
  ASSERT_EQ(value, 1);
  console.execute("add(2)");
  ASSERT_EQ(value, 3);
  console.execute("add(3)");
  ASSERT_EQ(value, 6);
  console.execute("add()");
  ASSERT_EQ(value, 7);
}