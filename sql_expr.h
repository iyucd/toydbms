#ifndef SQLPARSER_SQL_EXPR_H
#define SQLPARSER_SQL_EXPR_H

#include <string>
#include <fstream>

/* Relational operators */
#define OP_SELECTION	1
#define OP_PROJECTION	2
#define OP_PRODUCT	    3
#define OP_GROUP	    4
#define OP_DELTA	    5
#define OP_INSERT	    6
#define OP_DELETE	    7
#define OP_UPDATE	    8
#define OP_SORT		    9
#define OP_DISTINCT		109

/* Iteration */
#define OP_ITERATE	    10
#define OP_OPEN		    11
#define OP_GETNEXT	    12
#define OP_CLOSE	    13
#define OP_PROJECTROW	14
#define OP_SELECTROW	15
#define OP_INSERTROW	16
#define OP_DELETEROW	17
#define OP_FETCH1	    18
#define OP_SORTSPEC	    19

/* Standard arithmetic */
#define OP_EQUAL	    20
#define OP_NOTEQ	    21
#define OP_LEQ		    22
#define OP_GEQ		    23
#define OP_LT		    24
#define OP_GT		    25
#define	OP_PLUS		    26
#define OP_BMINUS	    27		/* Subtract */
#define OP_TIMES	    28
#define OP_DIVIDE	    29
#define OP_UMINUS	    30		/* Unary minus, negate */
#define OP_AND		    31
#define OP_OR		    32
#define OP_NOT		    33
#define OP_FCALL        34
#define OP_IN		    35

/* Transaction control */
#define OP_STXN		    40
#define OP_COMMIT	    41
#define OP_CLOSEALL	    42

/* DDL */
#define OP_SET		    50
#define OP_TABLENAME	51		/* Reference to a table */
#define OP_COLNAME	    52		/* Reference to a column */
#define OP_OUTCOLNAME	53		/* Rename of column (AS operator) */
#define OP_AS		    53		/* Rename of column (AS operator) */
#define OP_COLUMNDEF	54		/* A column definition (only to say if it is a key or primary key ) */
#define OP_CREATETABLE	55
#define OP_FNAME	    56
#define OP_DROPTABLE	57
#define OP_SHOWTABLES	58
#define OP_DESCRIBE	    59

/* Composition */
#define OP_RLIST	    60		/* List in reverse order */

/* String operators	*/
#define OP_STREQUAL	    70
#define OP_STRNOTEQ	    71
#define OP_STRLEQ	    72
#define OP_STRGEQ	    73
#define OP_STRLT	    74
#define OP_STRGT	    75
#define OP_STRCONCAT	76

/* Conversion operators */
#define	OP_ATOI		    80
#define OP_ITOA		    81

/* Literals */
#define	OP_STRING	    90
#define OP_NUMBER	    91
#define OP_NULL		    92
#define OP_ROW		    93
#define OP_EOF		    255

#define OP_ERROR	    -1

namespace UCD {
    class SQLExpression {
    public:
        int index;

        SQLExpression();
        ~SQLExpression();

        void clear();

        int getFunc();
        void setFunc(int);
        int getCount();
        void setCount(int);

        std::string getName(int);
        void setName(int,std::string);
        std::string getData(int);
        void setData(int,std::string);
        std::string getAltData(int);
        void setAltData(int,std::string);
        int getNum(int);
        void setNum(int,int);
        UCD::SQLExpression* getExpression(int);
        void setExpression(int,UCD::SQLExpression*);
        void print();
        void* execute();
    private:
        int func;
        int count;
        int type[2];  // node type data (0-name,1-data,3-num,4-child)

        int num[2];
        std::string data[2];
        std::string alt_data[2];
        UCD::SQLExpression* child[2];
    };
}
#endif //SQLPARSER_SQL_EXPR_H
