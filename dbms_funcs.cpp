#include <dirent.h>
#include <string.h>
#include <string> 
#include <vector>
#include <iostream>
#include <sstream> 
#include "sql_expr.h"
#include "globals.h"
#include "Table.h"
#include "btree.h" // btree if from https://panthema.net/2007/stx-btree/ 
using namespace std;

extern vector<Table> tables;
extern Table new_table;
extern newRow new_row;
extern newSFW new_sfw; 
extern bool reading_select;
extern bool reading_group_by;
extern stack<string> symbol_stack;
extern bool file_data_loaded;
extern void init_new_sfw();

void updateNumberOfUniqueValuesVector(string table_name, vector<string> &newValues);
bool comparisonContainsTwoColumns(string lhs, string rhs);

// array of symbols/Keywords, used when parsing SELECT and WHERE expressions
string compare_ops[] = {"=",">","<","<>"};
string where_keywords[] = {"AND","AS","IN"};
string arithmetic_ops[] = {"+","-","*","/"};

// Utility Functions
string make_lower_case(string the_string) {
    string temp_string = the_string;
    // change input to lower case
    for (int i = 0; i < temp_string.size(); i++) temp_string[i] = tolower(temp_string[i]);
    return (temp_string);
}
string make_upper_case(string the_string) {
    string temp_string = the_string;
    // change input to lower case
    for (int i = 0; i < temp_string.size(); i++) temp_string[i] = toupper(temp_string[i]);
    return (temp_string);
}
bool string_to_bool(string input) {
    string low_input = make_lower_case(input);
    if (input == "0")
        return false;
    else
        return true;
}
bool is_compare_op(string str) {
    bool compare_op = false;
    string temp_str = make_upper_case(str);
    int i = 0;
    while (i < (sizeof(compare_ops)/sizeof(compare_ops[0])) && compare_op == false) {
        if (temp_str == compare_ops[i] )
            compare_op = true;
        i++;
    }
    return (compare_op);
}

// DBMS functions

/*
 * Returns index in tables vector of Table with name table_name
 */
int table_index(string table_name) {
    int i = 0;
    bool found = false;
    while (found == false && i < tables.size()) {
        if (tables[i].GetTableName() == table_name)
            found = true;
        if (found == false) i++;
    }
    if (found)
        return i;
    else
        return(-99);
}

/*
 * Returns true if strings in values are valid database values (Either integers, quoted strings, or NULL)
 */
bool valid_values(vector<string> &values) {
    bool valid = true;
    for (int i = 0; i < values.size(); i++) {
        // If value is a quoted string, it's valid so contiue and check next value
        if (values[i].at(0) == '\'' && values[i].at(values[i].size()-1) == '\'') continue;
        // If value us "NULL", it's valid so continue and check next value
        else if (values[i] == "NULL") continue;
        else {
            for (int j = 0; j < values[i].size(); j++) {
                // Iterate through characters and ensure we are looking at a number.
                if (!isdigit(values[i].at(j))){
                    valid = false;
                    break;
                }
            }
        }
    }
    return valid; 
}


// Checks vector for duplicate values
bool has_duplicates(vector<string> &values){
    vector<string> values_ = values;
    // Sort the vector and check for duplicates
    sort(values_.begin(), values_.end());

    bool result = false;
    for (int i = 0; i < (values_.size()-1); i++)
        for (int j = i+1; j < values_.size(); j++)
            if (values_[i] == values_[j]){
                result = true;
                return result;
            }
    return result;
}
string load_row_lookup_table() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    DIR *dp;
    struct dirent *dirp;
    string dir = "./data";
    Table new_table;
    ifstream in;
    string file_path, table_name;
    string input;
    vector<string> column_names;
    int status;
    if((dp  = opendir(dir.c_str())) != NULL) {
        unsigned int i = 0;
        while ((dirp = readdir(dp)) != NULL) {
            // This if ensures '.' and '..' directories are not added as tables.
            if (i > 1){
                input.clear();
                column_names.clear();
                table_name = string(dirp->d_name);
                file_path = "./data/" + table_name; // table_name includes extension at this point
                // Skip directories
                status = stat(file_path.c_str(), &sb);
                if(!S_ISDIR(sb.st_mode)) {
                    // Strip off .tbl extension
                    table_name = table_name.substr(0,table_name.size()-4);
                    new_table.SetTableName(table_name);
                    // open file for reading
                    in.open(file_path.c_str());
                    if (!in.is_open())
                        return ("Cannot open file");
                    getline(in, input);
                    stringstream ss;
                    ss << input;
                    // Read in column names
                    while (getline(ss,input,'\t')) {
                        column_names.push_back(input);
                    }
                    in.close();
                    new_table.SetColumnNames(column_names);
                    tables.push_back(new_table);
                }
            }
            i++;
        }
        closedir(dp);
    }
    if (tables.size() == 0)
        return ("No data in data directory");
    else
        return ("0");
}


string load_data() {
    ifstream in;
    string file_path;
    string input;
    vector<string> values;
    for (int i = 0; i < tables.size(); i++) {
        file_path = "./data/" + tables[i].GetTableName() + ".tbl";
        // open file for reading
        in.open(file_path.c_str());
        if (!in.is_open())
            return ("Cannot open file");
        getline(in, input); // this is column names, don't need these
        // Read in values
        while (getline(in, input)) {
            stringstream ss;
            ss << input;
            values.clear();
            while (getline(ss,input,'\t')) {
                values.push_back(input);
            }
            tables[i].InsertRow(values);
        }
        in.close();
    }
    return("0");
}

bool InVector(unsigned index, vector<unsigned> &the_vector) {
    bool found = false;
    unsigned i = 0;
    while (found == false && i < the_vector.size()){
        if (index == the_vector[i])
            found = true;
        i++;
    }
    return found;
}
bool table_in_database(string table_name) {
    bool found = false;
    unsigned i = 0;
    
    while (!found && i < tables.size()){
        if (table_name == tables[i].GetTableName()) 
            found = true;
        i++;
    }
    return found;
}
bool is_where_keyword(string str) {
    bool where_keyword = false;
    string temp_str = make_upper_case(str);
    int i = 0;
    while (i < (sizeof(where_keywords)/sizeof(where_keywords[0])) && where_keyword == false) {
        if (temp_str == where_keywords[i] ) 
            where_keyword = true;
        i++;
    }
    return (where_keyword);
}
bool is_arithmetic_op(string str) {
    bool arithmetic_op = false;
    string temp_str = make_upper_case(str);
    int i = 0;
    while (i < (sizeof(arithmetic_ops)/sizeof(arithmetic_ops[0])) && arithmetic_op == false) {
        if (temp_str == arithmetic_ops[i] ) 
            arithmetic_op = true;
        i++;
    }
    return (arithmetic_op);
}

// Does a tree traversal while extracting necessary data for
// create table function
void createTableEntry(UCD::SQLExpression *ep)
{
    if (ep) {
        if (ep->getFunc() == OP_COLNAME) {
            new_table.AddColumnName(ep->getName(0));
            // Add place holder for btree index in case it is used.
            stx::btree<string, int> new_tree;
            new_table.indexes.push_back(new_tree);
            
        }
        else if (ep->getFunc() == OP_TABLENAME) {
            new_table.SetTableName(string(ep->getName(0)));
        }
        else if (ep->getFunc() == OP_COLUMNDEF) {
            if (ep->getNum(1) == 1){
                new_table.AddKey(true);
                new_table.AddPrimaryKey(false);
            }
            else if (ep->getNum(1)==3) {
                new_table.AddKey(true);
                new_table.AddPrimaryKey(true);
            }
            else {
                new_table.AddKey(false);
                new_table.AddPrimaryKey(false);
            }
            createTableEntry(ep->getExpression(0));
        }
        else {
            createTableEntry(ep->getExpression(0));
            createTableEntry(ep->getExpression(1));
        }
    }
}
void createRowEntry(UCD::SQLExpression *ep) {
    if (ep) {
        if (ep->getFunc() == OP_NUMBER)
            new_row.values.push_back(to_string(ep->getNum(0)));
        else if (ep->getFunc() == OP_STRING)
            new_row.values.push_back(ep->getData(0));
        else if (ep->getFunc() == OP_NULL)
            new_row.values.push_back("NULL");
        else if (ep->getFunc() == OP_TABLENAME)
            new_row.table_name = string(ep->getName(0));
        else {
            createRowEntry(ep->getExpression(0));
            createRowEntry(ep->getExpression(1));
        }
    }
}
void createSFWEntry(UCD::SQLExpression *ep) {
    register int i;
    
    if(!ep) 
        return;
    switch(ep->getFunc()) {

    /* Literals */
    case OP_NUMBER: {
        if (reading_select && !new_sfw.readingIN) {
            new_sfw.select_input.push_back(to_string(ep->getNum(0)));
            if (!symbol_stack.empty()){
                new_sfw.select_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        }
        else if (new_sfw.readingIN) {
            new_sfw.in_list.push_back(to_string(ep->getNum(0)));
        }
        else {
            new_sfw.where_input.push_back(to_string(ep->getNum(0)));
            if (!symbol_stack.empty()) {
                new_sfw.where_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        } 
        return;
    }
    case OP_STRING: {
        if (reading_select && !new_sfw.readingIN)
            cerr << "Recieved OP_STRING while reading from SELECT, but no code to handle" << endl;
        else if (new_sfw.readingIN) {
            new_sfw.in_list.push_back(string(ep->getData(0)));
        }
        else {// reading 
            new_sfw.where_input.push_back(string(ep->getData(0)));
            if (!symbol_stack.empty()) {
                new_sfw.where_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        }
        return;
        //printf("%s ", ep->getData(0)); return;
    }
    case OP_NULL: {
        if (reading_select) {
            new_sfw.select_input.push_back("NULL");
            if (!symbol_stack.empty()) {
                new_sfw.select_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        }
        else {
            new_sfw.where_input.push_back("NULL");
            if (!symbol_stack.empty()) {
                new_sfw.where_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        } 
        return;
    }   
        

    /* Names */
    case OP_COLNAME: {
		if (reading_group_by) {
			new_sfw.groupByColName = string(ep->getName(0));
			reading_group_by = false;
		} else if (reading_select) {
            new_sfw.select_input.push_back(string(ep->getName(0)));
            if (!symbol_stack.empty()) {
                new_sfw.select_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        }
        else if (new_sfw.readingIN) {
            new_sfw.in_col_name = string(ep->getName(0));
        }
        else {
            new_sfw.where_input.push_back(string(ep->getName(0)));
            if (!symbol_stack.empty()) {
                new_sfw.where_input.push_back(symbol_stack.top());
                symbol_stack.pop();
            }
        } 
        return;
    }
    case OP_TABLENAME: {
        new_sfw.table_name.push_back(string(ep->getName(0)));    
        return;
    }
    case OP_FNAME: {
        cout << "Adding aggregate function" << endl;
        new_sfw.callsAggregateFunction = true;
        new_sfw.aggregateFunctionName = string(ep->getName(0));
        //printf("FUNC:%s ", ep->getName(0)); return;
    }
    case OP_COLUMNDEF: {
        cerr << "Recieved OP_COLUMNDEF, but no code to handle" << endl;
        //printf("(COLSPEC ");
        //printf("%s ", ep->values[1].num==1?"KEY":ep->values[1].num==3?"PRIMARY":"");
        //print_e(ep->values[0].ep, lev+2);
        //putchar(')');
        return;
    }
    /* Relational operators */
    case OP_PRODUCT:
            break;
    case OP_SORT:
            cerr << "ERROR: Not yet implemented" << endl; break;
    case OP_GROUP:
            reading_group_by = true;
            new_sfw.callsGroupBy = true;
            cout << "Calling GROUP BY" << endl;
            break;
    case OP_DELTA:
            cerr << "ERROR: Not yet implemented" << endl; break;
    
    case OP_PLUS: symbol_stack.push("+"); break;    
    case OP_BMINUS: symbol_stack.push("-"); break;
    case OP_TIMES:  symbol_stack.push("*"); break;
    case OP_DIVIDE: symbol_stack.push("/"); break;

    case OP_AND:    symbol_stack.push("AND"); break;
    case OP_OR:     cerr << "ERROR: Not yet implemented" << endl; break;
    case OP_NOT:    cerr << "ERROR: Not yet implemented" << endl; break;
    case OP_GT:     symbol_stack.push(">"); break;
    case OP_LT:     symbol_stack.push("<"); break;
    case OP_EQUAL:  symbol_stack.push("="); break;
    case OP_NOTEQ:  symbol_stack.push("<>"); break;
    case OP_GEQ:    symbol_stack.push(">="); break;
    case OP_LEQ:    symbol_stack.push("<="); break;

    case OP_SORTSPEC:
            cerr << "ERROR: Not yet implemented" << endl; break;

    case OP_OUTCOLNAME:
            symbol_stack.push("AS"); new_sfw.readingIN = false; break;

    case OP_RLIST:  reading_select = true; break;
    case OP_IN:     new_sfw.readingIN = true; new_sfw.isInSFW = true; break;
    }
    createSFWEntry(ep->getExpression(0));
    createSFWEntry(ep->getExpression(1)); 
}
// Seperates and reorders the expressions in the SELECT and WHERE statements.
// Reordering needed because the list is reversed when provided.
void parseSelectNWhere() {
    int j = 0;
    int i = 0;
    bool compare_seen = false;
    vector<string> temp;
    while (i < new_sfw.where_input.size()) {
        if (new_sfw.where_input[i] == "AND"){
            j++;
            compare_seen = false;
        }
        else if (is_compare_op(new_sfw.where_input[i])) {
            compare_seen = true;
            new_sfw.where_compare_op.push_back(new_sfw.where_input[i]);
        } 
        else if (compare_seen) {
            if (new_sfw.where_cond_RHS.size() < j+1)
                new_sfw.where_cond_RHS.push_back(temp);
            new_sfw.where_cond_RHS[j].push_back(new_sfw.where_input[i]);
        }
        else {
            if (new_sfw.where_cond_LHS.size() < j+1)
                new_sfw.where_cond_LHS.push_back(temp);
            new_sfw.where_cond_LHS[j].push_back(new_sfw.where_input[i]);
        }
        i++;
    }
    i = 0;
    j = 0;
    while (i < new_sfw.select_input.size()) {
        if (new_sfw.select_input[i] == "AS") {
            i++;
            new_sfw.proj_as_column_names.push_back(new_sfw.select_input[i]);
            j++;
        } 
        else {
            if (new_sfw.proj_expressions.size() < j+1)
                new_sfw.proj_expressions.push_back(temp);
            new_sfw.proj_expressions[j].push_back(new_sfw.select_input[i]);
        }

        i++;
    }
}
string fetch_rows(string table_name, string projection, string condition ) {
    stringstream ss;
    string where_column_name, where_cond_value, where_cond_operator, input;
    vector<string> proj_column_names;
    // extract needed info from the condition
    ss << condition;
    getline(ss,where_column_name, ' ');
    getline(ss, where_cond_operator, ' ');
    getline(ss, where_cond_value,' ');
    ss.str("");
    ss.clear();
    ss << projection;
    while (getline(ss,input,','))
        proj_column_names.push_back(input);
    if (!table_in_database(table_name))
        return ("Table does not exist");
    for (unsigned i = 0; i < proj_column_names.size(); i++)
    if (!tables[table_index(table_name)].HasColumn(where_column_name))
        return ("Column used in WHERE clause does not exist");
    // Check if data has been loaded and load data for this table if not.
    if (!tables[table_index(table_name)].GetDataLoaded())
       tables[table_index(table_name)].loadTableIntoMemory();
	
	int opcode;
	
	if(where_cond_operator == "=") {
		opcode = 0;
	} else if(where_cond_operator == ">") {
		opcode = 1;
	} else if(where_cond_operator == "<") {
		opcode = 2;
	} else if(where_cond_operator == "<>") {
		opcode = 3;
	} else {
		return("Error: Illegal operator in WHERE condition");
	}
	
    vector<vector<string> > results = tables[table_index(table_name)].SearchTable(where_column_name, where_cond_value, opcode);
    vector<string> columns = tables[table_index(table_name)].GetColumnNames();
    vector<unsigned> print_indices;
    for (unsigned i = 0; i < proj_column_names.size(); i++)
        print_indices.push_back(tables[table_index(table_name)].ColumnIndex(proj_column_names[i]));
    for (unsigned i = 0; i < print_indices.size(); i++) {
            cout << columns[print_indices[i]] << "\t\t";
    }
    cout << endl;
    for (unsigned i = 0; i < columns.size(); i++) {
        if (InVector(i, print_indices))
            cout << "===============";
    }
    cout << endl;
    for (unsigned i = 0; i < results.size(); i++) {
        for (unsigned j = 0; j < print_indices.size(); j++) {
            cout << results[i].at(print_indices[j]) << "\t\t";
        }
        cout << endl;
    }
    cout << results.size() << " rows fetched" << endl;
    // Done, so unload table data
    tables[table_index(table_name)].clearData();
    return("0");
}
string exec_SFW() {
    string projection, condition;
    for (int i = 0; i < new_sfw.proj_as_column_names.size(); i++) {
        projection += new_sfw.proj_as_column_names[i];
        if (i < new_sfw.proj_as_column_names.size()-1)
            projection += ",";
    }
    for (int i = 0; i < new_sfw.where_input.size(); i++) {
        condition += new_sfw.where_input[i]; 
        condition += " ";
    }
    string result = fetch_rows(new_sfw.table_name[0], projection, condition);
    return result;
}
string exec_in_SFW() {
    if (!tables[table_index(new_sfw.table_name[0])].GetDataLoaded())
        tables[table_index(new_sfw.table_name[0])].loadTableIntoMemory();
    vector<vector<string> > temp_results;
    vector<vector<string> > results = tables[table_index(new_sfw.table_name[0])].SearchTable(new_sfw.in_col_name, new_sfw.in_list[0], 0);
    vector<string> temp_row;
    for (int i = 1; i < new_sfw.in_list.size(); i++) {
        temp_results.clear();
        temp_results = tables[table_index(new_sfw.table_name[0])].SearchTable(new_sfw.in_col_name, new_sfw.in_list[i], 0);
        for (int j = 0; j < temp_results.size(); j++) {
            temp_row.clear();
            temp_row = temp_results[j];
            results.push_back(temp_row);
        }
    }


    vector<string> columns = tables[table_index(new_sfw.table_name[0])].GetColumnNames();
    vector<string> proj_column_names = new_sfw.proj_as_column_names;
    vector<unsigned> print_indices;
    for (unsigned i = 0; i < proj_column_names.size(); i++)
        print_indices.push_back(tables[table_index(new_sfw.table_name[0])].ColumnIndex(proj_column_names[i]));
    for (unsigned i = 0; i < print_indices.size(); i++) {
            cout << columns[print_indices[i]] << "\t\t";
    }
    cout << endl;
    for (unsigned i = 0; i < columns.size(); i++) {
        if (InVector(i, print_indices))
            cout << "===============";
    }
    cout << endl;
    for (unsigned i = 0; i < results.size(); i++) {
        for (unsigned j = 0; j < print_indices.size(); j++) {
            cout << results[i].at(print_indices[j]) << "\t\t";
        }
        cout << endl;
    }
    cout << results.size() << " rows fetched" << endl;

     
    tables[table_index(new_sfw.table_name[0])].clearData();
    return("0");
}
string delete_table_file(string table_name) {
    // Find table in global tables vector.
    bool found = false;
    int table_index, i = 0;
    while (!found && i < tables.size()) {
        if (tables[i].GetTableName() == table_name) {
            found = true;
            table_index = i;
        }
        if (!found)
            i++;
    }
    // Now delete table from memory
    tables.erase(tables.begin()+table_index);
    // Now delete files from storage
    string file_path = "./data/" + table_name + ".tbl";
    // Check if table exists and delete it if so.
    struct stat fb;
    if (stat(file_path.c_str(), &fb) == 0) {
        string sys_arg = "rm " + file_path;
        int error = system(sys_arg.c_str());
        if (error == -1)
            return ("table delete unsuccesful. Table file may not exist.");
        else {
            // Now delete any index files that may exist
            file_path = "./data/indexes/" + table_name + ".idx";
            if (stat(file_path.c_str(), &fb) == 0) {
                string sys_arg = "rm " + file_path;
                error = system(sys_arg.c_str());
                if (error == -1)
                    return ("deleting index file.");
            }   
                
            // Now delete primary key files
            file_path = "./data/primary_keys/" + table_name + ".pkey";
            if (stat(file_path.c_str(), &fb) == 0) {
                string sys_arg = "rm " + file_path;
                error = system(sys_arg.c_str());
                if (error == -1)
                    return ("deleting primary key file.");
                else {
                    // Now delete key files
                    file_path = "./data/keys/" + table_name + ".key";
                    if (stat(file_path.c_str(), &fb) == 0) {
                        string sys_arg = "rm " + file_path;
                        error = system(sys_arg.c_str());
                        if (error == -1)
                            return ("deleting key file.");
                        else return("0");
                    }
                    else {
                        return("No key file to delete");
                    }
                }
            }
            else {
                return("No primary key file to delete");
            }            
        }
    }
    else 
        return ("Table does not exist");
    return("0");
}
void drop_table(UCD::SQLExpression *ep) {
    //expression *ep = (expression *)e;
    string table_name =  ep->getName(0);
    string result = delete_table_file(table_name);
    if (result != "0")
        cerr << "ERROR: " << result << endl;
}


string save_primary_keys(string table_name, int table_index) {
    // Check if data directory already exists, if not error
    struct stat sb;
    int dir_err;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    // Check if primary_keys directory exists, if not create
    else if (stat("./data/primary_keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
         dir_err = system("mkdir -p ./data/primary_keys");
    // If an error occured while creating primary_keys directory, report error
    if (dir_err == -1)
        return ("Cannot create primary_key directory");
    else {
        // Create .pkey file if it doesn't exist, else add to the file by overwritting whole file
        string file_path = "./data/primary_keys/" + table_name + ".pkey";
        ofstream out;
        out.open(file_path.c_str(), fstream::trunc);
        if (!out.is_open())
            return ("Cannot create primary key file");
        else {
            // Successfully opened file, now write data stucture to the file
            for (int j = 0; j < tables[table_index].GetPrimaryKeyVecSize(); j++) 
                out << tables[table_index].GetPrimaryKey(j) << "\t"; 
            out.close();
            return ("0");
        }
    }
    return ("0"); 
}
string save_keys(string table_name, int table_index) {
    // Check if data directory already exists, if not error
    struct stat sb;
    int dir_err;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    // Check if keys directory exists, if not create
    else if (stat("./data/keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
         dir_err = system("mkdir -p ./data/keys");
    // If an error occured while creating keys directory, report error
    if (dir_err == -1)
        return ("Cannot create key directory");
    else {
        // Create .key file if it doesn't exist, else add to the file by overwritting whole file
        string file_path = "./data/keys/" + table_name + ".key";
        ofstream out;
        out.open(file_path.c_str(), fstream::trunc);
        if (!out.is_open())
            return ("Cannot create key file");
        else {
            // Successfully opened file, now write data stucture to the file
            for (int j = 0; j < tables[table_index].GetKeyVecSize(); j++) 
                out << tables[table_index].GetKey(j) << "\t"; 
            out.close();
            return ("0");
        }
    }
    return ("0"); 
}
string save_stats(string table_name) {
	// Check if data directory already exists, if not error
	struct stat sb;
    int dir_err;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        return ("data directory does not exist");
	}
    // Check if stats directory exists, if not create
    else if (stat("./data/stats", &sb) != 0 || !S_ISDIR(sb.st_mode)) {
         dir_err = system("mkdir -p ./data/stats");
	}
	// If an error occured while creating stats directory, report error
	if (dir_err == -1)
        return ("Cannot create key directory");
    else {
        // Create .stat file if it doesn't exist, else add to the file by overwritting whole file
        string file_path = "./data/stats/" + table_name + ".stat";
        ofstream out;
        out.open(file_path.c_str(), fstream::trunc);
        if (!out.is_open())
            return ("Cannot create stat file");
        else {
            // Successfully opened file, now write data stucture to the file 
            int index = table_index(table_name);
            
            out << tables[index].GetNumRows() << "\n";
            out << tables[index].GetNumCols() << "\n";
            out << tables[index].GetNumBlocks() << "\n";

            vector<int> numUniqueValues = (tables[index]).GetNumberOfUniqueValuesVector();
            for(int j = 0; j < numUniqueValues.size(); j++) {
				out << numUniqueValues[j] << "\t";
			}
			out.close();
            return ("0");
        }
    }
    return ("0"); 
}
string define_table(string table_name, unsigned long num_columns, vector<string> &column_names, 
    vector<bool> primary_keys, vector<bool> keys, vector<stx::btree<string, int> > indexes) {
    // Check for duplicate column names and report if so. 
    if (has_duplicates(column_names)) return("Column names cannot have duplicates");
    int dir_err;
    struct stat sb;
    string file_path = "./data/" + table_name + ".tbl";
    // Check if data directory already exists, if not create it
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        dir_err = system("mkdir -p ./data");
    // If an error occured while creating data directory, report error
    if (dir_err == -1)
        return ("Cannot create database directory");
    else {
        // Check if table exists, if not create it
        struct stat fb;
        if (stat(file_path.c_str(), &fb) == 0)
            return ("Table already exists");
        else {
            ofstream out;
            out.open(file_path.c_str());
            if (!out.is_open())
                return ("Cannot create table");
            else {
                Table new_table_;
                new_table_.SetTableName(table_name);
                new_table_.SetColumnNames(column_names);
                new_table_.SetPrimaryKeys(primary_keys);
                new_table_.SetKeys(keys);
                new_table_.setNumBlocks(1);
                new_table_.indexes = new_table.indexes;
                tables.push_back(new_table_);           // Add table to global tables vector
                
                save_primary_keys(table_name, tables.size()-1);     // Save primary keys to file
                save_keys(table_name, tables.size()-1);             // Save keys to file
                save_stats(table_name);

                // Save new table to file
                for (int i = 0; i < column_names.size(); i++) {
                    out << column_names[i] << "\t";
                    if (!out) return ("Failed to write to file");
                }
                out << endl;
                out.close();
                return ("0");
            }
        }
    }
}
bool IsPrimaryKey(string table_name, int column_index) {
    // Find table in global tables vector.
    bool found = false;
    int table_index, i = 0;
    while (!found && i < tables.size()) {
        if (tables[i].GetTableName() == table_name) {
            found = true;
            table_index = i;
        }
        if (!found)
            i++;
    }
    return (tables[table_index].IsPrimaryKey(column_index));
}
bool IsKey(string table_name, int column_index) {
    // Find table in global tables vector.
    bool found = false;
    int table_index, i = 0;
    while (!found && i < tables.size()) {
        if (tables[i].GetTableName() == table_name) {
            found = true;
            table_index = i;
        }
        if (!found)
            i++;
    }
    return (tables[table_index].IsKey(column_index));
}
string GetColumnName(string table_name, int column_index) {
    // Find table in global tables vector.
    bool found = false;
    int table_index, i = 0;
    while (!found && i < tables.size()) {
        if (tables[i].GetTableName() == table_name) {
            found = true;
            table_index = i;
        }
        if (!found)
            i++;
    }
    return(tables[table_index].GetColumnName(column_index));
}
string add_to_index(string table_name, string column_name,int column_index ,string key, int offset) {
    // Find table in global tables vector.
    bool found = false;
    int table_index,dir_err, i = 0;
    while (!found && i < tables.size()) {
        if (tables[i].GetTableName() == table_name) {
            found = true;
            table_index = i;
        }
        if (!found)
            i++;
    }
    // Add key to btree index
    tables[table_index].indexes[column_index].insert(key,offset);
    // Check if data directory already exists, if not error
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/indexes", &sb) != 0 || !S_ISDIR(sb.st_mode))
         dir_err = system("mkdir -p ./data/indexes");
    // If an error occured while creating data directory, report error
    if (dir_err == -1)
        return ("Cannot create indexes directory");
    else {
        // Create .idx file if it doesn't exist, else add to the file by overwritting whole file
        string file_path = "./data/indexes/" + table_name + ".idx";
        ofstream out;
        out.open(file_path.c_str(), fstream::trunc);
        if (!out.is_open())
            return ("Cannot create index file");
        else {
            // Successfully opened file, now write data stucture to the file
            stx::btree<string,int>::iterator iter;
            for (int j = 0; j < tables[table_index].indexes.size(); j++) {
                out << tables[table_index].indexes[j].size() << endl;
                for (iter = tables[table_index].indexes[j].begin(); iter != tables[table_index].indexes[j].end(); iter++) {
                    out << iter.key() << "\t" << iter.data() << "\t";
                }
                out << endl;
            }
           
            out.close();
            return ("0");
        }
    }
    
    return("0");
}
bool isDuplicatePrimaryKey(string table_name, string value) {
    bool result = tables[table_index(table_name)].IsCurrentPrimaryKeyValue(value);
    return result;
}
string insert_row(string table_name, unsigned long num_values, vector<string> &values) {
    if (!valid_values(values)) return ("One or more values are not valid. Valid = 'value', 123, or NULL ");
    string file_path = "./data/" + table_name + ".tbl";
    struct stat fb;
    if (stat(file_path.c_str(), &fb) != 0)
        return ("Table does not exist");
    else {
        // open file for reading
        ifstream in;
        in.open(file_path.c_str());
        if (!in.is_open())
            return ("Cannot open file");
        string input;
        vector<string> column_names;
        getline(in, input);
        stringstream ss;
        ss << input;
        // Read in column names so we know how many columns there are
        while (getline(ss,input,'\t')) {
            column_names.push_back(input);
        }
        in.close();
        // Ensure the number of values given matches the table size
        if (values.size() < column_names.size())
            return("Too few values");
        else if (values.size() > column_names.size())
            return("Too many values");
        else {
            // Open file for writing in append mode
            ofstream out;
            out.open(file_path.c_str(), ofstream::app);
            if (!out.is_open())
                return ("Cannot open file for writing");
            
            string result;
            // Check if data has been loaded into memory and load data for this table if not.
            if (!tables[table_index(table_name)].GetDataLoaded())
                tables[table_index(table_name)].loadTableIntoMemory();
             // Check if primary key values have been loaded into memory, load if not
             // Used for duplicate primary key detection
             if (!tables[table_index(table_name)].GetPrimaryKeyValuesLoaded()) {
                result = tables[table_index(table_name)].LoadPrimayKeyValues();
                if (result != "0")
                    cerr << "ERROR: " << result << endl; 
             }
             
            int offset;
            bool indexed_row = false; 
            for (int i = 0; i < values.size(); i++){
                // If column is a primary key column
                if (IsPrimaryKey(table_name,i)) {   
                    // If the primary key already exists in the DB, do not insert the row
                    if (isDuplicatePrimaryKey(table_name, values[i])) {
                        out.close();
                        return("Primary key value already exists in table.");
                    }
                    // else put in memory
                    else
                        tables[table_index(table_name)].InsertPrimaryKeyValue(values[i]);
                }
                // Record offset in case this is an indexed row.
                if (i == 0) {
                    offset = out.tellp();
                    //cout << out.tellp() << endl;
                }
                // If this value is a key or primary key, then add it to the btree index or create one.
                if (IsPrimaryKey(table_name,i) || IsKey(table_name,i)) {
                    string result = add_to_index(table_name, GetColumnName(table_name, i),i ,values[i], offset);
                    if (IsPrimaryKey(table_name,i)) {
                        // Open file for writing in append mode
                        ofstream out2;
                        string file_path2 = "./data/primary_keys/" + table_name + ".pkv";
                        out2.open(file_path2.c_str(), ofstream::app);
                        out2 << values[i] << endl;
                        out2.close();
                    }
                    if (result != "0")
                        cerr << "ERROR: " << result << endl;
                }
                out << values[i] << "\t";
                if (!out) return ("Failed to write to file");
            }
            tables[table_index(table_name)].InsertRow(values);
            tables[table_index(table_name)].incrementNumRows();
            tables[table_index(table_name)].updateNumBlocks(fb.st_size);
            
            out << endl;
            out.close();
            
            // Update number of unique values in indexed columns and save table statistics
            updateNumberOfUniqueValuesVector(table_name, values);
            save_stats(table_name);
            // Done so clear data and primary key values from memory
            tables[table_index(table_name)].clearData();
            tables[table_index(table_name)].ClearPrimaryKeyValues();

            return("0");
        }
    }
}

void updateNumberOfUniqueValuesVector(string table_name, vector<string> &newValues) {
	int tbl_index = table_index(table_name);
    int unique_value_index = 0;
	vector<bool> keys = tables[tbl_index].GetKeys();
	
    //Needs to be changed once load_data() is obsolete
    
    if(!file_data_loaded) {
        tables[tbl_index].loadTableIntoMemory();
    }

	for(int i = 0; i < keys.size(); i++) {
        bool found = false;
        bool hasIndex = false;

		if(keys[i]) {
            hasIndex = true;

            for(int j = 0; j < tables[tbl_index].GetNumRows() - 1; j++) {
                
                if(strcmp(newValues[i].c_str(), tables[tbl_index].GetValue(j, i).c_str()) == 0) {
                    found = true;
                }
            }
		}

        if(hasIndex) {
            if(!found) {
                tables[tbl_index].incrementNumUniqueValues(unique_value_index);
            }
            unique_value_index++;
        }
	}

    //tables[tbl_index].clearData();
}
/*
bool containsValue(vector<string> knownValues, string value) {
    bool found = false;

    for(int i = 0; i < knownValues.size(); i++) {
        if(strcmp(value.c_str(), knownValues[i].c_str()) == 0) {
            found = true;
        }

    }

    return found;
}
*/
string load_keys() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("keys directory does not exist");
    DIR *dp;
    struct dirent *dirp;
    string dir = "./data/keys";
    ifstream in;
    string file_path, table_name, input;

    if((dp  = opendir(dir.c_str())) != NULL) {
        unsigned int i = 0;
        while ((dirp = readdir(dp)) != NULL) {
            // This if ensures '.' and '..' directories are not added as tables.
            if (i > 1) {
                table_name = string(dirp->d_name);
                file_path = "./data/keys/" + table_name; // table_name includes extension at this point
                // Strip off .key extension
                table_name = table_name.substr(0,table_name.size()-4);
                // Find table in global tables vector.
                bool found = false;
                int table_index, k = 0;
                while (!found && k < tables.size()) {
                    if (tables[k].GetTableName() == table_name) {
                        found = true;
                        table_index = k;
                    }
                    if (!found)
                        k++;
                }
                // open file for reading
                in.open(file_path.c_str());
                if (!in.is_open())
                    return ("Cannot open file");
                while (in >> input)
                    tables[table_index].AddKey(string_to_bool(input));
                in.close();
            }
            i++;
        }
        closedir(dp); 
    }
    else
        return ("Cannot open key (.key) file");
    return("0"); 
}
string load_stats() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        return ("data directory does not exist");
    }
    else if (stat("./data/stats", &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        return ("stats directory does not exist");
    }
    DIR *dp;
    struct dirent *dirp;
    string dir = "./data/stats";
    ifstream in;
    string file_path, table_name, input;
    if((dp  = opendir(dir.c_str())) != NULL) {
        unsigned int i = 0;
        while ((dirp = readdir(dp)) != NULL) {
            // This if ensures '.' and '..' directories are not added as tables.
            if (i > 1) {
                table_name = string(dirp->d_name);
                file_path = "./data/stats/" + table_name; // table_name includes extension at this point
                // Strip off .stat extension
                table_name = table_name.substr(0,table_name.size()-5);
                // Find table in global tables vector.
                bool found = false;
                int table_index, k = 0;
                while (!found && k < tables.size()) {
                    if (tables[k].GetTableName() == table_name) {
                        found = true;
                        table_index = k;
                    }
                    if (!found)
                        k++;
                }
                // open file for reading
                in.open(file_path.c_str());
                if (!in.is_open())
                    return ("Cannot open file");
                if(in >> input) {
                    tables[table_index].setNumRows(stoi(input));  
                }
                if(in >> input) {
                    tables[table_index].setNumCols(stoi(input));
                }
                if(in >> input) {
                    tables[table_index].setNumBlocks(stoi(input));
                }
                while(in >> input) {
                    tables[table_index].AddNumUniqueValues(stoi(input));
                }
                in.close();
            }
            i++;
        }
        closedir(dp); 
    }
    else
        return ("Cannot open key (.key) file");
    return("0");
}
string load_primary_keys() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/primary_keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("primary_keys directory does not exist");
    DIR *dp;
    struct dirent *dirp;
    string dir = "./data/primary_keys";
    ifstream in;
    string file_path, table_name, input;

    if((dp  = opendir(dir.c_str())) != NULL) {
        unsigned int i = 0;
        while ((dirp = readdir(dp)) != NULL) {
            // This if ensures '.' and '..' directories are not added as tables.
            if (i > 1) {
                table_name = string(dirp->d_name);
                file_path = "./data/primary_keys/" + table_name; // table_name includes extension at this point
                // Do not load this file if it is not a .pkey file
                if (table_name.substr(table_name.size()-5,table_name.size()) != ".pkey")
                    continue;
                // Strip off .pkey extension
                table_name = table_name.substr(0,table_name.size()-5);
                // Find table in global tables vector.
                bool found = false;
                int table_index, k = 0;
                while (!found && k < tables.size()) {
                    if (tables[k].GetTableName() == table_name) {
                        found = true;
                        table_index = k;
                    }
                    if (!found)
                        k++;
                }
                // open file for reading
                in.open(file_path.c_str());
                if (!in.is_open())
                    return ("Cannot open file");
                while (in >> input)
                    tables[table_index].AddPrimaryKey(string_to_bool(input));
                in.close();
            }
            i++;
        }
        closedir(dp); 
    }
    else
        return ("Cannot open primary key (.pkey) file");
    return("0");
}
string load_btree_indexes() {
    
    struct stat sb;
    int dir_err;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/indexes", &sb) != 0 || !S_ISDIR(sb.st_mode))
        dir_err = system("mkdir -p ./data/indexes");
    // If an error occured while creating primary_keys directory, report error
    if (dir_err == -1)
        return ("Cannot create indexes directory");
    DIR *dp;
    struct dirent *dirp;
    string dir = "./data/indexes";
    ifstream in;
    string file_path, table_name, input1, input2;
    stx::btree<string, int> new_tree;

    if((dp  = opendir(dir.c_str())) != NULL) {
        unsigned int i = 0;
        while ((dirp = readdir(dp)) != NULL) {
            // This if ensures '.' and '..' directories are not added as tables.
            if (i > 1) {
                table_name = string(dirp->d_name);
                file_path = "./data/indexes/" + table_name; // table_name includes extension at this point
                // Strip off .idx extension
                table_name = table_name.substr(0,table_name.size()-4);
                // Find table in global tables vector.
                bool found = false;
                int table_index, k = 0;
                while (!found && k < tables.size()) {
                    if (tables[k].GetTableName() == table_name) {
                        found = true;
                        table_index = k;
                    }
                    if (!found)
                        k++;
                }
                // open file for reading
                in.open(file_path.c_str());
                if (!in.is_open())
                    return ("Cannot open file");
                int loop_end,offset;
                while (in >> input1) {
                    istringstream buffer(input1);
                    loop_end;
                    buffer >> loop_end;
                    new_tree.clear();
                    for (int j = 0; j < loop_end; j++) {
                        in >> input1;
                        in >> input2;
                        istringstream buffer2(input2);
                        buffer2 >> offset;
                        new_tree.insert(input1, offset);
                    }
                    tables[table_index].indexes.push_back(new_tree);
                    
                }
                in.close();
            }
            i++;
        }
        closedir(dp);
    }
    else
        return ("Cannot open index file");
    return("0");
}
bool isIndexSelectCandidate() {
    bool result = false;
    if (new_sfw.table_name.size() == 1){
        for (int i = 0; i < new_sfw.where_cond_LHS.size(); i++) {
            if (tables[table_index(new_sfw.table_name[0])].IsKey(tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[i][0]))) {
                new_sfw.where_LHS_indexed.push_back(true);
                result = true;
            }
            else 
                new_sfw.where_LHS_indexed.push_back(false);
        }
    }
    return result;
}
bool doneWithAllConditions(vector<bool> doneWithCondition) {
    bool result = true;
    for (int i = 0; i < doneWithCondition.size(); i++) {
        if (!doneWithCondition[i])
            result = false;
    }
    return result;
}
void displayResultsForOneTable(vector<string> results) {
    vector<vector<string> > results_tokens;
    vector<string> new_vec, proj_column_names;
    stringstream ss;
    string input;
    // Tokenize the results
    for (int i = 0; i < results.size(); i++) {
        results_tokens.push_back(new_vec);
        ss.clear();
        ss.str(string());
        ss << results[i];
        while(getline(ss,input,'\t'))
            results_tokens[i].push_back(input);

    }
    // Fill proj_column_names with the names of the columns from SELECT
    for (int i =0; i < new_sfw.proj_expressions.size(); i++)
        proj_column_names.push_back(new_sfw.proj_expressions[i][0]);

    vector<string> columns = tables[table_index(new_sfw.table_name[0])].GetColumnNames();
    vector<unsigned> print_indices;
    for (unsigned i = 0; i < proj_column_names.size(); i++)
        print_indices.push_back(tables[table_index(new_sfw.table_name[0])].ColumnIndex(proj_column_names[i]));
    for (unsigned i = 0; i < print_indices.size(); i++) {
            cout << columns[print_indices[i]] << "\t\t";
    }
    cout << endl;
    for (unsigned i = 0; i < columns.size(); i++) {
        if (InVector(i, print_indices))
            cout << "===============";
    }
    cout << endl;
    for (unsigned i = 0; i < results_tokens.size(); i++) {
        for (unsigned j = 0; j < print_indices.size(); j++) {
            cout << results_tokens[i].at(print_indices[j]) << "\t\t";
        }
        cout << endl;
    }
    cout << results_tokens.size() << " rows fetched" << endl;

}
string indexSelection() {
    // doneWithCondition used so we know if we have handled the condition at position i within 'where_cond_LHS'
    cout << "Index Selection" << endl;
    vector<bool> doneWithCondition;
    // Results stored here
    vector<string> results;
    // initialize the doneWithCondition vector
    for (int i = 0; i < new_sfw.where_LHS_indexed.size(); i++)
        doneWithCondition.push_back(false);
    // Find first indexed column in where conditions, this is the only column that does an index selection
    bool found = false;
    int j = 0;
    while (j < new_sfw.where_LHS_indexed.size() && !found) {
        if (new_sfw.where_LHS_indexed[j])
            found = true;
        else
            j++;
    }
    // mark condition which will be applied first as done
    doneWithCondition[j] = true;
    // First find column index so we can access it's btree
    int column_index = tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[j][0]);
    // open file for reading
    string file_path = "./data/" + new_sfw.table_name[0] + ".tbl";
    fstream in;
    in.open(file_path.c_str());
    if (!in.is_open())
        return ("Cannot open file for input");
    
    // Do the index selection
    stx::btree<string,int>::iterator iter;
    int offset;
    string input;
    if (new_sfw.where_compare_op[j] == "=") {
        iter = tables[table_index(new_sfw.table_name[0])].indexes[column_index].find(new_sfw.where_cond_RHS[j][0]);
        // if result found
        if (iter !=  tables[table_index(new_sfw.table_name[0])].indexes[column_index].end()) {
            for (;iter != tables[table_index(new_sfw.table_name[0])].indexes[column_index].upper_bound(new_sfw.where_cond_RHS[j][0]); iter++) {
                offset = iter.data();
                // Move to offset
                in.seekg(offset);
                // get the row at the offset
                getline(in,input);
                results.push_back(input);
            }
        }
        else{
            cout << "No columns meet the condtions!" << endl;
            return("0");
        }
    }
    else if (new_sfw.where_compare_op[j] == "<>") {
        // If condition is <> (not equal), must iterate through each leaf of btree and check condition
        iter = tables[table_index(new_sfw.table_name[0])].indexes[column_index].begin();
        for (;iter != tables[table_index(new_sfw.table_name[0])].indexes[column_index].end(); iter++) {
            if (iter.key() != new_sfw.where_cond_RHS[j][0]) {
                offset = iter.data();
                // Move to offset
                in.seekg(offset);
                // get the row at the offset
                getline(in,input);
                results.push_back(input);
            }
        }
        if (results.size() < 1) {
            cout << "No columns meet the condtions!" << endl;
            return("0");
        }
    }
    else if (new_sfw.where_compare_op[j] == ">") {
        iter = tables[table_index(new_sfw.table_name[0])].indexes[column_index].upper_bound(new_sfw.where_cond_RHS[j][0]);
        // if result found
        if (iter !=  tables[table_index(new_sfw.table_name[0])].indexes[column_index].end()) {
            for (;iter != tables[table_index(new_sfw.table_name[0])].indexes[column_index].end(); iter++) {
                offset = iter.data();
                // Move to offset
                in.seekg(offset);
                // get the row at the offset
                getline(in,input);
                results.push_back(input);
            }
        }
        else{
            cout << "No columns meet the condtions!" << endl;
            return("0");
        }
    }
    else if (new_sfw.where_compare_op[j] == "<") {
        iter = tables[table_index(new_sfw.table_name[0])].indexes[column_index].lower_bound(new_sfw.where_cond_RHS[j][0]);
        // if result found
        if (iter !=  tables[table_index(new_sfw.table_name[0])].indexes[column_index].begin()) {
            iter--;
            for (;iter != tables[table_index(new_sfw.table_name[0])].indexes[column_index].begin(); iter--) {
                offset = iter.data();
                // Move to offset
                in.seekg(offset);
                // get the row at the offset
                getline(in,input);
                results.push_back(input);
            }
            offset = iter.data();
            // Move to offset
            in.seekg(offset);
            // get the row at the offset
            getline(in,input);
            results.push_back(input);
        }
        else{
            cout << "No columns meet the condtions!" << endl;
            return("0");
        }
    }
    else {
        return ("Invalid compare operator in WHERE.");
    }
    // narrow down result vector with rest of conditions
    stringstream ss;
    while (!doneWithAllConditions(doneWithCondition)) {
        // Find next WHERE condition that is not done
        for (j = 0; j < doneWithCondition.size(); j++) {
            if (!doneWithCondition[j]){
                doneWithCondition[j] = true;
                break;
            }    
        }
        if (new_sfw.where_compare_op[j] == "=") {
            for (int i = 0; i < results.size(); i++) {
                // Get column index so we can compare the correct value in the results vector
                column_index = tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[j][0]);
                // Find correct column in result vector to compare with
                ss.clear();
                ss.str(string());
                ss << results[i];
                for (int k = 0; k <= column_index; k++) {
                    getline(ss,input,'\t');
                }
                if (input != new_sfw.where_cond_RHS[j][0]) {
                    // Remove that row from the results vector
                    results.erase(results.begin()+(i));
                    i--;
                }
            }
        }
        else if (new_sfw.where_compare_op[j] == "<>") {
            for (int i = 0; i < results.size(); i++) {
                // Get column index so we can compare the correct value in the results vector
                column_index = tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[j][0]);
                // Find correct column in result vector to compare with
                ss.clear();
                ss.str(string());
                ss << results[i];
                for (int k = 0; k <= column_index; k++) {
                    getline(ss,input,'\t');
                }
                if (input == new_sfw.where_cond_RHS[j][0]) {
                    // Remove that row from the results vector
                    results.erase(results.begin()+(i));
                    i--;
                }
            }
        }
        else if (new_sfw.where_compare_op[j] == ">") {
             for (int i = 0; i < results.size(); i++) {
                // Get column index so we can compare the correct value in the results vector
                column_index = tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[j][0]);
                // Find correct column in result vector to compare with
                ss.clear();
                ss.str(string());
                ss << results[i];
                for (int k = 0; k <= column_index; k++) {
                    getline(ss,input,'\t');
                }
                if (input <= new_sfw.where_cond_RHS[j][0]) {
                    // Remove that row from the results vector
                    results.erase(results.begin()+(i));
                    i--;
                }
             }
        }
        else if (new_sfw.where_compare_op[j] == "<") {
            for (int i = 0; i < results.size(); i++) {
                // Get column index so we can compare the correct value in the results vector
                column_index = tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[j][0]);
                // Find correct column in result vector to compare with
                ss.clear();
                ss.str(string());
                ss << results[i];
                for (int k = 0; k <= column_index; k++) {
                    getline(ss,input,'\t');
                }
                if (input >= new_sfw.where_cond_RHS[j][0]) {
                    // Remove that row from the results vector
                    results.erase(results.begin()+(i));
                    i--;
                }
             }
        }
        else {
            return ("Invalid compare operator in WHERE.");
        }
    }
    displayResultsForOneTable(results);
    init_new_sfw();
    in.close();
    return("0");
}

/*
 * Returns true if lhs is greater than rhs
 * returns false if lhs and rhs are not the same database type
 */
bool greaterThanComparison(string lhs, string rhs) {
    bool comparisonHoldsTrue = false;

    bool lhs_is_string = false;
    bool rhs_is_string = false;

    if(lhs[0] == '\'') {
        lhs_is_string = true;
    }
    if(rhs[0] == '\'') {
        rhs_is_string = true;
    }

    if(lhs_is_string && rhs_is_string) {
        comparisonHoldsTrue = lhs > rhs;
    } else if(!(lhs_is_string || rhs_is_string)) {
        comparisonHoldsTrue = stoi(lhs) > stoi(rhs);
    }

    return comparisonHoldsTrue;
}

/*
 * Returns true if lhs is less than rhs
 * returns false if lhs and rhs are not the same database type 
 */
bool lessThanComparison(string lhs, string rhs) {
    bool comparisonHoldsTrue = false;

    bool lhs_is_string = false;
    bool rhs_is_string = false;

    if(lhs[0] == '\'') {
        lhs_is_string = true;
    }
    if(rhs[0] == '\'') {
        rhs_is_string = true;
    }

    if(lhs_is_string && rhs_is_string) {
        comparisonHoldsTrue = lhs < rhs;
    } else if(!(lhs_is_string || rhs_is_string)) {
        comparisonHoldsTrue = stoi(lhs) < stoi(rhs);
    }

    return comparisonHoldsTrue;
}

bool comparisonContainsTwoColumns(string lhs, string rhs) {
	bool containsTwoCols = false;

	
	//check that both sides of a condition are valid SQL identifiers
	bool lhs_is_col = false;
	bool rhs_is_col = false;		
	if((lhs[0] > 63 && lhs[0] < 90) || (lhs[0] > 95 && lhs[0] < 123)
			|| lhs[0] == 36) {
		lhs_is_col = true;
	}
	if((rhs[0] > 63 && rhs[0] < 90) || (rhs[0] > 95 && rhs[0] < 123)
			|| rhs[0] == 36) {
		rhs_is_col = true;
	}
	
	//check that the two SQL identifiers are not in the same table
	if(lhs_is_col && rhs_is_col) {
		containsTwoCols = true;
	}
	
	return containsTwoCols;
}
