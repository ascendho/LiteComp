#pragma once


#include <vector>
#include <string>
#include <string_view>

inline constexpr std::string_view PROMPT{">> "};

inline constexpr std::string_view ERRORSIGN{R"(-------- ERROR --------  )"};

void launchRepl();

void printParserErrors(std::vector<std::string> errors);

