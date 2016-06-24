#include <iostream>
#include <map>
#include <ctype.h>
#include "globals.h"
#include "Table.h"

extern vector<int> getTablesPositions();
extern int table_index(string table_name);
extern newSFW new_sfw;
extern vector<Table> tables;
extern void init_new_sfw();

multimap<string, vector<string> > groupingMap;
void hashTuples(Table R);
void printGroups(Table R);
void averageGroups(Table R);
void countGroups(Table R);

string groupBy() {
	// get index of table in group by statement
	vector<int> indexes = getTablesPositions();

	// create Table object to represent the table in group by statement
	Table R = tables[indexes[0]];
	// hash all tuples into groupingMap using the column named in group by statement as the key.
	//hashTuples(R);

	if(!R.GetDataLoaded()) {
		R.loadTableIntoMemory();
	}
	vector<vector<string> > rData = R.GetData();

	//find position in given table of the column in the group by statement
	int group_by_col_pos = R.GetColumnIndex(new_sfw.groupByColName);

	// hash tuples of table using given column as key
	for(int i = 0; i < rData.size(); i++) {
		string mapKey = rData[i][group_by_col_pos];
		groupingMap.insert(pair<string, vector<string> >(mapKey, rData[i]));
	}

	if(new_sfw.callsAggregateFunction) {
		if(new_sfw.aggregateFunctionName == "avg") {
			averageGroups(R);
		} else if(new_sfw.aggregateFunctionName == "count") {
			countGroups(R);
		} else {
			return "Illegal aggregate function in GROUP BY statement.";
		}
	} else { //no aggregate function is called
		printGroups(R);
	}

	init_new_sfw();
	R.clearData();
	groupingMap.clear();
	return "0";
}

/*
 * Finds average of values in column specified by AVG function for each grouping
 * (Ignores string values)
 */
void averageGroups(Table R) {
	vector<double> avgVector;
	vector<string> keyVector;
	bool firstPass = true;
	int sum = 0;
	int count = 0;
	int index_of_func_operand;
	string currentKey = "";
	string previousKey = "";
	
	// find index of column in aggregate function
	if(!(new_sfw.proj_expressions.empty()) && R.HasColumn(new_sfw.proj_expressions[0][0])) {
		index_of_func_operand = R.GetColumnIndex(new_sfw.proj_expressions[0][0]);
	} else {
		cerr << "Invalid input to aggregate function" << endl;
		exit(1);
	}
	
	// execute function on each grouping
	for(map<string, vector<string> >::iterator it = groupingMap.begin(); it != groupingMap.end(); ++it) {
		currentKey = it->first;
		// if you are in the same bucket, or on the first pass
		if(currentKey == previousKey || firstPass) {
			if(isdigit((it->second)[index_of_func_operand][0])) {
				sum += stoi((it->second)[index_of_func_operand]);
				count += 1;
			}
		} else {
			if(count > 0) {
				avgVector.push_back((double)sum / count);
				keyVector.push_back(previousKey);
			}
			
			if(isdigit((it->second)[index_of_func_operand][0])) {
				sum = stoi((it->second)[index_of_func_operand]);
				count = 1;
			} else {
				sum = 0;
				count = 0;
			}
		} 

		previousKey = currentKey;
		firstPass = false;
	}
	
	// print output
	cout << "Average(" << new_sfw.proj_expressions[0][0] << ")\t" << new_sfw.groupByColName << "\n";

	int numLinesPrinted = 0;
	for(int i = 0; i < avgVector.size(); i++) {
		cout << avgVector[i] << "\t" << keyVector[i] << "\n";
		numLinesPrinted++;
	}

	cout << numLinesPrinted << " lines printed." << endl;
}

void countGroups(Table R) {
	vector<int> countVector;
	vector<string> keyVector;
	int count = 0;
	string previousKey = "";
	
	for(map<string, vector<string> >::iterator it = groupingMap.begin(); it != groupingMap.end(); ++it) {
		if(it->first == previousKey) {
			count++;
		} else {
			if(count > 0) {
				countVector.push_back(count);
				keyVector.push_back(previousKey);
			}
			count = 1;
			previousKey = it->first;
		}
	}

	cout << "Count\t" << new_sfw.groupByColName << "\n";
	for(int i = 0; i < countVector.size(); i++) {
		cout << countVector[i] << "\t" << keyVector[i] << "\n";
	}

	cout << R.GetNumRows() << " lines printed" << endl;
}

void printGroups(Table R) {
	//find indexes of columns to print
	vector<int> indexes_of_cols_to_print;

	for(int i = 0; i < new_sfw.proj_as_column_names.size(); i++) { 
		string col_name = new_sfw.proj_as_column_names[i];
		if(R.HasColumn(col_name)) {
			indexes_of_cols_to_print.push_back(R.GetColumnIndex(col_name));
		}
	}

	// print desired column names
	vector<string> rcols = R.GetColumnNames();
	for(int i = 0; i < indexes_of_cols_to_print.size(); i++) {
		cout << rcols[indexes_of_cols_to_print[i]] << "\t";
	}
	cout << "\n";

	for(map<string, vector<string> >::iterator it = groupingMap.begin(); it != groupingMap.end(); ++it) {
		for(int j = 0; j < indexes_of_cols_to_print.size(); j++) {
			cout << it->second[indexes_of_cols_to_print[j]] << "\t";
		}
		cout << "\n";
	}

	cout << R.GetNumRows() << " lines printed." << endl;
}


