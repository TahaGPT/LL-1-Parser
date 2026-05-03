#pragma once
#include <string>
#include <vector>
#include <iostream>
using namespace std;
 
// simple stack wrapper around vector; bottom = index 0, top = back
struct Stack {
    vector<string> data;
 
    void   push(const string &s) { data.push_back(s); }
    void   pop()                 { data.pop_back(); }
    string top() const           { return data.back(); }
    bool   empty() const         { return data.empty(); }
 
    // print bottom->top as a readable string
    string str() const;
};