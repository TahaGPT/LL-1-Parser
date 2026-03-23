#pragma once
#include "grammar.h"
using namespace std;

// Global FIRST and FOLLOW sets
extern map<string, set<string>> FIRST;
extern map<string, set<string>> FOLLOW;

// FIRST
set<string> firstOf(string X, Grammar &g, map<string, bool> &vis);
void computeFIRST(Grammar &g);

// FOLLOW
set<string> firstSeq(vector<string> seq);
void computeFOLLOW(Grammar &g, string start);

// Print to stream (cout or file)
void printFIRST(ostream &out = cout);
void printFOLLOW(ostream &out = cout);
