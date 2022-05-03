// KJsonTreeFormat.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

//  Restrictions
//  ============
//  From JSON:
//      *) empty dictionaries {} are ignored
//  To JSON:
//      *) if a node has both value and children, the value is dropped
//      *) if a node has both value and attributes, the value is dropped
//  Extension to Standard JSON:
//      *) NaN and Infinity are added
//      *) integer numbers are distinct from real numbers

#include <string>
#include <iostream>
#include <fstream>
#include "KTreeSerializer.h"
#include "KJsonParser.h"
#include "KJsonTreeFormat.h"

using namespace std;
using namespace tabree;


KJsonTreeFormat::KJsonTreeFormat()
{
}

KJsonTreeFormat::~KJsonTreeFormat()
{
}

void KJsonTreeFormat::Read(KTree& tree, std::istream& input) 
{
    KJsonParser parser;
    try {
        parser.Parse(input, tree);
    }
    catch (KException &e) {
        throw KException() << "JSON tree file syntax error: " << e.what();
    }
}

void KJsonTreeFormat::Write(const KTree& tree, std::ostream& output) 
{
    KJsonTreeSerializer(output).Serialize(tree);
}
