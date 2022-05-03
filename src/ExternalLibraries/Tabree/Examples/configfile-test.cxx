// configfile-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <fstream>
#include <tabree/KTreeFile.h>

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    string fileName = "UnitTable.ktf";
    //string fileName = "UnitTable.json";
    //string fileName = "UnitTable.xml";

    KTree config;
#if 1
    // example 1: simple reading with all defaults //
    try {
        KTreeFile(fileName).Read(config);
    }
#else
    // example 2: customized reading for XML files //
    try {
        ifstream input("UnitTable.xml");
        if (! input) {
            throw KException("unable to open file: " + fileName);
        }
        KXmlTreeFormat format;
        format.DisableWhiteSpaceTrimming();
        format.DisableInlineJson();
        //format.PreserveRootNode();
        //format.PreserveWhiteSpaceNode();
        format.Read(config, input);
    }
#endif
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
    

    // dump in various formats //
    config.Dump();
    KKtfTreeSerializer(cout).Serialize(config);
    KXmlTreeSerializer(cout).Serialize(config);
    KJsonTreeSerializer(cout).Serialize(config);


    // read values in various addressing methods //
    cout << config["Title"] << endl;
    cout << config["Date"].Or("Unknown Date").As<string>() << endl;
    
    string name = config["Quantity[0]/Name"];
    string standard_unit = config["Quantity"][0]["Unit"][0]["Symbol"];

    KTree unit = config["Quantity[0]/Unit"][1];
    string symbol = unit["Symbol"];
    float factor = unit["Factor"];

    cout << name << ": ";
    cout << "1.0 " << symbol << " = ";
    cout << factor << " " << standard_unit << endl;

    return 0;
}
