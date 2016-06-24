#include "joinfuncs.h"
#include <iostream>
#include <map>

extern int table_index(string table_name);
extern newSFW new_sfw;
extern vector<Table> tables;
extern bool greaterThanComparison(string lhs, string rhs);
extern bool lessThanComparison(string lhs, string rhs);
extern bool comparisonContainsTwoColumns(string lhs, string rhs);

struct condition_info {
	bool containsTwoCols;
	bool lhs_in_r;
	bool rhs_in_r;
	string compare_operator;
	string rhs_value;
	int lhs_index;
	int rhs_index;
};

vector<int> getTablesPositions();
bool isJoinCondition(string lhs, string rhs, string where_operator);
vector<vector<string> > executeRemainingConditions(vector<vector<string> > result_table
		, Table R, Table S);
void printResultTable(vector<vector<string> > result_table, vector<string> rcols, vector<string> scols, vector<int> indexes);

string indexJoin() {
	std::cout << "Index Join" << std::endl;
	// File stream for reading a tables data
	fstream in;
	// Column positions
    int R_col_pos;
    int L_col_pos;
	// Load table R (right) into memory, the second table listed in FROM. First table will be called L (left).
	Table R = tables[table_index(new_sfw.table_name[1])];
	// Load it's data
	R.loadTableIntoMemory();

	// Results stored here
    vector<vector<string> > results;
	
	// doneWithCondition used so we know if we have handled the condition at position i within 'where_cond_LHS'
    vector<bool> doneWithCondition;
    // initialize the doneWithCondition vector
    for (int i = 0; i < new_sfw.where_cond_LHS.size(); i++)
        doneWithCondition.push_back(false);
    
    // Find first condition that is a join condition
    bool found = false;
    int k = 0;
    while (k < new_sfw.where_cond_LHS.size() && !found) {
        if (isJoinCondition(new_sfw.where_cond_LHS[k][0], new_sfw.where_cond_RHS[k][0], new_sfw.where_compare_op[k]))
            found = true;
        else
            k++;
    }
    if (found == true) {
    	// mark condition which will be applied first as done
    	doneWithCondition[k] = true;

    	// Iterate through rows of second table R, in memory, and compare with first table using btree indexes.
	    // Store results that meet first checked condition in results
	    vector<string> R_row;
	    stx::btree<string,int>::iterator iter;
   	 	int offset;
    	vector<vector<string> > Ls_join_rows;
    	stringstream ss;
    	// open file for reading indexed table
		string file_path = "./data/" + new_sfw.table_name[0] + ".tbl";
		in.open(file_path.c_str());
		if (!in.is_open())
		    return ("Cannot open file for reading during index join.");

	    for (int i = 0; i < R.GetNumRowsOfData(); i++) {
	    	R_row.clear();
	    	R_row = R.GetRow(i);
	    	
	    	// Get column position of R's column for in memory access
	    	R_col_pos = R.GetColumnIndex(new_sfw.where_cond_RHS[k][0]);
			// Get column position of the second tables (L) compare column. Used for btree access and results printing
			L_col_pos = tables[table_index(new_sfw.table_name[0])].GetColumnIndex(new_sfw.where_cond_LHS[k][0]);
		    
		    // Get R's compare value
		    string R_compare_value = R_row[R_col_pos];

		    // Find columns in table L which succesfully join with R
		    iter = tables[table_index(new_sfw.table_name[0])].indexes[L_col_pos].find(R_compare_value);
		    // if result found
	        if (iter !=  tables[table_index(new_sfw.table_name[0])].indexes[L_col_pos].end()) {
	        	int a = 0;
	        	Ls_join_rows.clear();
	            for (;iter != tables[table_index(new_sfw.table_name[0])].indexes[L_col_pos].upper_bound(R_compare_value); iter++, a++) {
	                offset = iter.data();
	                // Move to offset
	                in.seekg(offset);
	                // get the row at the offset
	                string input;
	                getline(in,input);
	                // Tokenize and store L's join rows
	                ss.clear();
	                ss.str(string());
	                ss << input;
	                vector<string> temp;
	                Ls_join_rows.push_back(temp);
	                while(getline(ss,input,'\t'))
	                	Ls_join_rows[a].push_back(input);
	            }
	            // Now join the rows from L that join with the row from R
	            if (Ls_join_rows.size() > 0) {
	            	vector<string> temp_results; 
	            	for (int b = 0; b < Ls_join_rows.size(); b++) {
	            		temp_results.clear();
	            		temp_results = R_row;
	            		 for (int c = 0; c < Ls_join_rows[b].size(); c++) {
	            		 	temp_results.push_back(Ls_join_rows[b][c]);
	            		 }
	            		 results.push_back(temp_results);
	            	}
	            }
	        }
    	}
    }
    else {
    	cerr << "ERROR: No condition in the WHERE clasue meets the join conditions." << endl;
    	return("0");
    }
    new_sfw.where_cond_LHS.erase(new_sfw.where_cond_LHS.begin() + k);
	new_sfw.where_cond_RHS.erase(new_sfw.where_cond_RHS.begin() + k);
	new_sfw.where_compare_op.erase(new_sfw.where_compare_op.begin() + k);

	// Load table S
	Table L = tables[table_index(new_sfw.table_name[0])];
	// Load it's data
	L.loadTableIntoMemory();

	results = executeRemainingConditions(results, R, L);
	
	//find indexes of columns to print
	vector<int> indexes_of_cols_to_print;
	for(int i = 0; i < new_sfw.proj_as_column_names.size(); i++) { 
		string col_name = new_sfw.proj_as_column_names[i];
		if(R.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetColumnIndex(col_name));
		} else if(L.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetNumCols() + L.GetColumnIndex(col_name));
		} else {
			return "Error: " + col_name + " is not in specified tables.";
		}
	}
	
	vector<string> rcols = R.GetColumnNames();
	vector<string> lcols = L.GetColumnNames();

	//print resulting table
	printResultTable(results, rcols, lcols, indexes_of_cols_to_print);
    in.close();
	return "0";
}

string hashJoin() {
	
	std::cout << "Hash Join" << std::endl;
	//create vector of indexes for tables vector
	vector<int> table_indexes = getTablesPositions();
	
	//assign Table pointers R and S
	Table R = tables[table_indexes[0]];
	Table S = tables[table_indexes[1]];

	//load data from tables in FROM clause into memory
	R.loadTableIntoMemory();
	S.loadTableIntoMemory();

	/*
	 * Execute first join condition
	 * Hash both tables to separate maps.
	 * Iterate through and compare buckets.
	 * Store In: result_table
	 */
	vector<vector<string> > result_table;
	vector<vector<string> > rData = R.GetData();
	vector<vector<string> > sData = S.GetData();

	bool first_condition_finished = false;
	for(int i = 0; i < new_sfw.where_cond_LHS.size(); i++) {

		if(!first_condition_finished) {
			// Get column positions
			bool isJoinCond = isJoinCondition(new_sfw.where_cond_LHS[i][0], new_sfw.where_cond_RHS[i][0], new_sfw.where_compare_op[i]);

			if(isJoinCond) {
				bool lhs_in_r = R.HasColumn(new_sfw.where_cond_LHS[i][0]);
				
				int lhs_col_pos;
				int rhs_col_pos;
				if(lhs_in_r) {
					lhs_col_pos = R.GetColumnIndex(new_sfw.where_cond_LHS[i][0]);
					rhs_col_pos = S.GetColumnIndex(new_sfw.where_cond_RHS[i][0]);
				} else {
					lhs_col_pos = S.GetColumnIndex(new_sfw.where_cond_LHS[i][0]);
					rhs_col_pos = R.GetColumnIndex(new_sfw.where_cond_RHS[i][0]);
				}

				// Hash R and S
				map<string, vector<string> > rmap;
				map<string, vector<string> > smap;

				if(lhs_in_r) {
					for(int j = 0; j < rData.size(); j++) {
						rmap[rData[j][lhs_col_pos]] = rData[j];
					}

					for(int j = 0; j < sData.size(); j++) {
						smap[sData[j][rhs_col_pos]] = sData[j];
					}
				} else {
					for(int j = 0; j < rData.size(); j++) {
						rmap[rData[j][rhs_col_pos]] = rData[j];
					}

					for(int j = 0; j < sData.size(); j++) {
						smap[sData[j][lhs_col_pos]] = sData[j];
					}
				}

				// Perform Join
				vector<string> r_row;
				vector<string> s_row;
				bool condition_met;

				for(int j = 0; j < rData.size(); j++) {
					for(int k = 0; k < sData.size(); k++) {
						condition_met = false;

						r_row = rData[j];
						s_row = sData[k];
						vector<string> result_row;
						if(lhs_in_r) {
							if(r_row[lhs_col_pos] == s_row[rhs_col_pos]) {
								condition_met = true;
								result_row = rmap.at(r_row[lhs_col_pos]);
								vector<string> temp = smap.at(s_row[rhs_col_pos]);

								for(int l = 0; l < temp.size(); l++) {
									result_row.push_back(temp[l]);
								}
							}
						} else {
							if(s_row[lhs_col_pos] == r_row[rhs_col_pos]) {
								condition_met = true;
								result_row = rmap.at(r_row[rhs_col_pos]);
								vector<string> temp = smap.at(s_row[lhs_col_pos]);

								for(int l = 0; l < temp.size(); l++) {
									result_row.push_back(temp[l]);
								}
							}
						}

						if(condition_met) {
							//add joined row to result_table
							result_table.push_back(result_row);
						}
					}
				}

				new_sfw.where_cond_LHS.erase(new_sfw.where_cond_LHS.begin() + i);
				new_sfw.where_cond_RHS.erase(new_sfw.where_cond_RHS.begin() + i);
				new_sfw.where_compare_op.erase(new_sfw.where_compare_op.begin() + i);
				first_condition_finished = true;
			}
		}
	}

	result_table = executeRemainingConditions(result_table, R, S);
	
	//find indexes of columns to print
	vector<int> indexes_of_cols_to_print;
	for(int i = 0; i < new_sfw.proj_as_column_names.size(); i++) { 
		string col_name = new_sfw.proj_as_column_names[i];
		if(R.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetColumnIndex(col_name));
		} else if(S.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetNumCols() + S.GetColumnIndex(col_name));
		} else {
			return "Error: " + col_name + " is not in specified tables.";
		}
	}
	
	vector<string> rcols = R.GetColumnNames();
	vector<string> scols = S.GetColumnNames();

	//print resulting table
	printResultTable(result_table, rcols, scols, indexes_of_cols_to_print);
	return "0";
}

string nestedLoopJoin() {
	
	std::cout << "Nested Loop Join" << std::endl;
	//create vector of indexes for tables vector
	vector<int> table_indexes = getTablesPositions();
	
	//assign Table pointers R and S
	Table R = tables[table_indexes[0]];
	Table S = tables[table_indexes[1]];

	//load data from tables in FROM clause into memory
	R.loadTableIntoMemory();
	S.loadTableIntoMemory();

	/*
	 * Execute first join condition
	 * Outer Loop: tuples of R
	 * Inner Loop: tuples of S
	 * Store In: result_table
	 */
	vector<vector<string> > result_table;
	vector<vector<string> > rData = R.GetData();
	vector<vector<string> > sData = S.GetData();
	
	bool first_condition_finished = false;
	for(int i = 0; i < new_sfw.where_cond_LHS.size(); i++) {

		if(!first_condition_finished) {
			// Get column positions
			bool isJoinCond = isJoinCondition(new_sfw.where_cond_LHS[i][0], new_sfw.where_cond_RHS[i][0], new_sfw.where_compare_op[i]);

			if(isJoinCond) {
				bool lhs_in_r = R.HasColumn(new_sfw.where_cond_LHS[i][0]);
				
				int lhs_col_pos;
				int rhs_col_pos;
				if(lhs_in_r) {
					lhs_col_pos = R.GetColumnIndex(new_sfw.where_cond_LHS[i][0]);
					rhs_col_pos = S.GetColumnIndex(new_sfw.where_cond_RHS[i][0]);
				} else {
					lhs_col_pos = S.GetColumnIndex(new_sfw.where_cond_LHS[i][0]);
					rhs_col_pos = R.GetColumnIndex(new_sfw.where_cond_RHS[i][0]);
				}
				
				// Perform Join
				vector<string> r_row;
				vector<string> s_row;
				bool condition_met;

				for(int j = 0; j < rData.size(); j++) {
					for(int k = 0; k < sData.size(); k++) {
						condition_met = false;
						r_row = rData[j];
						s_row = sData[k];

						if(lhs_in_r) {
							if(r_row[lhs_col_pos] == s_row[rhs_col_pos]) {
								condition_met = true;
							}
						} else {
							if(s_row[lhs_col_pos] == r_row[rhs_col_pos]) {
								condition_met = true;
							}
						}

						if(condition_met) {
							//create joined row and add to result_table
							vector<string> result_row = r_row;
							for(int l = 0; l < s_row.size(); l++) {
								result_row.push_back(s_row[l]);
							}
							result_table.push_back(result_row);
						}
					}
				}

				new_sfw.where_cond_LHS.erase(new_sfw.where_cond_LHS.begin() + i);
				new_sfw.where_cond_RHS.erase(new_sfw.where_cond_RHS.begin() + i);
				new_sfw.where_compare_op.erase(new_sfw.where_compare_op.begin() + i);
				first_condition_finished = true;
			}
		}
	}
	
	result_table = executeRemainingConditions(result_table, R, S);
	
	//find indexes of columns to print
	vector<int> indexes_of_cols_to_print;
	for(int i = 0; i < new_sfw.proj_as_column_names.size(); i++) { 
		string col_name = new_sfw.proj_as_column_names[i];
		if(R.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetColumnIndex(col_name));
		} else if(S.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetNumCols() + S.GetColumnIndex(col_name));
		} else {
			return "Error: " + col_name + " is not in specified tables.";
		}
	}
	
	vector<string> rcols = R.GetColumnNames();
	vector<string> scols = S.GetColumnNames();

	//print resulting table
	printResultTable(result_table, rcols, scols, indexes_of_cols_to_print);
	return "0";
}

/*
 * Returns cross product of two tables
 */
string crossProduct() {
	
	std::cout << "Cross Product" << std::endl;
	//create vector of indexes for tables vector
	vector<int> table_indexes = getTablesPositions();
	
	//assign Table pointers R and S
	Table R = tables[table_indexes[0]];
	Table S = tables[table_indexes[1]];

	//load data from tables in FROM clause into memory
	R.loadTableIntoMemory();
	S.loadTableIntoMemory();
	
	vector<vector<string> > result_table;
	vector<vector<string> > rData = R.GetData();
	vector<vector<string> > sData = S.GetData();
	vector<string> result_row;
	
	for(int i = 0; i < rData.size(); i++) {
		for(int j = 0; j < sData.size(); j++) {
			result_row = rData[i];
			
			for(int k = 0; k < S.GetNumCols(); k++) {
				result_row.push_back(sData[j][k]);
			}
			
			result_table.push_back(result_row);
		}
	}

	//find indexes of columns to print
	vector<int> indexes_of_cols_to_print;
	for(int i = 0; i < new_sfw.proj_as_column_names.size(); i++) { 
		string col_name = new_sfw.proj_as_column_names[i];
		if(R.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetColumnIndex(col_name));
		} else if(S.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetNumCols() + S.GetColumnIndex(col_name));
		} else {
			return "Error: " + col_name + " is not in specified tables.";
		}
	}
	vector<string> rcols = R.GetColumnNames();
	vector<string> scols = S.GetColumnNames();

	printResultTable(result_table, rcols, scols, indexes_of_cols_to_print);
	
	return "0";
}

/*
 * Print table resulting from join
 */
void printResultTable(vector<vector<string> > result_table, vector<string> rcols, vector<string> scols, vector<int> indexes) {
	//print column names
	for(int i = 0; i < indexes.size(); i++) {
		if(indexes[i] < rcols.size()) {
			cout << rcols[indexes[i]] << "\t";
		} else {
			cout << scols[indexes[i] - rcols.size()] << "\t";
		}
	}
	
	cout << "\n";
	for (int i = 0; i < result_table[0].size(); i++)
		cout << "=========";
	cout << endl;

	//print data
	for(int i = 0; i < result_table.size(); i++) {
		for(int j = 0; j < indexes.size(); j++) {
			cout << result_table[i][indexes[j]] << "\t"; 
		}
		cout << "\n";
	}
	cout << endl;
}

/*
 * Eliminates tuples that do not meet conditions remaining in WHERE clause after a join
 */
vector<vector<string> > executeRemainingConditions(vector<vector<string> > result_table 
		, Table R, Table S) {
	/*
	struct condition_info {
		bool containsTwoCols;
		bool lhs_in_r;
		bool rhs_in_r;
		string compare_operator;
		string rhs_value;
		int lhs_index;
		int rhs_index;
	};
	 */
	//Fill condition_info struct array of indexes for remaining columns
	condition_info conditions[new_sfw.where_cond_LHS.size()];
	for(int i = 0; i < new_sfw.where_cond_LHS.size(); i++) {
		conditions[i].containsTwoCols = comparisonContainsTwoColumns(new_sfw.where_cond_LHS[i][0], new_sfw.where_cond_RHS[i][0]);
		conditions[i].lhs_in_r = R.HasColumn(new_sfw.where_cond_LHS[i][0]);
		if(conditions[i].lhs_in_r) {
			conditions[i].lhs_index = R.GetColumnIndex(new_sfw.where_cond_LHS[i][0]);
		} else {
			conditions[i].lhs_index = S.GetColumnIndex(new_sfw.where_cond_LHS[i][0]);
		}
		
		if(conditions[i].containsTwoCols) {
			conditions[i].rhs_in_r = R.HasColumn(new_sfw.where_cond_RHS[i][0]);
			if(conditions[i].rhs_in_r) {
				conditions[i].rhs_index = R.GetColumnIndex(new_sfw.where_cond_RHS[i][0]);
			} else {
				conditions[i].rhs_index = S.GetColumnIndex(new_sfw.where_cond_RHS[i][0]);
			}
		} else {
			conditions[i].rhs_value = new_sfw.where_cond_RHS[i][0];
		}
		
		conditions[i].compare_operator = new_sfw.where_compare_op[i];
	}
	int num_cols_in_r = R.GetNumCols();
	 
	for(int i = 0; i < new_sfw.where_cond_LHS.size(); i++) {
				
		//for each row of result_table
		for(int j = 0; j < result_table.size(); j++) {
		
			string lhs_value;
			string rhs_value;
			// if the lhs of comparison is in R
			if(conditions[i].lhs_in_r) {
				lhs_value = result_table[j][conditions[i].lhs_index];						
			} else {
				lhs_value = result_table[j][num_cols_in_r + conditions[i].lhs_index];
			}

			//if condition i contains 2 sql identifiers
			if(conditions[i].containsTwoCols) {
				// if rhs of comparison is in R
				if(conditions[i].rhs_in_r) {
					rhs_value = result_table[j][conditions[i].rhs_index];
				} else {
					rhs_value = result_table[j][num_cols_in_r + conditions[i].rhs_index];
				}
			} else {
				rhs_value = conditions[i].rhs_value;
			}
			
			// if the comparison uses "="
			if(conditions[i].compare_operator == "=") {
				if(!(lhs_value == rhs_value)) {
					result_table.erase(result_table.begin() + j);
					j--;
				}
			// if the comparison uses ">"	
			} else if(conditions[i].compare_operator == ">") {
				
				if(!(greaterThanComparison(lhs_value, rhs_value))) {
					result_table.erase(result_table.begin() + j);
					j--;
				}
			// if the comparison uses "<"
			} else if(conditions[i].compare_operator == "<") {
				if(!(lessThanComparison(lhs_value, rhs_value))) {
					result_table.erase(result_table.begin() + j);
					j--;
				}
			// if the comparison uses "<>"
			} else if(conditions[i].compare_operator == "<>") {
				if(!(lhs_value != rhs_value)) {
					result_table.erase(result_table.begin() + j);
					j--;
				}
			// otherwise comparison operator is invalid				
			} else {
				cerr << "Invalid operator in WHERE condition" << endl;
				exit(1);
			}			
		}
	}
	
	return result_table;
}

/*
 * Returns vector of column names contained in both cols1 and cols2
 */
vector<string> findCommonColumns(vector<string> cols1, vector<string> cols2) {
	
	vector<string> commonCols;
	
	for(int i = 0; i < cols1.size(); i++) {
		for(int j = 0; j < cols2.size(); j++) {
			if(cols1[i] == cols2[j]) {
				commonCols.push_back(cols1[i]);
			}
		}
	}
	return commonCols;
}

/*
 * Sets global flags in new_sfw for join operations.
 */
void checkForJoin() {
	if(multipleTablesInFrom()) {
		
		new_sfw.multipleTables = true;
		
		if(isJoinOperation()) {
			if(joinColumnsAreIndexed()) {
				new_sfw.isIndexJoin = true;
			} else if(tableContains1000Rows()) {
				new_sfw.isHashJoin = true;
			} else {
				new_sfw.isNestedLoopJoin = true;
			} 
		}
	}
}

/*
 * Returns true if FROM clause contains more than one table
 */
bool multipleTablesInFrom() {
	return (new_sfw.table_name.size() > 1);
}

/*
 * returns true if condition in WHERE clause is of the form "table1.a = table2.b"
 */
bool isJoinOperation() {
	bool containsJoinCondition = false;
	
	for(int i = 0; i < new_sfw.where_cond_LHS.size(); i++) {
		
		string lhs = new_sfw.where_cond_LHS[i][0];
		string rhs = new_sfw.where_cond_RHS[i][0];
		string where_operator = new_sfw.where_compare_op[i];

		if(isJoinCondition(lhs, rhs, where_operator)) {
			containsJoinCondition = true;
		}
	}
	
	return containsJoinCondition;
}

/*
 * Returns true if RHS and LHS are valid SQL identifiers and are contained in different tables
 */
bool isJoinCondition(string lhs, string rhs, string where_operator) {
	bool isJoinCond = false;

	//check that operator is "="
	if(where_operator == "=") {
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
			vector<int> table_indexes = getTablesPositions();
			Table R = tables[table_indexes[0]];
			Table S = tables[table_indexes[1]];

			if(R.HasColumn(lhs) != R.HasColumn(rhs)) {
				isJoinCond = true;
			}
		}
	}

	return isJoinCond;
}


/*
 * Returns true if left hand side of conditions in a join condition is indexed and first table in FROM is the table using the index
 */
bool joinColumnsAreIndexed() {
	
	bool columnsIndexed = false;
	vector<int> table_indexes;
	table_indexes.push_back(table_index(new_sfw.table_name[0])); // Changed so only first table indexed is accepted
	
	for(int j = 0; j < new_sfw.where_cond_LHS.size(); j++) {
		
		//bool lhs_found = false;
		//bool rhs_found = false;
		bool column_index_found = false;
		
		//string index_column_name = "";
		
		for(int k = 0; k < table_indexes.size(); k++) {
			
			vector<string> column_names = tables[table_indexes[k]].GetColumnNames();
			vector<bool> key_vector = tables[table_indexes[k]].GetKeys();
			
			for(int l = 0; l < column_names.size(); l++) {
				if(new_sfw.where_cond_LHS[j][0] == column_names[l] && key_vector[l]) {
					column_index_found = true;
					
					if(!(isalpha(new_sfw.where_cond_RHS[j][0][0]) 
							|| new_sfw.where_cond_RHS[j][0][0] == '_' 
							|| new_sfw.where_cond_RHS[j][0][0] == '@'
							|| new_sfw.where_cond_RHS[j][0][0] == '$')) {
						
						column_index_found = false;
					}
				}	
			}
			
			if(column_index_found) {
				columnsIndexed = true;
			}

			column_index_found = false;
		}
		
		return columnsIndexed;
	}
}

/*
 * Returns true if one or both tables contain 1000 rows or more
 */
bool tableContains1000Rows() {
	
	bool contains1000Rows = false;
	
	for(int i = 0; i < new_sfw.table_name.size(); i++) {
		int idx = table_index(new_sfw.table_name[i]);
		
		if(tables[idx].GetNumRows() > 999) {
			contains1000Rows = true;
		}
	}
	
	return contains1000Rows;
}

/*
 * Returns a vector of current tables' positions in tables vector  
 */
vector<int> getTablesPositions() {
	vector<int> table_positions;
	for(int i = 0; i < new_sfw.table_name.size(); i++) {
		table_positions.push_back(table_index(new_sfw.table_name[i]));
	}

	return table_positions;
}



