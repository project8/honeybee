// kte-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <tabree/KTabreeFile.h>

using namespace std;
using namespace tabree;

#include "UnitTable.kte"


int main(int argc, char** argv)
{
    KTabree tabree;
    try {
	KTabreeEmbedded(KTE_UnitTable).Read(tabree);
    }
    catch (KException &e) {
	cerr << "ERROR: " << e.what() << endl;
	return -1;
    }

    tabree.Dump();

    return 0;
}
