// Using Catch2 unit testing library

// Let Catch2 define a main function
#define CATCH_CONFIG_MAIN

#include "../include/catch.hpp"

#include "ast.hpp"
#include "token.hpp"

TEST_CASE("Test String") {

    auto program = Program();

    auto stmt = std::make_shared<DeclareStatement>(DeclareStatement{Token{TokenType::DECLARE, "let"}});

    stmt->name = std::make_shared<Identifier>(Identifier(Token{TokenType::IDENTIFIER, "myVar"}, "myVar"));
    stmt->value = std::make_shared<Identifier>(Identifier(Token{TokenType::IDENTIFIER, "anotherVar"}, "anotherVar"));

    program.statements.push_back(std::move(stmt));

    if (program.string() != "let myVar = anotherVar;") {
        std::cerr << "program.string wrong. got=" << program.string() << std::endl;
    }

    REQUIRE(program.string() == "let myVar = anotherVar;");
}
