#pragma once
#include "first_follow.h"
using namespace std;

// Global LL(1) parsing table
extern map<string, map<string, Production>> table;

// Helpers
set<string> getTerminals(Grammar &g);
set<string> firstProd(Production p);

// Table construction (resets global table before building)
bool buildTable(Grammar &g);

// Output
void printTable(Grammar &g, ostream &out = cout);
void saveTableToCSV(Grammar &g, const string &filename = "output/parsing_table.csv");
