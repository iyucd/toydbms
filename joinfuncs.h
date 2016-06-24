#ifndef JOINFUNCS_H
#define JOINFUNCS_H

#include "Table.h"
#include "globals.h"
#include "stdlib.h"

vector<string> findCommonColumns(vector<string> cols1, vector<string> cols2);

string indexJoin();
string hashJoin();
string nestedLoopJoin();
string crossProduct();

void checkForJoin();
bool multipleTablesInFrom();
bool isJoinOperation();
bool joinColumnsAreIndexed();
bool tableContains1000Rows();

#endif
