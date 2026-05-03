#include "parser.h"
#include <filesystem>
 
namespace fs = std::filesystem;
 
// derive base name e.g. "grammar1" from "input/grammar1.txt"
static string basename(const string &path) {
    return fs::path(path).stem().string();
}
 
static void processgrammar(const string &inputfile, const string &outputdir) {
    // Guard: skip input-string files (names containing '_') if passed directly
    string stem = fs::path(inputfile).stem().string();
    if (stem.find('_') != string::npos) {
        cout << "  [SKIP] Not a grammar file: " << inputfile << "\n";
        return;
    }
 
    // reset global state for each grammar
    FIRST.clear();
    FOLLOW.clear();
    table.clear();
 
    cout << "\n========================================\n";
    cout << "Processing: " << inputfile << "\n";
    cout << "========================================\n";
 
    Grammar g = readCFG(inputfile);
    if (g.empty()) {
        cerr << "  [ERROR] Could not read or empty grammar: " << inputfile << "\n";
        return;
    }
 
    // read start symbol: always use the first LHS declared in the file.
    // this is unambiguous, matches the textbook convention, and survives
    // mutual recursion where every NT appears in some other NT's RHS.
    auto readstart = [&]() -> string {
        ifstream ff(inputfile);
        string ln;
        while (getline(ff, ln)) {
            // strip \r for windows-style line endings
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            size_t arrow = ln.find("->");
            if (arrow == string::npos) continue;
            string lhs = ln.substr(0, arrow);
            // trim whitespace from lhs
            size_t a = lhs.find_first_not_of(" \t");
            size_t b = lhs.find_last_not_of(" \t");
            if (a == string::npos) continue;
            return lhs.substr(a, b - a + 1);
        }
        return g.begin()->first; // last resort
    };
    string start = readstart();
    cout << "  [Start symbol: " << start << "]\n";
 
    string base    = basename(inputfile);
    string outgrm  = outputdir + "/" + base + "_transformed.txt";
    string outff   = outputdir + "/" + base + "_first_follow.txt";
    string outtbl  = outputdir + "/" + base + "_parsing_table.txt";
    string outcsv  = outputdir + "/" + base + "_parsing_table.csv";
 
    // ---- part 1: transformations ----
    cout << "\nOriginal Grammar:\n";
    printGrammar(g);
 
    leftFactor(g);
    cout << "\nAfter Left Factoring:\n";
    printGrammar(g);
 
    removeLeftRecursion(g, start);
    cout << "\nAfter Left Recursion Removal:\n";
    printGrammar(g);
 
    {
        ofstream f(outgrm);
        f << "=== Transformed Grammar ===\n\n";
        printGrammar(g, f);
    }
    cout << "  -> Saved: " << outgrm << "\n";
 
    // ---- first / follow ----
    computeFIRST(g);
    computeFOLLOW(g, start);
 
    printFIRST(g);
    printFOLLOW(g);
 
    {
        ofstream f(outff);
        printFIRST(g, f);
        printFOLLOW(g, f);
    }
    cout << "  -> Saved: " << outff << "\n";
 
    // ---- parsing table ----
    bool isll1 = buildtable(g);
    printtable(g);
 
    {
        ofstream f(outtbl);
        f << (isll1 ? "Grammar is LL(1)\n" : "Grammar is NOT LL(1)\n");
        printtable(g, f);
    }
    cout << "  -> Saved: " << outtbl << "\n";
 
    savetabletocsv(g, outcsv);
 
    cout << (isll1 ? "\nResult: LL(1)\n" : "\nResult: NOT LL(1)\n");
 
    // ---- part 2: parse input strings ----
    // look for matching input files: <base>_valid.txt, <base>_errors.txt,
    // <base>_edge_cases.txt  (or any <base>_*.txt in same directory)
    fs::path grammarpath(inputfile);
    fs::path inputdir = grammarpath.parent_path();
 
    vector<string> suffixes = {"_valid", "_errors", "_edge_cases"};
    bool anyfound = false;
 
    for (auto &suf : suffixes) {
        string candidate = inputdir.string() + "/" + base + suf + ".txt";
        if (!fs::exists(candidate)) continue;
 
        anyfound = true;
        string tag      = base + suf;
        string tracefile = outputdir + "/" + tag + "_trace.txt";
 
        cout << "\n  Parsing strings from: " << candidate << "\n";
        runparser(start, candidate, tracefile);
    }
 
    if (!anyfound)
        cout << "  [INFO] No input string files found for " << base
             << " (expected input/" << base << "_valid.txt etc.)\n";
}
 
int main(int argc, char *argv[]) {
    const string inputdir  = "input";
    const string outputdir = "output";
 
    fs::create_directories(outputdir);
 
    if (argc > 1) {
        for (int i = 1; i < argc; i++)
            processgrammar(argv[i], outputdir);
    } else {
        if (!fs::exists(inputdir) || !fs::is_directory(inputdir)) {
            cerr << "No input files given and '" << inputdir << "' not found.\n"
                 << "Usage: " << argv[0] << " [input/grammar1.txt ...]\n";
            return 1;
        }
 
        vector<string> files;
        for (auto &e : fs::directory_iterator(inputdir)) {
            string stem = e.path().stem().string();
            // only grammar files: .txt with no underscore in filename
            if (e.path().extension() == ".txt" &&
                stem.find('_') == string::npos)
                files.push_back(e.path().string());
        }
 
        sort(files.begin(), files.end());
 
        if (files.empty()) {
            cerr << "No grammar .txt files found in '" << inputdir << "'.\n";
            return 1;
        }
 
        for (auto &f : files)
            processgrammar(f, outputdir);
    }
 
    return 0;
}
 