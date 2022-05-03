// KTreeBuilder.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include "KTree.h"
#include "KJsonParser.h"
#include "KTreeBuilder.h"


using namespace std;
using namespace tabree;



KTreeBuilder::KTreeBuilder(KTree* Tree)
{
    fIsInlineJsonEnabled = true;

    fCurrentDepth = -1;
    fLastNode = nullptr;
    fCurrentParentStack.push_back(Tree);
}

KTreeBuilder::~KTreeBuilder()
{
}

void KTreeBuilder::DisableInlineJson()
{
    fIsInlineJsonEnabled = false;
}

void KTreeBuilder::AddNode(int Depth, const std::string& Key, const std::string& Value) 
{
    if (fCurrentDepth < 0) {
        // initial depth, can be non-zero
        fCurrentDepth = Depth;
    }

    if (Depth > fCurrentDepth) {
        fDepthStepStack.push_back(Depth - fCurrentDepth);
        fCurrentDepth = Depth;
        fCurrentParentStack.push_back(fLastNode);
    }
    else if (Depth < fCurrentDepth) {
	    while (fCurrentDepth > max(Depth, 0)) {
            fCurrentDepth -= fDepthStepStack.back();
            fDepthStepStack.pop_back(); 
            fCurrentParentStack.pop_back();
	    }
	    if (Depth != fCurrentDepth) {
	        throw KException() << "bad indent structure";
	    }
    }

    // make empty-string void-value //
    KVariant VariantValue;
    if (! Value.empty()) {
        VariantValue = Value;
    }
    fLastNode = &fCurrentParentStack.back()->AppendNode(Key);

    if ((! Value.empty()) && fIsInlineJsonEnabled) {
        istringstream Line(Value.c_str());
        KTree JsonTree;
        KJsonParser JsonParser;
        try {
            JsonParser.Parse(Line, JsonTree);
            *fLastNode = JsonTree;
        }
        catch (KException &e) {
            if (
                ((*Value.begin() == '{') && (*Value.rbegin() == '}')) ||
                ((*Value.begin() == '[') && (*Value.rbegin() == ']')) 
            ){
                cerr << "WARNING: invalid JSON syntax: " << e.what() << ": " << Value << endl;
            }
            fLastNode->Value() = VariantValue;
        }
    }
    else {
        fLastNode->Value() = VariantValue;
    }
}

void KTreeBuilder::AppendLine(const std::string& Line) 
{
    if (fLastNode) {
        fLastNode->Value() = fLastNode->As<string>() + '\n' + Line;
    }
}
