// ktf-treedump.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include "KTreeFile.h"

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    string filename, nodepath = "/", format;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--node=", 7) == 0) {
            nodepath = string(argv[i]).substr(7);
        }
        else if (strncmp(argv[i], "--format=", 9) == 0) {
            format = string(argv[i]).substr(9);
        }
        else if (filename.empty()) {
            filename = argv[i];
        }
    }
    if (filename.empty()) {
        cerr << "USAGE: " << argv[0];
        cerr << " [ --node=NODEPATH ] [ --format=FORMAT ] FILENAME" << endl;
	return -1;
    }

    KTree tree;
    try {
        KTreeFile(filename).Read(tree);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    if (format == "ktf") {
        KKtfTreeSerializer(cout).Serialize(tree[nodepath]);
    }
    else if (format == "json") {
        KJsonTreeSerializer(cout).Serialize(tree[nodepath]);
    }
    else if (format == "xml") {
        KXmlTreeSerializer(cout).Serialize(tree[nodepath]);
    }
    else if (format == "plist") {
        KPlistTreeSerializer(cout).Serialize(tree[nodepath]);
    }
    else {
        tree[nodepath].Dump();
    }

    return 0;
}
