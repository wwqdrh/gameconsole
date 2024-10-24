#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "argument.h"
#include "console.h"

TEST(ConsoleTest, TestConsoleComplete) {
  auto console = Console(
      [](const std::string &val) -> void { std::cout << val << std::endl; });

  console.add_command(
      "command1",
      [](std::vector<Variant> args) {
        return Variant(std::string("command1"));
      },
      {std::make_shared<Argument>("name", 1, "a name' bool argument")},
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
