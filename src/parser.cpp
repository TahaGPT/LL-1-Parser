#include "parser.h"

map<string, map<string, Production>> table;

// -------------------- HELPERS --------------------
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

// -------------------- PARSING TABLE --------------------
bool buildTable(Grammar &g) {
    table.clear();   // reset for each grammar run
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

// -------------------- OUTPUT --------------------
void printTable(Grammar &g, ostream &out) {
    auto terminals = getTerminals(g);

    out << "\nLL(1) Table:\n\n";
    out << "NT/T\t";
    for (auto &t : terminals) out << t << "\t";
    out << "\n";

    for (auto &[A, _] : g) {
        out << A << "\t";
        for (auto &t : terminals) {
            if (table[A].count(t)) {
                out << A << "->";
                for (auto &s : table[A][t]) out << s;
            }
            out << "\t";
        }
        out << "\n";
    }
}

void saveTableToCSV(Grammar &g, const string &filename) {
    auto terminals = getTerminals(g);

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file!\n";
        return;
    }

    // Header row
    file << "NT/T,";
    for (auto &t : terminals)
        file << "\"" << t << "\",";
    file << "\n";

    // Table rows
    for (auto &[A, _] : g) {
        file << "\"" << A << "\",";

        for (auto &t : terminals) {
            if (table[A].count(t)) {
                file << "\"";
                file << A << "->";
                for (auto &s : table[A][t]) file << s;
                file << "\"";
            }
            file << ",";
        }
        file << "\n";
    }

    file.close();
    cout << "\n\nTable saved to " << filename << endl;
}
