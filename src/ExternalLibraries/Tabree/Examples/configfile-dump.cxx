// configfile-dump.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <tabree/KTreeFile.h>

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    if (argc < 2) {
        cerr << "USAGE: " << argv[0] << " INPUT_FILE" << endl;
        return 0;
    }
    string fileName = argv[1];
    
    KTree config;
    try {
        KTreeFile(fileName).Read(config);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
    
    config.Dump();
    KKtfTreeSerializer(cout).Serialize(config);
    KXmlTreeSerializer(cout).Serialize(config);
    KJsonTreeSerializer(cout).Serialize(config);

    return 0;
}
