#ifndef SQLPARSER_SQL_DRIVER_H
#define SQLPARSER_SQL_DRIVER_H

#include <string>
#include <cstddef>
#include <istream>

#include "sql_driver.h"
#include "sql_expr.h"
//#include "sql_parser.hpp"
#include "sql_parser.tab.hh"

#define MAXNODE 20000

namespace UCD{

    class SQLDriver{
    public:
        int freen = 0;

        SQLDriver() = default;
        virtual ~SQLDriver();

        // Handling the scanner.
        bool trace_scanning;

        /**
         * parse - parse from a file
         * @param filename - valid string with input file
         */
        void parse( const char * const filename );
        /**
         * parse - parse from a c++ input stream
         * @param is - std::istream&, valid input stream
         */
        void parse( std::istream &iss );

        void clear();

        UCD::SQLExpression *makearray(UCD::SQLExpression);
        int listlen(UCD::SQLExpression);
        int makeexpr(int , int , int, int);
        int makestrexpr(std::string *);
        int makename(int, std::string *);
        int makename(int, std::string *, std::string *);
        int setname (int, std::string *);
        int setname (int, std::string *, std::string *);
        int makenum(int);
        bool quit();
        UCD::SQLExpression* cvt_itoe(int);
        UCD::SQLExpression* compile(UCD::SQLExpression*);
        UCD::SQLExpression* evalexpr(UCD::SQLExpression*);
        UCD::SQLExpression* optimize(UCD::SQLExpression*);
        void print_relation(UCD::SQLExpression*);
        void print();

    private:
        bool quitted = false;

        void print_exptree(SQLExpression *, int);
        void parse_helper( std::istream &stream );

        UCD::SQLParser  *parser  = nullptr;
        UCD::SQLScanner *scanner = nullptr;
    };

} /* end namespace MC */
#endif //SQLPARSER_SQL_DRIVER_H
