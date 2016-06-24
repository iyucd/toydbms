#ifndef SQLPARSER_SQL_SCANNER_H
#define SQLPARSER_SQL_SCANNER_H


#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "sql_parser.tab.hh"
#include "location.hh"

namespace UCD{

    class SQLScanner : public yyFlexLexer{
    public:

        SQLScanner(std::istream *in) : yyFlexLexer(in)
        {
            loc = new UCD::SQLParser::location_type();
        };

        //get rid of override virtual function warning
        using FlexLexer::yylex;

        virtual
        int yylex( UCD::SQLParser::semantic_type * const lval,
                   UCD::SQLParser::location_type *location );
        // YY_DECL defined in mc_lexer.l
        // Method body created by flex in mc_lexer.yy.cc


    private:
        /* yyval ptr */
        UCD::SQLParser::semantic_type *yylval = nullptr;
        /* location ptr */
        UCD::SQLParser::location_type *loc    = nullptr;
    };

} /* end namespace UCD */

#endif //SQLPARSER_SQL_SCANNER_H
