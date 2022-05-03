// ktf-embed.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <fstream>
#include "KTabreeFormatProcessor.h"

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    if (argc <= 1) {
	cerr << argv[0] << ": generates Tabree-Embedded file" << endl;
	cerr << "USAGE: " << argv[0] << " INPUT_FILENAME > OUTPUT_FILENAME" << endl;
	return -1;
    }
    string fileName = argv[1];
    ifstream file(fileName.c_str());
    if (! file) {
        cerr << "ERROR: unable to open file: " << fileName << endl;
        return -1;
    }

    string name = fileName.substr(0, fileName.find_first_of('.'));
    string::size_type pathLength = name.find_last_of('/');
    if (pathLength != string::npos) {
        name.erase(0, pathLength+1);
    }
    if (name.empty()) {
        name = "untitled";
    }

    cout << "static const char* KTE_" << name << " = " << endl;

    KTabreeFormatLineExtractor lineExtractor(file);
    string line;
    while(lineExtractor.GetNext(line)) {
        cout << '"';
        for (unsigned i = 0; i < line.size(); i++) {
            if ((line[i] == '\\') || (line[i] == '"')) {
                cout << '\\';
            }
            cout << line[i];
        }
        cout << "\\n" << '"' << endl;
    }

    cout << ";" << endl;

    return 0;
}
