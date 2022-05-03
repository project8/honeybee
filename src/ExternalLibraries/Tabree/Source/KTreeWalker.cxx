// KTreeWalker.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include "KTree.h"
#include "KTreeWalker.h"

using namespace std;
using namespace tabree;


KTreeWalker::KTreeWalker(KTreeHandler* Handler)
{
    fHandler = Handler;
}

KTreeWalker::~KTreeWalker()
{
}

void KTreeWalker::Process(KTree& Tree, std::string RootNodeName) 
{
    if (! fHandler) {
        return;
    }

    if (RootNodeName.empty()) {
        RootNodeName = Tree.NodeName();
    }

    if (fHandler->StartTree(Tree)) {
        ProcessNode(RootNodeName, Tree);
    }
    fHandler->EndTree(Tree);
}

void KTreeWalker::ProcessNode(const std::string& Name, KTree& Node) 
{
    if (Node.IsArray()) {
        if (! fHandler->StartArray(Name, Node)) {
            return;
        }
    }

    for (unsigned i = 0; i < Node.Length(); i++) {
        KTree& Element = Node[i];
        KTree AttributeList;
        vector<string> KeyList;
        for (unsigned j = 0; j < Element.KeyList().size(); j++) {
            const string& Key = Element.KeyList()[j];
            if (Key.empty()) {
                // array of array; not supported in XML Tree model //
            }
            else if (Key[0] == '@') {
                AttributeList[Key.substr(1)] = Element[Key];
            }
            else {
                KeyList.push_back(Key);
            }
        }
        if (fHandler->StartNode(Name, Element, AttributeList)) {
            for (unsigned j = 0; j < KeyList.size(); j++) {
                KTree& ChildNode = Element[KeyList[j]];
                if (! ChildNode.IsEmpty()) {
                    ProcessNode(KeyList[j], ChildNode);
                }
            }
            fHandler->EndNode(Name, Element);
        }
    }

    if (Node.IsArray()) {
        fHandler->EndArray(Name, Node);
    }
}
