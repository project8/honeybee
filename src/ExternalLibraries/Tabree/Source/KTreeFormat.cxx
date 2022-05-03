// KTreeFormat.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <iostream>
#include "KJsonParser.h"
#include "KTreeFormat.h"

using namespace std;
using namespace tabree;


KTreeFormat::KTreeFormat()
{
}

KTreeFormat::~KTreeFormat()
{
}

void KTreeFormat::FillNodeValue(KTree& tree, const std::string& text, bool isJsonEnabled)
{
    bool isJson = (
        ((*text.begin() == '{') && (*text.rbegin() == '}')) ||
        ((*text.begin() == '[') && (*text.rbegin() == ']')) 
    );
    if (isJson && ! isJsonEnabled) {
        tree.Value() = text;
        return;
    }
    
    try {
        istringstream line(text.c_str());
        KJsonParser().Parse(line, tree);
    }
    catch (KException &e) {
        if (isJson) {
            cerr << "WARNING: invalid JSON syntax: " << e.what() << ": " << text << endl;
        }
        tree = KTree();
        tree.Value() = text;
    }
}
