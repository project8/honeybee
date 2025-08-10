// demo-parser.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <kebap/Kebap.h>

using namespace std;
using namespace kebap;


int main(int argc, char** argv)
{
    if (argc < 2) {
	cerr << "Usage: " << argv[0] << " SourceFileName" << endl;
	return 0;
    }
    ifstream SourceFile(argv[1]);
    if (! SourceFile) {
	cerr << "ERROR: unable to open " << argv[1] << endl;
	return 0;
    }

    KPStandardParser Parser(argc-1, argv+1);
    try {
        Parser.Parse(SourceFile);
    }
    catch (KPException &e) {
	cerr << "ERROR: " << e.what() << endl;
	return EXIT_FAILURE;
    }

    KPValue Result;
    try {
	if (Parser.HasEntryOf("main")) {
	    Result = Parser.Execute("main");
	}
	else {
	    Result = Parser.Execute();
	}
    }
    catch (KPException &e) {
        cerr << "ERROR: " << e.what() << endl;
    }
    
    return Result.AsLong();
}
