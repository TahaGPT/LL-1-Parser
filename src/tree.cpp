#include "tree.h"
 
// recursive indented tree printer
void printtree(NodePtr root, ostream &out, const string &prefix, bool islast) {
    if (!root) return;
 
    out << prefix;
    out << (islast ? "└── " : "├── ");
    out << root->label << "\n";
 
    string childprefix = prefix + (islast ? "    " : "│   ");
    for (int i = 0; i < (int)root->children.size(); i++)
        printtree(root->children[i], out, childprefix, i == (int)root->children.size() - 1);
}