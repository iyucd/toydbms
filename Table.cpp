#include <string>
#include <iostream>
#include "Table.h"
using namespace std;

/*
 * Table class represents a relation stored in database
 */
Table::Table() {
    primary_key_values_loaded = false;
    primary_key_flags_loaded = false;
    key_flags_loaded = false;
    data_loaded = false;
    btrees_loaded = false;
    numRows = 0;
    numCols = 0;
    numBlocks = 0;
}
Table::Table(string table_name) {
    name_ = table_name;
    numRows = 0;
    numCols = 0;
    primary_key_values_loaded_ = false;
    primary_key_flags_loaded_ = false;
    key_flags_loaded_ = false;
    data_loaded_ = false;
     btrees_loaded_ = false;
}
Table::Table(string table_name, vector<string> column_names) {
    name_ = table_name;
    column_names_ = column_names;

    primary_key_values_loaded_ = false;
    primary_key_flags_loaded_ = false;
    key_flags_loaded_ = false;
    data_loaded_ = false;
     btrees_loaded_ = false;
}
Table::Table(string table_name, vector<string> column_names, vector<vector<string> > data) {
    name_ = table_name;
    column_names_ = column_names;
    data_ = data;
    primary_key_values_loaded_ = false;
    primary_key_flags_loaded_ = false;
    key_flags_loaded_ = false;
    data_loaded_ = false;
    btrees_loaded_ = false;
}
void Table::InsertPrimaryKeyValue(string value) {
    primary_key_values_.push_back(value);
}
bool Table::IsCurrentPrimaryKeyValue(string value) {
    bool found = false;
    int i = 0;
    while (i < primary_key_values_.size() && !found) {
        if(value == primary_key_values_[i])
            found = true;
        i++;
    }
    return found;
}
void Table::loadTableIntoMemory() {
  ifstream in;
  string file_path;
  string input;
  vector<string> values;

  file_path = "./data/" + this->GetTableName() + ".tbl";
  // open file for reading
  in.open(file_path.c_str());
  if (!in.is_open()) {
      cerr << "ERROR: Cannot open input file " << name_ << endl;
      in.close();
      return;
  }
  getline(in, input); // this is column names, don't need these
  // Clear current data before loading from file.
  data_.clear();
  // Read in values
  while (getline(in, input)) {
      stringstream ss;
      ss << input;
      values.clear();
      while (getline(ss,input,'\t')) {
          values.push_back(input);
      }
      this->InsertRow(values);
  }
  data_loaded_ = true;
  in.close();
}
string Table::GetTableName() {
    return name_;
}
vector<string> Table::GetColumnNames() {
    return column_names_;
}
bool Table::HasColumn(string column_name) {
    bool found = false;
    string lowercase_col_name = make_lower_case(column_name);
    int i = 0;
    while (i < column_names_.size() && found == false){
        if (lowercase_col_name == column_names_[i]) found = true;
        i++;
    }
    return found;
}
int Table::ColumnIndex(string column_name) {
    bool found = false;
    int i = 0;
    string lowercase_col_name = make_lower_case(column_name);
    while (i < column_names_.size() && found == false){
        if (lowercase_col_name == column_names_[i]) found = true;
        if (found == false)i++;
    }
    if (found == false) i = -99;
    return i;
}
void Table::SetTableName(string table_name) {
    name_ = table_name;
}
void Table::SetColumnNames(vector<string> column_names) {
    column_names_ = column_names;
    numCols = column_names.size();
}
void Table::initNumRows() {
    numRows = 0;
}
void Table::initNumUniqueValues() {
    numUniqueValuesOfIndexedColumns.clear();
}
void Table::setNumRows(int num) {
    numRows = num;
}
void Table::setNumCols(int num) {
    numCols = num;
}
void Table::setNumBlocks(int num) {
    numBlocks = num;
}
void Table::AddNumUniqueValues(int numUnique) {
    numUniqueValuesOfIndexedColumns.push_back(numUnique);
}
void Table::InsertRow(vector<string> row) {
    data_.push_back(row);
}
void Table::AddPrimaryKey(bool value) {
    primary_key_.push_back(value);
}
void Table::AddKey(bool value) {
    key_.push_back(value);
}
vector<vector<string> > Table::SearchTable(string column_name, string value, int opcode) {
    string lowercase_col_name = make_lower_case(column_name);
    vector<vector<string> > results;
    if (HasColumn(lowercase_col_name)) {
		
		bool value_is_string = false;
		
		if(value[0] == '\'') {
			value_is_string = true;
		}
		
		if(opcode == 0) {
			for (int i = 0; i < data_.size(); i++)
				if (data_[i][ColumnIndex(lowercase_col_name)] == value)
					results.push_back(data_[i]);
		} else if(opcode == 1) {
			
			if(value_is_string) {
				for (int i = 0; i < data_.size(); i++) {
					if(data_[i][ColumnIndex(lowercase_col_name)][0] == '\'') {
						if (data_[i][ColumnIndex(lowercase_col_name)] > value) {
							results.push_back(data_[i]);
						}
					}
				}
			} else {
				for (int i = 0; i < data_.size(); i++) {
					if (data_[i][ColumnIndex(lowercase_col_name)][0] != '\'') {
						if (stoi(data_[i][ColumnIndex(lowercase_col_name)]) > stoi(value)) {
							results.push_back(data_[i]);
						}
					}
				}
			}
		} else if(opcode == 2) {
			if(value_is_string) {
				for (int i = 0; i < data_.size(); i++) {
					if(data_[i][ColumnIndex(lowercase_col_name)][0] == '\'') {
						if (data_[i][ColumnIndex(lowercase_col_name)] < value) {
							results.push_back(data_[i]);
						}
					}
				}
			} else {
				for (int i = 0; i < data_.size(); i++) {
					if (data_[i][ColumnIndex(lowercase_col_name)][0] != '\'') {
						if (stoi(data_[i][ColumnIndex(lowercase_col_name)]) < stoi(value)) {
							results.push_back(data_[i]);
						}
					}
				}
			}
		} else if(opcode == 3) {
			for (int i = 0; i < data_.size(); i++)
				if (data_[i][ColumnIndex(lowercase_col_name)] != value)
					results.push_back(data_[i]);
		}
    }

    return results;
}
void Table::clearData() {
    data_.clear();
    data_loaded_ = false;
}

void Table::incrementNumRows() {
    (this->numRows)++;
}
void Table::incrementNumCols() {
    (this->numCols)++;
}
void Table::updateNumBlocks(off_t sizeInBytes) {
    int blocks = 1 + (sizeInBytes / 16384);
    setNumBlocks(blocks);
}
void Table::incrementNumUniqueValues(int index) {
    (this->numUniqueValuesOfIndexedColumns[index])++;
}

void Table::SetPrimaryKeys(vector<bool> primary_keys) {
    primary_key_ = primary_keys;
}
void Table::SetKeys(vector<bool> keys) {
    key_ = keys;

    for(int i = 0; i < keys.size(); i++) {
        if(keys[i]) {
            numUniqueValuesOfIndexedColumns.push_back(0);
        }
    }
}
void Table::AddColumnName(string column_name) {
    column_names_.push_back(column_name);
    incrementNumCols();
}
int Table::GetKeyVecSize() {
    return (key_.size());
}
int Table::GetPrimaryKeyVecSize() {
    return (primary_key_.size());
}
int Table::GetNumRows() {
    return numRows;
}
int Table::GetNumCols() {
    return numCols;
}
int Table::GetNumBlocks() {
    return numBlocks;
}
vector<int> Table::GetNumberOfUniqueValuesVector() {
    return numUniqueValuesOfIndexedColumns;
}
void Table::init() {
    name_.clear();
    column_names_.clear();
    data_.clear();
    primary_key_.clear();
    key_.clear();
    indexes.clear();
    primary_key_values_.clear();
}
// Reverses the column names
void Table::reverse_column_names() {
    vector<string> temp;

    for (int i = column_names_.size()-1; i >= 0; i--)
        temp.push_back(column_names_[i]);
    column_names_.clear();
    column_names_ = temp;
}
void Table::reverse_key_vectors() {
    vector<bool> temp;
    // Reverse key_ vector
    for (int i = key_.size()-1; i >= 0; i--)
        temp.push_back(key_[i]);
    key_.clear();
    key_ = temp;
    temp.clear();
    // Reverse primary_key_ vectoru
    for (int i = primary_key_.size()-1; i >= 0; i--)
        temp.push_back(primary_key_[i]);
    primary_key_.clear();
    primary_key_ = temp;
}
int Table::GetColumnNamesVecSize() {
    return (column_names_.size());
}
vector<bool> Table::GetPrimaryKeys() {
    return (primary_key_);
}
vector<bool> Table::GetKeys() {
    return(key_);
}
bool Table::IsPrimaryKey(int index) {
    if (primary_key_[index] == true)
        return true;
    else
        return false;
}
bool Table::IsKey(int index) {
    if (key_[index] == true)
        return true;
    else
        return false;
}
string Table::GetValue(int row, int col) {
    return data_[row][col];
}

string Table::GetColumnName(int index) {
    return column_names_[index];
}
bool Table::GetPrimaryKey(int index) {
    return primary_key_[index];
}
bool Table::GetKey(int index) {
    return key_[index];
}
string Table::LoadPrimaryKeyFlags() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/primary_keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("primary_keys directory does not exist");
    
    ifstream in;
    string file_path, input;
    file_path = "./data/primary_keys/" + name_ + ".pkey";
    // Clear primary key flags before loading from file
    primary_key_.clear();
    // open file for reading
    in.open(file_path.c_str());
    if (!in.is_open()) {
        in.close();
        return ("0"); // no file to read, may not be an error
    }
    while (in >> input)
        AddPrimaryKey(string_to_bool(input)); 
    in.close();
    primary_key_flags_loaded_ = true;
    return("0");
}
void Table::ClearPrimaryKeyFlags() {
    primary_key_.clear();
    primary_key_flags_loaded_ = false;
}
string Table::LoadKeyFlags() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("keys directory does not exist");
    
    ifstream in;
    string file_path, input;

    file_path = "./data/keys/" + name_ +".key";
    // Clear key flags before loading from file
    key_.clear();            
    // open file for reading
    in.open(file_path.c_str());
    if (!in.is_open()) {
        in.close();
        return ("0"); // There may not be a key file, so no error.
    }
    while (in >> input)
        AddKey(string_to_bool(input));
    in.close();
    key_flags_loaded_ = true;
    return("0");     
}
void Table::ClearKeyFlags() {
    key_.clear();
    key_flags_loaded_ = false;
}
string Table::LoadPrimayKeyValues() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/primary_keys", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("primary_keys directory does not exist");
    
    ifstream in;
    string file_path, input;

    file_path = "./data/primary_keys/" + name_ + ".pkv";
    // Clear primary key values before loading from file
    primary_key_values_.clear();
    // open file for reading
    in.open(file_path.c_str());
    if (!in.is_open()) {
        in.close();
        return ("0"); // File may not exist, may not be an error.
    }
    while (in >> input)
        InsertPrimaryKeyValue(input);
    in.close();
    primary_key_values_loaded_ = true;
    return("0");
}
void Table::ClearPrimaryKeyValues() {
    primary_key_values_.clear();
    primary_key_values_loaded_ = false;
}
bool Table::GetPrimaryKeyValuesLoaded() {
    return primary_key_values_loaded_;
}
void Table::SetPrimaryKeyValuesLoaded(bool new_value) {
    primary_key_values_loaded_ = new_value;
}
string Table::LoadBtrees() {
    struct stat sb;
    if (stat("./data", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("data directory does not exist");
    else if (stat("./data/indexes", &sb) != 0 || !S_ISDIR(sb.st_mode))
        return ("indexes directory does not exist");
    
    ifstream in;
    string file_path, input1, input2;
    stx::btree<string, int> new_tree;

    file_path = "./data/indexes/" + name_ + ".idx"; 
    
    // open file for reading
    in.open(file_path.c_str());
    if (!in.is_open()) {
        in.close();
        return ("0");
    }
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
        indexes.push_back(new_tree);             
    }
    in.close();
    btrees_loaded_ = true;     
    return("0");
}
void Table::ClearBtrees() {
    indexes.clear();
    btrees_loaded_ = false;
}
void Table::SetBtreesLoaded(bool new_value) {
    btrees_loaded_ = new_value;
}
bool Table::GetBtreesLoaded() {
    return btrees_loaded_;
}
void Table::SetDataLoaded(bool new_value) {
    data_loaded_ = new_value;
}
bool Table::GetDataLoaded() {
    return data_loaded_;
}
void Table::SetPrimaryKeyFlagsLoaded(bool new_value) {
    primary_key_flags_loaded_ = new_value;
}
bool Table::GetPrimaryKeyFlagsLoaded() {
    return primary_key_flags_loaded_;
}
void Table::SetKeyFlagsLoaded(bool new_value) {
    key_flags_loaded_ = new_value;
}
bool Table::GetKeyFlagsLoaded() {
    return key_flags_loaded_;
}
int Table::GetColumnIndex(string name) {
    bool found = false;
    int i = 0;
    while (i < column_names_.size() && !found) {
        if (name == column_names_[i])
            found = true;
        if (!found)
            i++;
    }
    return i;
}

vector<vector<string> > Table::GetData() {
    return data_;
}

vector<string> Table::GetRow(int rowNum) {
	return data_[rowNum];
}
int Table::GetNumRowsOfData() {
    return (data_.size());
}
/*
// Not completley tested
string Table::LoadAllDataIntoMemory() {
    loadTableIntoMemory();
    LoadPrimaryKeyFlags();
    LoadKeyFlags();
    LoadPrimayKeyValues();
    LoadBtrees();
}
// Not completley tested
string Table::ClearAllDataFromMemory() {
    ClearPrimaryKeyFlags();
    ClearKeyFlags();
    clearData();
    ClearPrimaryKeyValues();
    //ClearBtrees();
}
*/
