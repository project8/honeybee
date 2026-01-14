// tree-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <time.h>
#include <tabree/KTreeFile.h>

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    KTree tree;

    // simple key-value pair
    tree["Title"] = "Test Tree";

    // more structured
    tree["MetaInformation/Creator"] = "tree-test.cxx";

    // multi-dimension array style, variant values
    tree["MetaInformation"]["Timestamp"] = (long) time(NULL);

    // sub-tree, XML-style attributes
    KTree& date = tree["MetaInformation/CreationDate"];
    date["Date"] = "04 Nov 2012 16:34:36";
    date["Date@TimeZone"] = "CET";

    // arrays
    KTree& fibonacci = tree["Fibonacci"];
    fibonacci[0] = 1;
    fibonacci[1] = 1;
    for (int i = 2; i < 8; i++) {
        fibonacci[i] = (int) fibonacci[i-1] + (int) fibonacci[i-2];
    }
        
    // writing in various formats //
    //KKtfTreeSerializer(cout).Serialize(tree);
    KXmlTreeSerializer(cout).Serialize(tree);
    //KJsonTreeSerializer(cout).Serialize(tree);

    // for reading, use KTreeFile::Read()
#if 0
    KTree tree;
    try {
        KTreeFile(fileName).Read(tree);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
    tree.Dump();
#endif

    return 0;
}
