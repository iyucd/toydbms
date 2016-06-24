#ifndef TABLE_H
#define TABLE_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "btree.h" // btree if from https://panthema.net/2007/stx-btree/
using namespace std;

extern string make_lower_case(string the_string);
extern string make_upper_case(string the_string);

// Utility Function
extern bool string_to_bool(string input);

class Table {
private:
    // data members
    string name_;
    vector<string> column_names_;
    vector<vector<string> > data_; // Holds table data
    bool data_loaded_; // flag is true if data for this table is loaded.
    vector<bool> primary_key_; // flags which are true if column i is a primary key
    bool primary_key_flags_loaded_; // bool is true if primary_key_ (flags) are loaded
    vector<bool> key_;          //flags which are  true if column i is a key
    bool key_flags_loaded_; // bool is true if key_ (flags) are loaded
    //Holds all primary keys in table, for duplicate detection
    vector<string> primary_key_values_;
    bool primary_key_values_loaded_;
    bool btrees_loaded_;
    //statistics
    int numRows;
    int numCols;
    int numBlocks;
    vector<int> numUniqueValuesOfIndexedColumns;
public:
    // location i is the btree for column i in table, if one exists
    vector<stx::btree<string, int> > indexes; 
    // Constructors
    Table();
    Table(string table_name);
    Table(string table_name, vector<string> column_names);
    Table(string table_name, vector<string> column_names, vector<vector<string> > data);
    // Mutators
    void init(); 
    void SetTableName(string table_name);
    void SetColumnNames(vector<string> column_names);
    void initNumRows();
    void initNumUniqueValues();
    void setNumRows(int);
    void setNumCols(int);
    void setNumBlocks(int);
    void AddNumUniqueValues(int);
    void AddColumnName(string column_name);
    void InsertRow(vector<string> row);
    void SetPrimaryKeys(vector<bool> primary_keys);
    void AddPrimaryKey(bool value);
    void AddKey(bool value);
    void SetKeys(vector<bool> keys);
    void incrementNumRows();
    void incrementNumCols();
    void updateNumBlocks(off_t);
    void incrementNumUniqueValues(int index);
    void InsertPrimaryKeyValue(string value);
    string LoadPrimaryKeyFlags();
    void ClearPrimaryKeyFlags();
    string LoadKeyFlags();
    void ClearKeyFlags();
    string LoadPrimayKeyValues();
    void ClearPrimaryKeyValues();
    void SetPrimaryKeyValuesLoaded(bool new_value); // Sets the primary_key_values_loaded_ flag
    void clearData();
    string LoadBtrees();
    void ClearBtrees();
    void SetBtreesLoaded(bool new_value); // Sets the btrees_loaded_ flag
    void SetDataLoaded(bool new_value); // Sets the data_loaded_ flag
    void SetPrimaryKeyFlagsLoaded(bool new_value); // Sets the primary_key_flags_loaded_ flag
    void SetKeyFlagsLoaded(bool new_value); // Sets the key_flags_loaded_ flag
    // Loads 'data_', 'primary_key_' flags, 'key_' flags, 'primary_key_values_', 
    // and btree 'indexes'. Basically loads all table data
    /*string LoadAllDataIntoMemory();*/
    // Clears 'data_', 'primary_key_' flags, 'key_' flags, 'primary_key_values_', 
    // and btree 'indexes'. Basically clears all table data
    /*string ClearAllDataFromMemory();*/

    // Accessors
    void loadTableIntoMemory();
    string GetTableName();
    vector<string> GetColumnNames();
    string GetColumnName(int index);
    int GetColumnNamesVecSize();
    vector<vector<string> > SearchTable(string column_name, string value, int opcode);
    vector<bool> GetPrimaryKeys();
    bool GetPrimaryKey(int index);
    vector<bool> GetKeys();
    bool GetKey(int index);
    int GetPrimaryKeyVecSize();
    int GetKeyVecSize();
    int GetNumRows();
    int GetNumCols();
    int GetNumBlocks();
    vector<int> GetNumberOfUniqueValuesVector();
    bool IsPrimaryKey(int index);
    bool IsKey(int index);
    string GetValue(int row, int col);
    bool IsCurrentPrimaryKeyValue(string value);
    bool GetPrimaryKeyValuesLoaded();
    bool GetBtreesLoaded(); // returns value of btrees_loaded_ flag
    bool GetDataLoaded(); // returns value of data_loaded_ flag
    bool GetPrimaryKeyFlagsLoaded(); // returns value of primary_key_flags_loaded_ flag
    bool GetKeyFlagsLoaded(); // returns value of key_flags_loaded_ flag
    int GetColumnIndex(string name);
    vector<vector<string> > GetData();
    vector<string> GetRow(int rowNum);
    int GetNumRowsOfData();

    // Other member functions
    bool HasColumn(string column_name);
    int ColumnIndex(string column_name);
    void reverse_column_names();
    // Reverses both the key and primary key vectors so they are in the correct order
    void reverse_key_vectors();
};
#endif
