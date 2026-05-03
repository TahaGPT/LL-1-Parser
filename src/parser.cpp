#include "parser.h"
#include "stack.h"
#include "error_handler.h"
#include <iomanip>
#include <sstream>

map<string, map<string, vector<Production>>> table;

// -------------------- helpers --------------------

set<string> getterminals(Grammar &g) {
    set<string> t;
    for (auto &[_, prods] : g)
        for (auto &p : prods)
            for (auto &s : p)
                if (isTerminal(s) && s != "ε")
                    t.insert(s);
    t.insert("$");
    return t;
}

set<string> firstprod(Production p) {
    return firstSeq(p);
}

// -------------------- parsing table --------------------

bool buildtable(Grammar &g) {
    table.clear();
    bool ok = true;

    for (auto &[A, prods] : g) {
        // Pass 1: non-nullable productions (FIRST does not contain ε)
        for (auto &p : prods) {
            auto f = firstprod(p);
            if (f.count("ε")) continue;

            for (auto &a : f) {
                if (a == "ε") continue;
                if (!table[A][a].empty())
                    ok = false; // LL(1) conflict
                table[A][a].push_back(p);
            }
        }

        // Pass 2: nullable productions (FIRST contains ε)
        for (auto &p : prods) {
            auto f = firstprod(p);
            if (!f.count("ε")) continue;

            // non-ε terminals in FIRST(p)
            for (auto &a : f) {
                if (a == "ε") continue;
                
                bool found = false;
                for (auto &ex : table[A][a]) if (ex == p) { found = true; break; }
                if (!found) {
                    if (!table[A][a].empty())
                        ok = false;
                    table[A][a].push_back(p);
                }
            }

            // FOLLOW cells
            for (auto &b : FOLLOW[A]) {
                bool found = false;
                for (auto &ex : table[A][b]) if (ex == p) { found = true; break; }
                if (!found) {
                    if (!table[A][b].empty())
                        ok = false;
                    table[A][b].push_back(p);
                }
            }
        }
    }
    return ok;
}

// -------------------- table output --------------------

void printtable(Grammar &g, ostream &out) {
    auto terminals = getterminals(g);

    out << "\nLL(1) Table:\n\n";
    out << "NT/T\t";
    for (auto &t : terminals) out << t << "\t";
    out << "\n";

    for (auto &[A, _] : g) {
        out << A << "\t";
        for (auto &t : terminals) {
            if (table[A].count(t) && !table[A][t].empty()) {
                for (size_t i = 0; i < table[A][t].size(); i++) {
                    if (i > 0) out << " | ";
                    out << A << "->";
                    for (auto &s : table[A][t][i]) out << s;
                }
            }
            out << "\t";
        }
        out << "\n";
    }
}

void savetabletocsv(Grammar &g, const string &filename) {
    auto terminals = getterminals(g);

    ofstream file(filename);
    if (!file.is_open()) { cerr << "Error opening file!\n"; return; }

    file << "NT/T,";
    for (auto &t : terminals) file << "\"" << t << "\",";
    file << "\n";

    for (auto &[A, _] : g) {
        file << "\"" << A << "\",";
        for (auto &t : terminals) {
            if (table[A].count(t) && !table[A][t].empty()) {
                file << "\"";
                for (size_t i = 0; i < table[A][t].size(); i++) {
                    if (i > 0) file << " | ";
                    file << A << "->";
                    for (auto &s : table[A][t][i]) file << s;
                }
                file << "\"";
            }
            file << ",";
        }
        file << "\n";
    }

    file.close();
    cout << "Table saved to " << filename << "\n";
}

// -------------------- part 2: stack-based parser --------------------

// build a readable string from remaining tokens
static string remaininginput(const vector<string> &tokens, int pos) {
    string s;
    for (int i = pos; i < (int)tokens.size(); i++) s += tokens[i] + " ";
    return s.empty() ? "$" : s;
}

// print one row of the trace table
static void traceline(int step, const string &stk, const string &inp,
                      const string &action, ostream &out) {
    out << left
        << setw(6)  << step
        << setw(36) << stk
        << setw(28) << inp
        << action << "\n";
}

// -------------------- parse tree builder --------------------
// stack entry: pairs a grammar symbol with its tree node
struct stackentry {
    string  sym;
    NodePtr node;
};

parseresult parsestring(const string &start, vector<string> tokens, ostream &out) {
    // append $ sentinel
    tokens.push_back("$");

    // trace header
    out << left
        << setw(6)  << "Step"
        << setw(36) << "Stack (bottom->top)"
        << setw(28) << "Input"
        << "Action\n";
    out << string(90, '-') << "\n";

    // ---- initialise ----
    NodePtr root = make_shared<TreeNode>(start);

    // working stack stores (symbol, treenode) pairs
    vector<stackentry> wstack;
    wstack.push_back({"$",   nullptr});
    wstack.push_back({start, root});

    int pos    = 0;
    int step   = 0;
    int errors = 0;

    auto stackstr = [&]() -> string {
        string s;
        for (auto &e : wstack) s += e.sym + " ";
        if (!s.empty()) s.pop_back();
        return s;
    };

    while (!wstack.empty()) {
        string X = wstack.back().sym;
        string a = tokens[pos];

        // build display strings before any mutation
        string stkdisp = stackstr();
        string inpdisp = remaininginput(tokens, pos);

        ++step;

        // case 1: both are $  -> accept (only clean if no errors)
        if (X == "$" && a == "$") {
            traceline(step, stkdisp, inpdisp, errors ? "Accept (with errors)" : "Accept", out);
            return {errors == 0, errors, root};
        }

        // case 2: top is terminal, matches input
        if (isTerminal(X) && X == a) {
            traceline(step, stkdisp, inpdisp, "Match " + X, out);
            wstack.pop_back();
            pos++;
            continue;
        }

        // case 3: top is non-terminal, look up table
        if (!isTerminal(X) && X != "$") {
            if (table.count(X) && table[X].count(a) && !table[X][a].empty()) {
                // Fallback for conflicts: pick first, but prefer one starting with current terminal
                Production prod = table[X][a][0];
                for (auto &p : table[X][a]) {
                    if (!p.empty() && p[0] == a) {
                        prod = p;
                        break;
                    }
                }
                NodePtr node = wstack.back().node;

                // build action string
                string act = X + " -> ";
                for (auto &s : prod) act += s + " ";

                traceline(step, stkdisp, inpdisp, act, out);

                wstack.pop_back();

                // first pass: create all child nodes in production order
                vector<NodePtr> children;
                for (auto &sym : prod)
                    children.push_back(make_shared<TreeNode>(sym));

                if (node) node->children = children;

                // second pass: push non-epsilon children onto stack in reverse
                // so that leftmost symbol ends up on top
                for (int i = (int)prod.size() - 1; i >= 0; i--) {
                    if (prod[i] != "ε")
                        wstack.push_back({prod[i], children[i]});
                }
                continue;
            }

            // error: no table entry -> panic mode recovery
            ++errors;
            string recovery = handleerror(step, X, a, out);

            // panic mode: skip input until we find something in FOLLOW[X]
            // or a terminal that matches X
            if (a == "$") {
                // premature end: pop the non-terminal
                traceline(step, stkdisp, inpdisp,
                          "[RECOVERY] Pop " + X, out);
                wstack.pop_back();
            } else {
                traceline(step, stkdisp, inpdisp,
                          "[RECOVERY] Skip '" + a + "'", out);
                pos++;
            }
            continue;
        }

        // case 4: terminal mismatch
        ++errors;
        handleerror(step, X, a, out);

        if (a == "$") {
            // missing symbol: pop from stack
            traceline(step, stkdisp, inpdisp,
                      "[RECOVERY] Pop " + X, out);
            wstack.pop_back();
        } else {
            // unexpected symbol: skip input
            traceline(step, stkdisp, inpdisp,
                      "[RECOVERY] Skip '" + a + "'", out);
            pos++;
        }
    }

    // fell through without accept
    traceline(step + 1, "", remaininginput(tokens, pos), "Reject", out);
    return {false, errors, nullptr};
}

// -------------------- file i/o --------------------

vector<vector<string>> readinputfile(const string &filename) {
    ifstream f(filename);
    vector<vector<string>> result;
    string line;

    while (getline(f, line)) {
        // trim
        size_t s = line.find_first_not_of(" \t\r\n");
        if (s == string::npos) continue;
        line = line.substr(s);

        vector<string> tokens;
        istringstream  ss(line);
        string         tok;
        while (ss >> tok) tokens.push_back(tok);
        if (!tokens.empty()) result.push_back(tokens);
    }
    return result;
}

void runparser(const string &start,
               const string &inputfile, const string &outputfile) {

    auto inputs = readinputfile(inputfile);
    if (inputs.empty()) {
        cout << "  [INFO] No input strings in " << inputfile << "\n";
        return;
    }

    ofstream out(outputfile);
    if (!out.is_open()) {
        cerr << "  [ERROR] Cannot open " << outputfile << "\n";
        return;
    }

    out << "Parsing Trace  |  Source: " << inputfile << "\n";
    out << string(90, '=') << "\n";

    int accepted = 0, rejected = 0;

    for (int i = 0; i < (int)inputs.size(); i++) {
        // build display string for the input
        string dispinput;
        for (auto &t : inputs[i]) dispinput += t + " ";

        out << "\n[String " << (i + 1) << "]  Input: " << dispinput << "\n";
        out << string(90, '-') << "\n";

        auto res = parsestring(start, inputs[i], out);

        out << "\nResult: ";
        if (res.accepted) {
            out << "ACCEPTED\n";
            accepted++;
            out << "\nParse Tree:\n";
            printtree(res.tree, out);
        } else {
            if (res.errors)
                out << "REJECTED (" << res.errors << " error(s))\n";
            else
                out << "REJECTED\n";
            rejected++;
        }

        out << "\n" << string(90, '=') << "\n";

        cout << "    String " << (i + 1) << ": \""
             << dispinput << "\" -> "
             << (res.accepted ? "ACCEPTED" : "REJECTED")
             << (res.errors ? " (" + to_string(res.errors) + " error(s))" : "") << "\n";
    }

    out << "\nSummary: " << accepted << " accepted, " << rejected << " rejected "
        << "out of " << inputs.size() << " string(s).\n";

    cout << "  -> Saved: " << outputfile << "\n";
}
