#include "globals.h"
#include "Table.h"

// newRow variable to hold info on a newly created row
newRow new_row;

// newSFW (Select,From,Where) variable to hold SFW information for a new SFW.
newSFW new_sfw; 

// Globals
bool file_data_loaded;
bool print_error_message;

// Global tables vector, holds all tables
vector<Table> tables;
// newTable variable used for create table SQL command
Table new_table;
// State variable used for SFW(select,from,where) statement. Used to 
// differentiate betweem columns from SELECT and columns from WHERE
bool reading_select;
bool reading_group_by;
// Global symbol stack used for traversing the tree and extracting symbols
stack<string> symbol_stack;

void init_new_sfw() {
	new_sfw.table_name.clear();
    new_sfw.proj_expressions.clear();
    new_sfw.proj_as_column_names.clear();
    new_sfw.where_cond_LHS.clear();
    new_sfw.where_compare_op.clear();
    new_sfw.where_cond_RHS.clear();
    new_sfw.select_input.clear();
    new_sfw.where_input.clear();
    new_sfw.where_LHS_indexed.clear();
    new_sfw.join_condition_columns.clear();
    new_sfw.groupByColName = "";
    new_sfw.aggregateFunctionName = "";
    new_sfw.multipleTables = false;
    new_sfw.isNestedLoopJoin = false;
    new_sfw.isIndexJoin = false;
    new_sfw.isHashJoin = false;
    new_sfw.readingIN = false;
    new_sfw.in_list.clear();
    new_sfw.isInSFW = false;
    new_sfw.in_col_name = "";
    new_sfw.callsGroupBy = false;
    new_sfw.callsAggregateFunction = false;

}
