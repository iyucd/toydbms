%{
/* C++ string header, for string ops below */
#include <string>
#include <iostream>
#include "sql_driver.h"

/* Implementation of yyFlexScanner */
#include "sql_scanner.h"
#undef  YY_DECL
#define YY_DECL int UCD::SQLScanner::yylex(UCD::SQLParser::semantic_type * const lval, UCD::SQLParser::location_type *location )

/* typedef to make the returns for the tokens shorter */
using token = UCD::SQLParser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return( token::QUIT )

/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
%option nodefault
%option yyclass="UCD::SQLScanner"
%option noyywrap
%option c++
%%
%{          /** Code executed at the beginning of yylex **/
            yylval = lval;
%}
[a-zA-Z_$][a-zA-Z0-9_$]*	{ char buf[10];
                  int i=0; char *np;
				  for(i=0; i<yyleng && i<9; i++) buf[i]=toupper(yytext[i]); buf[i]=0;
				  if(!strcmp("SELECT", buf)) return token::SELECT;
				  if(!strcmp("FROM", buf)) return token::FROM;
				  if(!strcmp("WHERE", buf)) return token::WHERE;
				  if(!strcmp("AS", buf)) return token::AS;
				  if(!strcmp("GROUP", buf)) return token::GROUP;
				  if(!strcmp("BY", buf)) return token::BY;
				  if(!strcmp("HAVING", buf)) return token::HAVING;
				  if(!strcmp("ORDER", buf)) return token::ORDER;
				  if(!strcmp("ASC", buf)) return token::ASC;
				  if(!strcmp("DESC", buf)) return token::DESC;
				  if(!strcmp("INSERT", buf)) return token::INSERT;
				  if(!strcmp("INTO", buf)) return token::INTO;
				  if(!strcmp("VALUES", buf)) return token::VALUES;
				  if(!strcmp("DELETE", buf)) return token::DELETE;
				  if(!strcmp("CREATE", buf)) return token::CREATE;
				  if(!strcmp("TABLE", buf)) return token::TABLE;
				  if(!strcmp("DROP", buf)) return token::DROP;
				  if(!strcmp("QUIT", buf)) return token::QUIT;
				  if(!strcmp("KEY", buf)) return token::KEY;
				  if(!strcmp("PRIMARY", buf)) return token::PRIMARY;
				  if(!strcmp("DISTINCT", buf)) return token::DISTINCT;
				  if(!strcmp("NULL", buf)) return token::NUL;
				  if(!strcmp("IN", buf)) return token::IN;
				  if(!strcmp("ON", buf)) return token::ON;
				  if(!strcmp("JOIN", buf)) return token::JOIN;
				  if(!strcmp("AND", buf)) return token::AND;
				  if(!strcmp("OR", buf)) return token::OR;
				  if(!strcmp("NOT", buf)) return token::NOT;
				  if(!strcmp("DESCRIBE", buf)) return token::DESCRIBE;
				  if(!strcmp("UPDATE", buf)) return token::UPDATE;
				  if(!strcmp("SET", buf)) return token::SET;
				  if(!strcmp("SHOW", buf)) return token::SHOW;
				  if(!strcmp("TABLES", buf)) return token::TABLES;
				  np = (char *)malloc(yyleng+1);
				  for(i=0; i<yyleng; i++) np[i] = tolower(yytext[i]);
				  np[i] = '\0';
				  yylval->str = new std::string(np, yyleng);
				  return token::IDENT;
				}
[0-9]+		    { yylval->num = atoi(yytext); return token::NUMBER; }
'[^\n']*'		{ yylval->str = new std::string(yytext, yyleng); loc->step(); return token::STRING; }
'[^'\n]*\n		{ loc->step(); return token::ERROR; }  /* std::cout << " : ERROR: Unterminated string \n"; */
=				{ return '='; }
\<\>			{ return token::NOTEQ; }
\<				{ return '<'; }
\>				{ return '>'; }
\<=				{ return token::LEQ; }
\>=				{ return token::GEQ; }
\n				{ loc->step(); }
[ \t]			{ ; }
\(				{ return '('; }
\)				{ return ')'; }
\+				{ return '+'; }
-				{ return '-'; }
\*				{ return '*'; }
\/				{ return '/'; }
;				{ return ';'; }
,				{ return ','; }
.				{ return '.'; } /* std::cout << " : Warning: Invalid character, line ignored\n"; */
%%


