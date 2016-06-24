%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines
%define api.namespace {UCD}
%define parser_class_name {SQLParser}

%code requires{
   namespace UCD {
      class SQLDriver;
      class SQLScanner;
   }

// The following definitions is missing when %locations isn't used
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

}

%parse-param { UCD::SQLScanner  &scanner  }
%parse-param { UCD::SQLDriver  &driver  }

%error-verbose

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   /* include for all driver functions */
   #include "sql_driver.h"
   #include "sql_expr.h"
   #include "sql_scanner.h"

#undef yylex
#define yylex scanner.yylex

    char linebuf[500];
}

//define api.value.type variant
//define parse.assert

%union {
    int  			            num;
    std::string*		        str;
    //class UCD::SQLExpression*   expr;
}

%token	<str>   IDENT
%token	<num>   NUMBER  "integer"
%token	<str>   STRING  "string"
%token	NUL

%token	<num>   SELECT
%token	<num>   AS
%token	<num>   FROM
%token	<num>   WHERE
%token	<num>   GROUP
%token	<num>   BY
%token	<num>   HAVING
%token	<num>   ORDER
%token	<num>   ASC
%token	<num>   DESC
%token	<num>   INSERT
%token	<num>   INTO
%token	<num>   VALUES
%token	<num>   UPDATE
%token	<num>   SET
%token	<num>   DELETE
%token	<num>   CREATE
%token	<num>   TABLE
%token	<num>   DESCRIBE
%token	<num>   SHOW
%token	<num>   TABLES
%token	<num>   DROP
%token	<num>   PRIMARY
%token	<num>   DISTINCT
%token	<num>   KEY
%token	<num>   IN
%token	<num>   ON
%token	<num>   JOIN

%token	NOTEQ
%token	GEQ
%token	LEQ
%token	AND
%token	OR
%token	NOT

%token	ERROR
%token	QUIT

%destructor { delete $$; } STRING

%locations

%type<num>  bin_op
%type<num>  sql_command select_expr
%type<num>  value join outcol
%type<num>  projection data_list whereclause havingclause
%type<num>  column_decl column_decls
%type<num>  grouplist groupclause
%type<num>  orderspec orderclause orderlist
%type<num>  expr fcall fncargs
%type<num>  tablename colname groupspec
//type<exr>

%%
program:   sql_command  { YYACCEPT; }
	|	   QUIT ';'	    { YYABORT; }
	;

sql_command
    :	INSERT INTO tablename VALUES '(' data_list ')' ';'
		{   driver.evalexpr(driver.optimize(driver.compile(driver.cvt_itoe(
		        driver.makeexpr(OP_INSERTROW, 2, $3, $6)))));
		    $$=0;
		    driver.clear();
		}
	|	INSERT INTO tablename select_expr ';'
		{   std::cout << "Multi-row insert not implemented yet\n";
		    driver.clear();
		}
	|	select_expr ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(driver.cvt_itoe($1)))));
		    driver.clear();
		}
	|	UPDATE tablename SET colname '=' expr WHERE expr ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(
                driver.cvt_itoe(driver.makeexpr(OP_UPDATE, 2, $2,
                    driver.makeexpr(OP_PROJECTION,2,
                        driver.makeexpr(OP_SELECTION, 2, $2, $8),
                        driver.makeexpr(OP_RLIST,2,
                            driver.makeexpr(OP_AS,2,$4,$6),0))))))));
		    driver.clear();
		}
	|	DELETE FROM tablename WHERE expr ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(
		        driver.cvt_itoe(driver.makeexpr(OP_DELETE, 2, $3, $5))))));
		    driver.clear();
		}
	|	CREATE TABLE tablename '(' column_decls ')' ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(driver.cvt_itoe(
		        driver.makeexpr(OP_CREATETABLE, 2, $3, $5))))));
		    $$=0;
		    driver.clear();
		}
	|	DESCRIBE tablename ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(
		        driver.cvt_itoe(driver.makeexpr(OP_DESCRIBE, 1, $2, 0))))));
		    driver.clear();
		}
	|	SHOW TABLES ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(
		        driver.cvt_itoe(driver.makeexpr(OP_SHOWTABLES, 0, 0, 0))))));
		    driver.clear();
		}
	|	DROP TABLE tablename ';'
		{   driver.print_relation(driver.evalexpr(driver.optimize(driver.compile(driver.cvt_itoe(
		        driver.makeexpr(OP_DROPTABLE, 2, $3, 0))))));
		    driver.clear();
		}
	|	error ';'
		{   driver.clear(); YYACCEPT;  }
	;

tablename:	IDENT   { $$ = driver.setname(OP_TABLENAME, $1); }
    |	error ';'   { std::cout << "Invalid tablename\n"; driver.clear(); YYACCEPT; }
	;

colname	:	IDENT   { $$ = driver.setname(OP_COLNAME, $1); }
    |	error ';'   { std::cout << "Invalid column name\n"; driver.clear(); YYACCEPT; }
	;

expr:  value
	|  expr bin_op value               { $$ = driver.makeexpr($2, 2, $1, $3); }
	|  expr bin_op '(' expr ')'        { $$ = driver.makeexpr($2, 2, $1, $4); }
	|  expr bin_op '(' select_expr ')' { $$ = driver.makeexpr($2, 2, $1, $4); }
	|  expr bin_op '(' data_list ')'   { $$ = driver.makeexpr($2, 2, $1, $4); }
	|  '(' select_expr ')'             { $$ = $2; }
	|  '(' expr ')'                    { $$ = $2; }
    |  fcall                           { $$ = $1; }
    ;

fcall: IDENT '(' fncargs ')'  { $$ = driver.makeexpr(OP_FCALL, 2, driver.setname(OP_FNAME, $1), $3); }
    |  IDENT '(' ')'          { $$ = driver.makeexpr(OP_FCALL, 2, driver.setname(OP_FNAME, $1), 0); }
    ;

fncargs : expr             { $$ = driver.makeexpr(OP_RLIST, 2, $1, 0); }
    |     fncargs ',' expr { $$ = driver.makeexpr(OP_RLIST, 2, $3, $1); }
    ;

value:	IDENT               { $$ = driver.setname(OP_COLNAME, $1); }
	|	IDENT '.' IDENT     { $$ = driver.setname(OP_COLNAME, $3, $1); }
	|	NUMBER              { $$ = driver.makenum($1); }
	|	STRING              { $$ = driver.makestrexpr($1); }
	;

select_expr
    :	SELECT projection FROM join whereclause groupclause havingclause orderclause
		{ $$ = $4;
		  if($5) $$ = driver.makeexpr(OP_SELECTION, 2, $$, $5);
		  if($6) $$ = driver.makeexpr(OP_GROUP, 2, $$, $6);
		  $$ = driver.makeexpr(OP_PROJECTION, 2, $$, $2);
		  if($7) $$ = driver.makeexpr(OP_SELECTION, 2, $$, $7);
		  if($8) $$ = driver.makeexpr(OP_SORT, 2, $$, $8);
		}
	;

whereclause:        { $$ = 0; }
	|	WHERE expr  { $$ = $2; }
	;

havingclause:       { $$ = 0; }
    |	HAVING expr	{ $$ = $2; }
    ;

groupclause:                { $$ = 0; }
    |	GROUP BY grouplist  { $$ = $3; }
    ;

grouplist: groupspec               { $$ = driver.makeexpr(OP_RLIST, 2, $1, 0); }
    |	   grouplist ',' groupspec { $$ = driver.makeexpr(OP_RLIST, 2, $3, $1); }
    ;

groupspec:	colname
	;

orderclause:                { $$ = 0;  }
    |	ORDER BY orderlist  { $$ = $3; }
    ;

orderlist: orderspec               { $$ = driver.makeexpr(OP_RLIST, 1, $1, 0);  }
    |	   orderlist ',' orderspec { $$ = driver.makeexpr(OP_RLIST, 2, $3, $1); }
    ;

orderspec:	NUMBER   { $$ = driver.makeexpr(OP_SORTSPEC, 2, $1, 0); }
	|	NUMBER ASC   { $$ = driver.makeexpr(OP_SORTSPEC, 2, $1, 0); }
	|	NUMBER DESC  { $$ = driver.makeexpr(OP_SORTSPEC, 2, $1, driver.makeexpr(OP_NULL, 0, 0, 0)); }
	|	colname      { $$ = driver.makeexpr(OP_SORTSPEC, 2, $1, 0); }
	|	colname ASC  { $$ = driver.makeexpr(OP_SORTSPEC, 2, $1, 0); }
	|	colname DESC { $$ = driver.makeexpr(OP_SORTSPEC, 2, $1, driver.makeexpr(OP_NULL, 0, 0, 0)); }
	;

projection:	outcol              { $$ = driver.makeexpr(OP_RLIST, 2, $1, 0); }
    |   DISTINCT outcol         { $$ = driver.makeexpr(OP_DISTINCT, 1, driver.makeexpr(OP_RLIST, 2, $2, 0), 0); }
	|	projection ',' outcol   { $$ = driver.makeexpr(OP_RLIST, 2, $3, $1); }
	;

outcol:	expr AS colname { $$ = driver.makeexpr(OP_OUTCOLNAME, 2, $1, $3); }
	  |	colname         { $$ = driver.makeexpr(OP_OUTCOLNAME, 2, $1, $1); }
	  ;

join:	tablename           { $$ = $1; }
	|	join ',' tablename  { $$ = driver.makeexpr(OP_PRODUCT, 2, $1, $3); }
	|	join JOIN tablename ON expr
        {   $$ = driver.makeexpr(OP_PRODUCT, 2, $1, $3);
            $$ = driver.makeexpr(OP_SELECTION, 2, $$, $5);
        }
	;

data_list:	NUMBER           { $$ = driver.makeexpr(OP_RLIST, 2, driver.makenum($1), 0); }
	|	STRING               { $$ = driver.makeexpr(OP_RLIST, 2, driver.makestrexpr($1), 0); }
	|	NUL                  { $$ = driver.makeexpr(OP_RLIST, 2, driver.makeexpr(OP_NULL,0,0, 0), 0); }
	|	data_list ',' NUMBER { $$ = driver.makeexpr(OP_RLIST, 2, driver.makenum($3), $1); }
	|	data_list ',' STRING { $$ = driver.makeexpr(OP_RLIST, 2, driver.makestrexpr($3), $1); }
	|	data_list ',' NUL    { $$ = driver.makeexpr(OP_RLIST, 2, driver.makeexpr(OP_NULL, 0, 0, 0), $1); }
	|   error                { std::cout << "Unterminated string detected.\n"; driver.clear(); YYACCEPT; }
	;

column_decls:   column_decl                     { $$ = driver.makeexpr(OP_RLIST, 2, $1, 0); }
	        |	column_decls ',' column_decl    { $$ = driver.makeexpr(OP_RLIST, 2, $3, $1); }
	        ;

column_decl:    colname             { $$ = driver.makeexpr(OP_COLUMNDEF, 2, $1, 0); }
	       |	colname KEY         { $$ = driver.makeexpr(OP_COLUMNDEF, 2, $1, 1); }
	       |	colname PRIMARY KEY { $$ = driver.makeexpr(OP_COLUMNDEF, 2, $1, 3); }
	       ;

bin_op:	'='	    { $$ = OP_EQUAL; }
	|	NOTEQ	{ $$ = OP_NOTEQ; }
	|	'>'	    { $$ = OP_GT; }
	|	'<' 	{ $$ = OP_LT; }
	|	GEQ	    { $$ = OP_GEQ; }
	|	LEQ	    { $$ = OP_LEQ; }
	|	'+'	    { $$ = OP_PLUS; }
	|	'-'	    { $$ = OP_BMINUS; }
	|	'*'	    { $$ = OP_TIMES; }
	|	'/'	    { $$ = OP_DIVIDE; }
	|	AND	    { $$ = OP_AND; }
	|	OR	    { $$ = OP_OR; }
	|	IN	    { $$ = OP_IN; }
	;

%%

void UCD::SQLParser::error( const location_type &l, const std::string &err_message ) {
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
