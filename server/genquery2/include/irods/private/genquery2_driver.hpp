#ifndef IRODS_GENQUERY2_DRIVER_HPP
#define IRODS_GENQUERY2_DRIVER_HPP

#include "irods/private/genquery2_scanner.hpp"
#include "irods/private/genquery2_ast_types.hpp" // For irods::experimental::genquery2::select.

#include "parser.hpp" // Generated by Bison.

#ifndef yyFlexLexerOnce
#  include <FlexLexer.h>
#endif // yyFlexLexerOnce

#include <string>

// Give the expected function signature of the yylex() function to flex.
// The parameter, drv, is required. It is referenced by the flex script, lexer.l.
// This is an easy alternative to the "%code provides" directive in parser.y.
//#undef  YY_DECL
//#define YY_DECL yy::parser::symbol_type scanner::yylex(irods::experimental::genquery2::driver& drv)

namespace irods::experimental::genquery2
{
    class driver
    {
      public:
        driver() = default;

        auto parse(const std::string& _s) -> int;

        // Holds an AST-like representation of a GenQuery2 string.
        irods::experimental::genquery2::select select;

        // The Flex scanner implementation.
        scanner lexer;

        // Holds the current location of the parser.
        yy::location location;

        // Used by the lexer to capture string literals.
        // This aids in handling escape sequences.
        std::string string_literal;
    }; // class driver
} // namespace irods::experimental::genquery2

#endif // IRODS_GENQUERY2_DRIVER_HPP
