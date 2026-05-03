#include "error_handler.h"
#include <iostream>

// formats error message, prints it, returns a recovery label
string handleerror(int step, const string &stacktop, const string &input, ostream &out) {
    string msg;

    if (input == "$")
        msg = "Premature end of input; expected '" + stacktop + "'";
    else if (stacktop == "$")
        msg = "Extra input '" + input + "' after end of accepted string";
    else
        msg = "Expected '" + stacktop + "', found '" + input + "'";

    out << "  [ERROR step " << step << "] " << msg << "\n";
    return "skip '" + input + "'";
}