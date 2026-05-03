#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
using namespace std;
 
struct TreeNode {
    string          label;
    vector<shared_ptr<TreeNode>> children;
 
    TreeNode(const string &l) : label(l) {}
};
 
using NodePtr = shared_ptr<TreeNode>;
 
// print tree with indented ascii art
void printtree(NodePtr root, ostream &out = cout, const string &prefix = "", bool islast = true);