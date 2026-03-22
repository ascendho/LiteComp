#ifndef REPL_HPP
#define REPL_HPP

#include <vector>
#include <string>

static const std::string PROMPT{">> "};

static const std::string ERRORSIGN{R"(-------- ERROR --------  )"};

void lanuchRepl();

void printParserErrors(std::vector<std::string> errors);

#endif