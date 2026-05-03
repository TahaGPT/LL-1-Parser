#include "grammar.h"
 
// -------------------- UTIL --------------------
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}
 
bool isTerminal(const string &s) {
    return !s.empty() && !isupper(s[0]);
}
 
// -------------------- CFG INPUT --------------------
Production tokenize(const string &prod) {
    Production tokens;
    stringstream ss(prod);
    string token;
 
    while (ss >> token) {
        if (token == "epsilon" || token == "@")
            token = "ε";
        tokens.push_back(token);
    }
    return tokens;
}
 
Grammar readCFG(const string &filename) {
    ifstream file(filename);
    Grammar grammar;
    string line;
 
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
 
        int pos    = line.find("->");
        string lhs = trim(line.substr(0, pos));
        string rhs = trim(line.substr(pos + 2));
 
        stringstream ss(rhs);
        string part;
 
        while (getline(ss, part, '|')) {
            grammar[lhs].push_back(tokenize(trim(part)));
        }
    }
    return grammar;
}
 
// -------------------- PRINT --------------------
void printGrammar(const Grammar &g, ostream &out) {
    for (auto &[A, prods] : g) {
        out << A << " -> ";
        for (int i = 0; i < (int)prods.size(); i++) {
            for (auto &s : prods[i]) out << s << " ";
            if (i != (int)prods.size() - 1) out << "| ";
        }
        out << "\n";
    }
}
 
// -------------------- LEFT FACTORING --------------------
vector<string> LCP(const vector<Production> &prods) {
    if (prods.empty()) return {};
    vector<string> prefix = prods[0];
 
    for (int i = 1; i < (int)prods.size(); i++) {
        vector<string> temp;
        for (int j = 0; j < (int)prefix.size() && j < (int)prods[i].size(); j++) {
            if (prefix[j] == prods[i][j])
                temp.push_back(prefix[j]);
            else break;
        }
        prefix = temp;
    }
    return prefix;
}
 
void leftFactor(Grammar &g) {
    bool changed = true;
 
    while (changed) {
        changed = false;
 
        for (auto &[A, prods] : g) {
            map<string, vector<Production>> groups;
 
            for (auto &p : prods)
                if (!p.empty()) groups[p[0]].push_back(p);
 
            for (auto &[_, group] : groups) {
                if (group.size() < 2) continue;
 
                auto prefix = LCP(group);
                if (prefix.empty()) continue;
 
                string A1 = A + "'";
                while (g.count(A1)) A1 += "'";
 
                vector<Production> newA, newA1;
 
                for (auto &p : group) {
                    Production suffix(p.begin() + prefix.size(), p.end());
                    if (suffix.empty()) suffix.push_back("ε");
                    newA1.push_back(suffix);
                }
 
                for (auto &p : prods)
                    if (find(group.begin(), group.end(), p) == group.end())
                        newA.push_back(p);
 
                Production np = prefix;
                np.push_back(A1);
                newA.push_back(np);
 
                g[A]  = newA;
                g[A1] = newA1;
 
                changed = true;
                break;
            }
            if (changed) break;
        }
    }
}
 
// -------------------- LEFT RECURSION --------------------
void removeDirect(string A, Grammar &g) {
    vector<Production> alpha, beta;
 
    for (auto &p : g[A]) {
        if (!p.empty() && p[0] == A)
            alpha.push_back(Production(p.begin() + 1, p.end()));
        else
            beta.push_back(p);
    }
 
    if (alpha.empty()) return;
 
    string A1 = A + "'";
    while (g.count(A1)) A1 += "'";
 
    vector<Production> newA, newA1;
 
    for (auto &b : beta) {
        // If beta production is pure epsilon, the new production is just {A1}
        // (epsilon is the empty string marker, not a real symbol to keep)
        if (b.size() == 1 && b[0] == "ε")
            newA.push_back({A1});
        else {
            Production nb = b;
            nb.push_back(A1);
            newA.push_back(nb);
        }
    }
 
    for (auto &a : alpha) {
        Production na = a;
        na.push_back(A1);
        newA1.push_back(na);
    }
 
    newA1.push_back({"ε"});
 
    g[A]  = newA;
    g[A1] = newA1;
}
 
void removeLeftRecursion(Grammar &g, const string &startSymbol) {
    // Collect non-terminals in a stable order.
    // Ensure the start symbol is the first non-terminal evaluated so that it
    // is substituted into others, correctly breaking indirect left recursion loops
    // and preventing unnecessary substitution for grammars without left recursion.
    // Non-terminals introduced by left factoring (containing "'") are appended
    // after the original ones so they are processed last.
    vector<string> originals, primed;
    for (auto &p : g) {
        if (p.first.find('\'') != string::npos)
            primed.push_back(p.first);
        else if (p.first != startSymbol)
            originals.push_back(p.first);
    }
    
    // stable sort each group alphabetically (map gives alphabetical order)
    vector<string> nts;
    if (g.count(startSymbol)) {
        nts.push_back(startSymbol);
    }
    nts.insert(nts.end(), originals.begin(), originals.end());
    nts.insert(nts.end(), primed.begin(),    primed.end());
 
    for (int i = 0; i < (int)nts.size(); i++) {
        for (int j = 0; j < i; j++) {
            vector<Production> newP;
 
            for (auto &p : g[nts[i]]) {
                if (!p.empty() && p[0] == nts[j]) {
                    // substitute: replace Ai -> Aj β  with  Ai -> γ β  for each Aj -> γ
                    Production suffix(p.begin() + 1, p.end());
                    for (auto &q : g[nts[j]]) {
                        // Treat pure-epsilon production as empty base
                        Production base = q;
                        if (base.size() == 1 && base[0] == "ε")
                            base.clear();
 
                        if (base.empty()) {
                            // Aj -> ε: substituted production is just suffix (or ε if suffix empty)
                            if (suffix.empty())
                                newP.push_back({"ε"});
                            else
                                newP.push_back(suffix);
                        } else {
                            Production temp = base;
                            temp.insert(temp.end(), suffix.begin(), suffix.end());
                            newP.push_back(temp);
                        }
                    }
                } else {
                    newP.push_back(p);
                }
            }
            g[nts[i]] = newP;
        }
        removeDirect(nts[i], g);
    }
}
 