#include "first_follow.h"

map<string, set<string>> FIRST;
map<string, set<string>> FOLLOW;

// -------------------- FIRST --------------------
set<string> firstOf(string X, Grammar &g, map<string, bool> &vis) {
    if (!FIRST[X].empty()) return FIRST[X];

    // terminal or epsilon symbol: FIRST(X) = {X}
    if (isTerminal(X) || X == "ε")
        return FIRST[X] = {X};

    if (vis[X]) return FIRST[X];
    vis[X] = true;

    for (auto &p : g[X]) {
        bool nullable = true;

        for (auto &sym : p) {
            // use firstOf so ε and any un-seeded symbol are handled correctly
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
    // Always seed ε explicitly so firstSeq works when ε appears as a
    // literal symbol inside a production (e.g. Start -> ε Second).
    FIRST["ε"] = {"ε"};

    // Seed every terminal that appears in any RHS.
    for (auto &[_, prods] : g)
        for (auto &p : prods)
            for (auto &sym : p)
                if (isTerminal(sym) && sym != "ε")
                    FIRST[sym] = {sym};

    map<string, bool> vis;
    for (auto &p : g)
        firstOf(p.first, g, vis);
}

// -------------------- FOLLOW --------------------

// FIRST of a sequence of symbols. Relies on FIRST being fully populated.
// Also returns whether the entire sequence can derive epsilon.
struct FirstSeqResult {
    set<string> first;
    bool allNullable;
};

FirstSeqResult firstSeqWithNullable(vector<string> seq) {
    set<string> res;
    bool allNullable = true;

    for (auto &s : seq) {
        auto &fs = FIRST[s];
        for (auto &t : fs)
            if (t != "ε") res.insert(t);

        if (!fs.count("ε")) {
            allNullable = false;
            break;
        }
    }

    if (allNullable) res.insert("ε");
    return {res, allNullable};
}

// Keep original firstSeq for backward compatibility
set<string> firstSeq(vector<string> seq) {
    return firstSeqWithNullable(seq).first;
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
                    // skip terminals and the ε symbol — only process non-terminals
                    if (isTerminal(B) || B == "ε") continue;

                    vector<string> beta(p.begin() + i + 1, p.end());
                    auto result = firstSeqWithNullable(beta);

                    // Add FIRST(beta) - {ε} to FOLLOW(B)
                    for (auto &t : result.first)
                        if (t != "ε" && FOLLOW[B].insert(t).second)
                            changed = true;

                    // FIX: If beta is empty OR beta can derive epsilon (all symbols nullable),
                    // add FOLLOW(A) to FOLLOW(B)
                    if (beta.empty() || result.allNullable) {
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
// Print FIRST only for non-terminals (grammar LHS symbols), not bare terminals.
void printFIRST(Grammar &g, ostream &out) {
    out << "\nFIRST:\n";
    for (auto &[A, _] : g) {
        out << A << " : { ";
        for (auto &x : FIRST[A]) out << x << " ";
        out << "}\n";
    }
}

// Print FOLLOW only for non-terminals (grammar LHS symbols).
void printFOLLOW(Grammar &g, ostream &out) {
    out << "\nFOLLOW:\n";
    for (auto &[A, _] : g) {
        out << A << " : { ";
        for (auto &x : FOLLOW[A]) out << x << " ";
        out << "}\n";
    }
}
