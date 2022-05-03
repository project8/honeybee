// KKtfTreeFormat.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <fstream>
#include "KTabree.h"
#include "KTreeSerializer.h"
#include "KKtfTreeFormat.h"

using namespace std;
using namespace tabree;


KKtfTreeFormat::KKtfTreeFormat()
{
    fHeadingChar = '#';
    fIndentChar = '\0';
}

KKtfTreeFormat::~KKtfTreeFormat()
{
}

void KKtfTreeFormat::SetHeadingChar(char Char)
{
    fHeadingChar = Char;
}

void KKtfTreeFormat::SetIndentChar(char Char)
{
    fIndentChar = Char;
}

void KKtfTreeFormat::Read(KTree& tree, std::istream& input) 
{
    KTabreeFormat format;
    format.SetCommentHeader(fHeadingChar);
    format.SetTreeIndent(fIndentChar);

    KTabree tabree;
    try {
	format.Read(tabree, input);
    }
    catch (KException &e) {
        throw KException() << "KTF tree file syntax error: " << e.what();
    }

    tree["/"] = tabree.Tree();
}

void KKtfTreeFormat::Write(const KTree& tree, std::ostream& output) 
{
    KKtfTreeSerializer(output).Serialize(tree);
}
