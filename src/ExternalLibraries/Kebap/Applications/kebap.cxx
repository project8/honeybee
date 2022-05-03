// kebap.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <string>
#include <vector>
#include "Kebap.h"

using namespace std;
using namespace kebap;


int main(int argc, char** argv)
{
    KPStandardParser Parser(argc-1, argv+1);

    KPModule* Module = Parser.GetModule();
    KPSymbolTable* SymbolTable = Parser.GetSymbolTable();
    KPStatementParser* StatementParser = Parser.GetStatementParser();

    KPTokenizer Tokenizer(cin, Parser.GetTokenTable());

    vector<ifstream*> InputStreamList;
    if (argc > 1) {
	string FileName = argv[1];
	auto *InputStream = new ifstream(FileName.c_str());
	if (! *InputStream) {
	    throw KPException() << "unable to open file: " << FileName;
	}
	InputStreamList.push_back(InputStream);
	Tokenizer.InputBuffer()->SetChildInput(*InputStream);
    }

    cout << "> " << flush;

    vector<KPModuleEntry*> EntryList;
    while (! Tokenizer.LookAhead().IsEmpty()) {
        KPStatement* Statement = nullptr;
	try {
            KPModuleEntry* Entry = Module->CreateEntry(&Tokenizer);
            if (Entry != nullptr) {
                Entry->Parse(&Tokenizer, StatementParser, SymbolTable);
                EntryList.push_back(Entry);
                continue;
            }

            Statement = StatementParser->Parse(&Tokenizer, SymbolTable);
            KPValue Result = Statement->Execute(SymbolTable).ReturnValue;
            if (! Result.IsVoid()) {
                cout << Result.AsString() << endl;
            }
            cout << "> " << flush;
	}
	catch (KPException &e) {
	    cerr << "ERROR: " << e.what() << endl;
	}
        delete Statement;	
    }

    for (unsigned i = 0; i < InputStreamList.size(); i++) {
	delete InputStreamList[i];
    }
    for (unsigned j = 0; j < EntryList.size(); j++) {
	delete EntryList[j];
    }

    return 0;
}
