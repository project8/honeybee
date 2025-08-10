// tree-walker-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <tabree/KTreeFile.h>
#include <tabree/KTreeWalker.h>

using namespace std;
using namespace tabree;


class TStandardSymbolSelector: public KTreeHandler {
  public:
    TStandardSymbolSelector(void) {}
    virtual ~TStandardSymbolSelector() {}
    virtual bool StartTree(KTree& node);
    virtual bool StartNode(const std::string& name, KTree& node, KTree& attributeList);
};



bool TStandardSymbolSelector::StartTree(KTree& node)
{
    cout << "## List of Standard Units ##" << endl;

    return true;
}

bool TStandardSymbolSelector::StartNode(const std::string& name, KTree& node, KTree& attributeList)
{
    if (name != "Unit") {
        return true;  // skip this node ("return"), but go deeper ("true")
    }
    if (attributeList["Standard"].As<string>() != "SI") {
        return false;  // skip this node and all children
    }

    // this node is for a SI standard unit //
    // get name from the parent node, get symbol from a child node //

    cout << node["../Name"] << ": " << node["Symbol"] << endl;

    return false;   // no need to go deeper
}



int main(int argc, char** argv)
{
    KTree config;
    try {
        KTreeFile("UnitTable.ktf").Read(config);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    TStandardSymbolSelector selector;
    KTreeWalker(&selector).Process(config);

    return 0;
}
