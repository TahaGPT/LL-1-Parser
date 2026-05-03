#include "stack.h"
 
string Stack::str() const {
    string s;
    for (auto &x : data) s += x + " ";
    if (!s.empty()) s.pop_back();
    return s;
}