#include "parser.h"
#include <filesystem>

namespace fs = std::filesystem;

// Derive a base name (e.g. "grammar1") from a full path like "input/grammar1.txt"
static string baseName(const string &path) {
    fs::path p(path);
    return p.stem().string();   // filename without extension
}

static void processGrammar(const string &inputFile, const string &outputDir) {
    // ---- Reset global state for each grammar file ----
    FIRST.clear();
    FOLLOW.clear();
    table.clear();

    cout << "\n========================================\n";
    cout << "Processing: " << inputFile << "\n";
    cout << "========================================\n";

    Grammar g = readCFG(inputFile);
    if (g.empty()) {
        cerr << "  [ERROR] Could not read or empty grammar: " << inputFile << "\n";
        return;
    }

    string base   = baseName(inputFile);
    string outGrm = outputDir + "/" + base + "_transformed.txt";
    string outFF  = outputDir + "/" + base + "_first_follow.txt";
    string outTbl = outputDir + "/" + base + "_parsing_table.txt";
    string outCSV = outputDir + "/" + base + "_parsing_table.csv";

    // ---------- Transformations ----------
    cout << "\nOriginal Grammar:\n";
    printGrammar(g);

    leftFactor(g);
    cout << "\nAfter Left Factoring:\n";
    printGrammar(g);

    removeLeftRecursion(g);
    cout << "\nAfter Left Recursion Removal:\n";
    printGrammar(g);

    // Write grammar_transformed.txt
    {
        ofstream f(outGrm);
        f << "=== Original + Transformed Grammar ===\n\n";
        f << "After Left Factoring and Left Recursion Removal:\n\n";
        printGrammar(g, f);
    }
    cout << "  -> Saved: " << outGrm << "\n";

    // ---------- FIRST / FOLLOW ----------
    computeFIRST(g);
    computeFOLLOW(g, g.begin()->first);

    printFIRST();
    printFOLLOW();

    // Write first_follow_sets.txt
    {
        ofstream f(outFF);
        printFIRST(f);
        printFOLLOW(f);
    }
    cout << "  -> Saved: " << outFF << "\n";

    // ---------- Parsing Table ----------
    bool isLL1 = buildTable(g);
    printTable(g);

    // Write parsing_table.txt  (human-readable)
    {
        ofstream f(outTbl);
        f << (isLL1 ? "Grammar is LL(1)\n" : "Grammar is NOT LL(1)\n");
        printTable(g, f);
    }
    cout << "  -> Saved: " << outTbl << "\n";

    // Write parsing_table.csv
    saveTableToCSV(g, outCSV);
    cout << "  -> Saved: " << outCSV << "\n";

    cout << (isLL1 ? "\nResult: LL(1)\n" : "\nResult: NOT LL(1)\n");
}

int main(int argc, char *argv[]) {
    const string inputDir  = "input";
    const string outputDir = "output";

    // Create output directory if it doesn't exist
    fs::create_directories(outputDir);

    if (argc > 1) {
        // Files provided on command line (from Makefile GRAMMARS variable)
        for (int i = 1; i < argc; i++)
            processGrammar(argv[i], outputDir);
    } else {
        // Auto-discover all .txt files in input/
        if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
            cerr << "No input files given and '" << inputDir
                 << "' directory not found.\n"
                 << "Usage: " << argv[0] << " [input/grammar1.txt ...]\n";
            return 1;
        }

        vector<string> files;
        for (auto &entry : fs::directory_iterator(inputDir))
            if (entry.path().extension() == ".txt")
                files.push_back(entry.path().string());

        sort(files.begin(), files.end());

        if (files.empty()) {
            cerr << "No .txt files found in '" << inputDir << "'.\n";
            return 1;
        }

        for (auto &f : files)
            processGrammar(f, outputDir);
    }

    return 0;
}
