#pragma once
#include "first_follow.h"
#include "tree.h"
using namespace std;
 
// global ll(1) parsing table
extern map<string, map<string, vector<Production>>> table;
 
// helpers
set<string> getterminals(Grammar &g);
set<string> firstprod(Production p);
 
// table construction (resets global table before building)
bool buildtable(Grammar &g);
 
// output
void printtable(Grammar &g, ostream &out = cout);
void savetabletocsv(Grammar &g, const string &filename = "output/parsing_table.csv");
 
// ---- part 2: stack-based parsing ----
 
// result of parsing one input string
struct parseresult {
    bool    accepted;   // true if string accepted
    int     errors;     // number of errors encountered
    NodePtr tree;       // parse tree root (null if rejected)
};
 
// parse one token string; writes trace to 'out'
parseresult parsestring(const string &start, vector<string> tokens, ostream &out);
 
// read token lines from file; returns vector of token vectors
vector<vector<string>> readinputfile(const string &filename);
 
// run all strings from inputfile, write traces + trees to outputfile
void runparser(const string &start,
               const string &inputfile, const string &outputfile);
 