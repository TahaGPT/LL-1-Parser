#pragma once
#include <string>
#include <ostream>
using namespace std;

// records one parse error
struct parseerror {
    int    step;
    string msg;
};

// prints formatted error message; returns recovery action string
string handleerror(int step, const string &stacktop, const string &input,
                   ostream &out);