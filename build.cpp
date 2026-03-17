#include <bits/stdc++.h>
using namespace std;

using Production = vector<string>;
using Grammar = map<string, vector<Production>>;

// -------------------- UTIL --------------------
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
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

        int pos = line.find("->");
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
void printGrammar(const Grammar &g) {
    for (auto &[A, prods] : g) {
        cout << A << " -> ";
        for (int i = 0; i < prods.size(); i++) {
            for (auto &s : prods[i]) cout << s << " ";
            if (i != prods.size() - 1) cout << "| ";
        }
        cout << "\n";
    }
}

// -------------------- LEFT FACTORING --------------------
vector<string> LCP(const vector<Production>& prods) {
    if (prods.empty()) return {};
    vector<string> prefix = prods[0];

    for (int i = 1; i < prods.size(); i++) {
        vector<string> temp;
        for (int j = 0; j < prefix.size() && j < prods[i].size(); j++) {
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

                g[A] = newA;
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
        b.push_back(A1);
        newA.push_back(b);
    }

    for (auto &a : alpha) {
        a.push_back(A1);
        newA1.push_back(a);
    }

    newA1.push_back({"ε"});

    g[A] = newA;
    g[A1] = newA1;
}

void removeLeftRecursion(Grammar &g) {
    vector<string> nts;
    for (auto &p : g) nts.push_back(p.first);

    for (int i = 0; i < nts.size(); i++) {
        for (int j = 0; j < i; j++) {
            vector<Production> newP;

            for (auto &p : g[nts[i]]) {
                if (!p.empty() && p[0] == nts[j]) {
                    for (auto &q : g[nts[j]]) {
                        Production temp = q;
                        temp.insert(temp.end(), p.begin() + 1, p.end());
                        newP.push_back(temp);
                    }
                } else newP.push_back(p);
            }
            g[nts[i]] = newP;
        }
        removeDirect(nts[i], g);
    }
}

// -------------------- FIRST --------------------
map<string, set<string>> FIRST;

set<string> firstOf(string X, Grammar &g, map<string,bool> &vis) {
    if (!FIRST[X].empty()) return FIRST[X];

    if (isTerminal(X) || X == "ε")
        return FIRST[X] = {X};

    if (vis[X]) return FIRST[X];
    vis[X] = true;

    for (auto &p : g[X]) {
        bool nullable = true;

        for (auto &sym : p) {
            auto temp = firstOf(sym, g, vis);

            for (auto &t : temp)
                if (t != "ε") FIRST[X].insert(t);

            if (!temp.count("ε")) {
                nullable = false;
                break;
            }
        }

        if (nullable) FIRST[X].insert("ε");
    }

    vis[X] = false;
    return FIRST[X];
}

void computeFIRST(Grammar &g) {
    map<string,bool> vis;
    for (auto &p : g)
        firstOf(p.first, g, vis);
}

// -------------------- FOLLOW --------------------
map<string, set<string>> FOLLOW;

set<string> firstSeq(vector<string> seq) {
    set<string> res;
    bool nullable = true;

    for (auto &s : seq) {
        for (auto &t : FIRST[s])
            if (t != "ε") res.insert(t);

        if (!FIRST[s].count("ε")) {
            nullable = false;
            break;
        }
    }

    if (nullable) res.insert("ε");
    return res;
}

void computeFOLLOW(Grammar &g, string start) {
    FOLLOW[start].insert("$");

    bool changed = true;

    while (changed) {
        changed = false;

        for (auto &[A, prods] : g) {
            for (auto &p : prods) {
                for (int i = 0; i < p.size(); i++) {
                    string B = p[i];
                    if (isTerminal(B)) continue;

                    vector<string> beta(p.begin()+i+1, p.end());
                    auto f = firstSeq(beta);

                    for (auto &t : f)
                        if (t != "ε" && FOLLOW[B].insert(t).second)
                            changed = true;

                    if (beta.empty() || f.count("ε")) {
                        for (auto &t : FOLLOW[A])
                            if (FOLLOW[B].insert(t).second)
                                changed = true;
                    }
                }
            }
        }
    }
}

// -------------------- PARSING TABLE --------------------
map<string, map<string, Production>> table;

set<string> getTerminals(Grammar &g) {
    set<string> t;
    for (auto &[_, prods] : g)
        for (auto &p : prods)
            for (auto &s : p)
                if (isTerminal(s) && s != "ε")
                    t.insert(s);
    t.insert("$");
    return t;
}

set<string> firstProd(Production p) {
    return firstSeq(p);
}

bool buildTable(Grammar &g) {
    bool ok = true;

    for (auto &[A, prods] : g) {
        for (auto &p : prods) {
            auto f = firstProd(p);

            for (auto &a : f) {
                if (a == "ε") continue;
                if (!table[A][a].empty()) ok = false;
                table[A][a] = p;
            }

            if (f.count("ε")) {
                for (auto &b : FOLLOW[A]) {
                    if (!table[A][b].empty()) ok = false;
                    table[A][b] = p;
                }
            }
        }
    }
    return ok;
}


void saveTableToCSV(Grammar &g, const string &filename = "table.csv") {
    auto terminals = getTerminals(g);

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file!\n";
        return;
    }

    // Header row
    file << "NT/T,";
    for (auto &t : terminals) {
        file << "\"" << t << "\",";
    }
    file << "\n";

    // Table rows
    for (auto &[A, _] : g) {
        file << "\"" << A << "\",";

        for (auto &t : terminals) {
            if (table[A].count(t)) {
                file << "\"";                 // opening quote
                file << A << "->";
                for (auto &s : table[A][t]) {
                    file << s;
                }
                file << "\"";                 // closing quote
            }
            file << ",";
        }
        file << "\n";
    }

    file.close();
    cout << "\n\nTable saved to " << filename << endl;
}

void printTable(Grammar &g) {
    auto terminals = getTerminals(g);

    cout << "\nLL(1) Table:\n\n";
    cout << "NT/T\t";
    for (auto &t : terminals) cout << t << "\t";
    cout << "\n";

    for (auto &[A, _] : g) {
        cout << A << "\t";
        for (auto &t : terminals) {
            if (table[A].count(t)) {
                cout << A << "->";
                for (auto &s : table[A][t]) cout << s;
            }
            cout << "\t";
        }
        cout << "\n";
    }
}

// -------------------- PRINT SETS --------------------
void printFIRST() {
    cout << "\nFIRST:\n";
    for (auto &[A, s] : FIRST) {
        cout << A << " : { ";
        for (auto &x : s) cout << x << " ";
        cout << "}\n";
    }
}

void printFOLLOW() {
    cout << "\nFOLLOW:\n";
    for (auto &[A, s] : FOLLOW) {
        cout << A << " : { ";
        for (auto &x : s) cout << x << " ";
        cout << "}\n";
    }
}

// -------------------- MAIN --------------------
int main() {
    Grammar g = readCFG("grammar1.txt");

    cout << "\nOriginal Grammar:\n";
    printGrammar(g);

    leftFactor(g);
    cout << "\nAfter Left Factoring:\n";
    printGrammar(g);

    removeLeftRecursion(g);
    cout << "\nAfter Left Recursion Removal:\n";
    printGrammar(g);

    computeFIRST(g);
    printFIRST();

    computeFOLLOW(g, g.begin()->first);
    printFOLLOW();

    bool isLL1 = buildTable(g);
    printTable(g);
    saveTableToCSV(g);

    cout << (isLL1 ? "\nGrammar is LL(1)\n" : "\nGrammar is NOT LL(1)\n");

}