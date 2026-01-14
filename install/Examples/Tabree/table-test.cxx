// table-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <fstream>
#include <tabree/KTabreeFile.h>

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    string fileName = "KFPDChannelMap.ktf";
    KTabree tabree;
    try {
	KTabreeFile(fileName).Read(tabree);
    }
    catch (KException &e) {
	cerr << "ERROR: " << e.what() << endl;
	return -1;
    }

    for (unsigned i = 0; i < tabree.NumberOfRows(); i++) {
        cout << tabree[i][0] << " ";   // column pointed by index
        cout << tabree[i]["FLTCard"] << " ";   // column pointed by name
        cout << tabree[i]["FLTChannel"] << " ";
        cout << tabree[i]["PreampSN"].Or("-") << " "; // with default value
        cout << endl;
    }

    return 0;
}
