#include "first_follow.h"

map<string, set<string>> FIRST;
map<string, set<string>> FOLLOW;

// -------------------- FIRST --------------------
set<string> firstOf(string X, Grammar &g, map<string, bool> &vis) {
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
    map<string, bool> vis;
    for (auto &p : g)
        firstOf(p.first, g, vis);
}

// -------------------- FOLLOW --------------------
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
                for (int i = 0; i < (int)p.size(); i++) {
                    string B = p[i];
                    if (isTerminal(B)) continue;

                    vector<string> beta(p.begin() + i + 1, p.end());
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

// -------------------- PRINT --------------------
void printFIRST(ostream &out) {
    out << "\nFIRST:\n";
    for (auto &[A, s] : FIRST) {
        out << A << " : { ";
        for (auto &x : s) out << x << " ";
        out << "}\n";
    }
}

void printFOLLOW(ostream &out) {
    out << "\nFOLLOW:\n";
    for (auto &[A, s] : FOLLOW) {
        out << A << " : { ";
        for (auto &x : s) out << x << " ";
        out << "}\n";
    }
}
