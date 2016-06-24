#ifndef GLOBALS_H
#define GLOBALS_H

#include <fstream>
#include <stack> 
#include <vector>
#include <string>
#include <stack>
using namespace std;

// Struct to hold info for a select,from,where statement
struct newSFW {
    vector<string> table_name;
    vector<vector<string > > proj_expressions;
    vector<string> proj_as_column_names;

    vector<vector<string > > where_cond_LHS;
    vector<string> where_compare_op;
    vector<vector<string > > where_cond_RHS;

    vector<string> select_input;
    vector<string> where_input;

    vector<bool> where_LHS_indexed;
    
    vector<string> join_condition_columns;
    vector<string> in_list; // Holds list of arguments when IN key word is used
    string in_col_name;
    
    string groupByColName;
    string aggregateFunctionName;
    
    bool multipleTables;
    bool isNestedLoopJoin;
    bool isIndexJoin;
    bool isHashJoin;
    bool readingIN; // flag used for parsing, so we know we are reading an IN list.
    bool isInSFW; // Used to indicate a situation in which the IN operator was used on the SFW expression.
    bool callsGroupBy;
    bool callsAggregateFunction;
};

// Structure to hold new row information
struct newRow {
    string table_name;
    vector<string> values;
};
#endif
