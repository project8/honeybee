// ktf-dump.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <fstream>
#include "KTabreeFile.h"

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    if (argc <= 1) {
	cerr << "USAGE: " << argv[0] << " FILENAME" << endl;
	return -1;
    }
    string fileName = argv[1];

    KTabree tabree;
    try {
	KTabreeFile(fileName).Read(tabree);
    }
    catch (KException &e) {
	cerr << "ERROR: " << e.what() << endl;
	return -1;
    }

    tabree.Dump();

    return 0;
}
