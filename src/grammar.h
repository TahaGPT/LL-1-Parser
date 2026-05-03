#pragma once
#include <bits/stdc++.h>
using namespace std;
 
using Production = vector<string>;
using Grammar    = map<string, vector<Production>>;
 
// Utility
string trim(const string &s);
bool   isTerminal(const string &s);
 
// CFG Input
Production tokenize(const string &prod);
Grammar    readCFG(const string &filename);
 
// Print to stream (cout or file)
void printGrammar(const Grammar &g, ostream &out = cout);
 
// Left Factoring
vector<string> LCP(const vector<Production> &prods);
void leftFactor(Grammar &g);
 
// Left Recursion Removal
void removeDirect(string A, Grammar &g);
void removeLeftRecursion(Grammar &g, const string &startSymbol);