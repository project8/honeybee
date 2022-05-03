// KTree.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include "KVariant.h"
#include "KTree.h"

using namespace std;
using namespace tabree;


KTree::KTree()
{
    fParentNode = nullptr;
    fReferenceCount = 0;
    fIsArrayContainer = false;
}

KTree::KTree(const KTree& Tree)
{
    fParentNode = nullptr;
    fReferenceCount = 0;
    fIsArrayContainer = false;

    Import(&Tree);
}

KTree& KTree::operator=(const KTree& Tree)
{
    if (&Tree != this) {
        Import(&Tree);
    }

    return *this;
}

KTree& KTree::operator=(const KTree::KEmptyArray& EmptyArray)
{
    Clear();
    fIsArrayContainer = true;

    return *this;
}

KTree::~KTree()
{
    if (fReferenceCount > 0) {
        cerr << "### ERROR ###: deleting referenced tree: ";
        cerr << "  reference count = " << fReferenceCount << endl;
    }

    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        fChildNodeList[i]->fReferenceCount--;
        if (fChildNodeList[i]->fReferenceCount <= 0) {
            delete fChildNodeList[i];
        }
    }
}

KTree& KTree::AppendNode(const std::string& Key)
{
    auto* ChildNode = new KTree();
    Insert(Key, ChildNode);

    return *ChildNode;
}

KTree& KTree::AppendPathNode(const std::string& Path)
{
    KTree& node = Find(Path);
    if (node.IsEmpty()) {
        return node;
    }
    if (node.IsArray()) {
        return node[node.Length()];
    }

    KTree* parent = node.ParentNode();
    if (! parent) {
        return node;
    }
    else {
        return parent->AppendNode(node.NodeName());
    }
}

void KTree::Link(KTree* Node, const std::string& Key)
{
    KTree* OldParentNode = Node->fParentNode;
    Insert(Key.empty() ? Node->NodeName() : Key, Node);
    Node->fParentNode = OldParentNode;
}

void KTree::Unlink(KTree* Node)
{
    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        if (fChildNodeList[i] == Node) {
            Node->fReferenceCount--;
            if (Node->fReferenceCount <= 0) {
                delete Node;
            }
            else if (Node->fParentNode == this) {
                Node->fParentNode = nullptr;
            }
            fChildNodeList.erase(fChildNodeList.begin() + i);
            fChildNodeKeyList.erase(fChildNodeKeyList.begin() + i);
            fChildNodeKeyIndexTable.clear();
            for (unsigned j = 0; j < fChildNodeList.size(); j++) {
                fChildNodeKeyIndexTable[fChildNodeKeyList[j]] = j;
            }
        }
        else if (fChildNodeList[i]->fIsArrayContainer) {
            fChildNodeList[i]->Unlink(Node);
        }
    }
}

KTree* KTree::ParentNode()
{
    if (fParentNode == nullptr) {
        return nullptr;
    }
    if (fParentNode->fIsArrayContainer) {
        return fParentNode->ParentNode();
    }

    return fParentNode;
}

const KTree* KTree::ParentNode() const
{
    if (fParentNode == nullptr) {
        return nullptr;
    }
    if (fParentNode->fIsArrayContainer) {
        return fParentNode->ParentNode();
    }

    return fParentNode;
}

void KTree::Insert(const std::string& Key, KTree* ChildNode)
{
    // if there are multiple entries with a same key, make the node an array //
    if ((! Key.empty()) && (fChildNodeKeyIndexTable.count(Key) > 0)) {
        unsigned Index = fChildNodeKeyIndexTable[Key];
        KTree* Node = fChildNodeList[Index];

        // if the node is not an array, insert an array node //
        if (! Node->fIsArrayContainer) {
            auto* ArrayNode = new KTree();
            fChildNodeList[Index] = ArrayNode;
            ArrayNode->fParentNode = this;
            ArrayNode->fReferenceCount = 1;
            ArrayNode->fIsArrayContainer = true;
            ArrayNode->fChildNodeList.push_back(Node);
            ArrayNode->fChildNodeKeyList.push_back("");

            Node->fParentNode = ArrayNode;
            Node = ArrayNode;
        }

        return Node->Insert("", ChildNode);
    }

    unsigned Index = fChildNodeList.size();
    fChildNodeList.push_back(ChildNode);
    fChildNodeKeyList.push_back(Key);
    fChildNodeKeyIndexTable[Key] = Index;

    ChildNode->fParentNode = this;
    ChildNode->fReferenceCount++;
}

KTree& KTree::Find(const std::string& Path) 
{
    if (Path.empty()) {
        return *this;
    }
    if (Path[0] == '/') {
        return Find(Path.substr(1));
    }

    if (Path == ".") {
        return *this;
    }
    if (Path == "..") {
        KTree* parent = ParentNode();
        if (! parent) {
            throw KException() << "invalid tree path: " << Path;
        }
        return *parent;
    }

    if (Path[0] == '[') {
        string::size_type End = Path.find_first_of(']');
        if (End == string::npos) {
            throw KException() << "invalid tree path: " << Path;
        }

        istringstream is(Path.substr(1, End-1));
        unsigned Index;
        if (!(is >> Index)) {
            throw KException() << "invalid tree path: " << Path;
        }
        string Excess;
        if (is >> Excess) {
            throw KException() << "invalid tree path: " << Path;
        }
        return FindArrayElement(Index).Find(Path.substr(End+1));
    }

    if (fIsArrayContainer) {
        // key access to array: assume first array element //
        if (fChildNodeList.empty()) {
            AppendNode("");
        }
        return fChildNodeList[0]->Find(Path);
    }

    string::size_type End;
    if (Path[0] == '@') {
        if (Path.size() > 1) {
            End = Path.substr(1).find_first_of("/[@");
            if (End != string::npos) {
                End += 1;  // for substr(1)
            }
        }
        else {
            End = 1;
        }
    }
    else {
        End = Path.find_first_of("/[@");
    }
    if (End != string::npos) {
        return Find(Path.substr(0, End)).Find(Path.substr(End));
    }
        
    std::map<std::string, unsigned>::const_iterator Entry = (
        fChildNodeKeyIndexTable.find(Path)
    );
    if (Entry == fChildNodeKeyIndexTable.end()) {
        return AppendNode(Path);
    }
    else {
        return *fChildNodeList[Entry->second];
    }
}

const KTree& KTree::Find(const std::string& Path) const 
{
    // const version //
    //... TODO: avoid duplication to the non-const version ...//

    static KTree voidNode;
    
    if (Path.empty()) {
        return *this;
    }
    if (Path[0] == '/') {
        return Find(Path.substr(1));
    }

    if (Path == ".") {
        return *this;
    }
    if (Path == "..") {
        const KTree* parent = ParentNode();
        if (! parent) {
            throw KException() << "invalid tree path: " << Path;
        }
        return *parent;
    }

    if (Path[0] == '[') {
        string::size_type End = Path.find_first_of(']');
        if (End == string::npos) {
            throw KException() << "invalid tree path: " << Path;
        }

        istringstream is(Path.substr(1, End-1));
        unsigned Index;
        if (!(is >> Index)) {
            throw KException() << "invalid tree path: " << Path;
        }
        string Excess;
        if (is >> Excess) {
            throw KException() << "invalid tree path: " << Path;
        }
        return FindArrayElement(Index).Find(Path.substr(End+1));
    }

    if (fIsArrayContainer) {
        // key access to array: assume first array element //
        if (fChildNodeList.empty()) {
            voidNode.SetBaseNodeName(this->NodeName() + "[0]");
            return voidNode;
        }
        return fChildNodeList[0]->Find(Path);
    }

    string::size_type End;
    if (Path[0] == '@') {
        if (Path.size() > 1) {
            End = Path.substr(1).find_first_of("/[@");
            if (End != string::npos) {
                End += 1;  // for substr(1)
            }
        }
        else {
            End = 1;
        }
    }
    else {
        End = Path.find_first_of("/[@");
    }
    if (End != string::npos) {
        return Find(Path.substr(0, End)).Find(Path.substr(End));
    }
        
    auto Entry = (
        fChildNodeKeyIndexTable.find(Path)
    );
    if (Entry == fChildNodeKeyIndexTable.end()) {
        voidNode.SetBaseNodeName(this->NodeName() + Path);
        return voidNode;
    }
    else {
        return *fChildNodeList[Entry->second];
    }
}

KTree& KTree::FindArrayElement(unsigned ArrayIndex) 
{
    // if children are in an array, returns Index-th child element
    // if not, if Index == 0, returns this, else creates children array

    if (! fIsArrayContainer) {
        if(ArrayIndex == 0) {
            return *this;
        }

        auto* ElementNode = new KTree(*this);
        ElementNode->fParentNode = this;
        ElementNode->fReferenceCount = 1;
        ElementNode->fIsArrayContainer = false;

        fChildNodeList.clear();
        fChildNodeKeyList.clear();
        fChildNodeKeyIndexTable.clear();
        fValue = KVariant();

        fChildNodeList.push_back(ElementNode);
        fChildNodeKeyList.push_back("");
        fIsArrayContainer = true;
    }

    while (ArrayIndex >= fChildNodeList.size()) {
        AppendNode("");
    }

    return *fChildNodeList[ArrayIndex];
}

const KTree& KTree::FindArrayElement(unsigned ArrayIndex) const 
{
    // if children are in an array, returns Index-th child element
    // if not, if Index == 0, returns this, else creates children array

    static KTree voidNode;

    if (! fIsArrayContainer) {
        if(ArrayIndex == 0) {
            return *this;
        }
        else {
            voidNode.SetBaseNodeName(
                this->NodeName() + "[" + KVariant(ArrayIndex).As<string>() + "]"
            );
            return voidNode;
        }
    }

    if (ArrayIndex >= fChildNodeList.size()) {
        voidNode.SetBaseNodeName(
            this->NodeName() + "[" + KVariant(ArrayIndex).As<string>() + "]"
        );
        return voidNode;
    }

    return *fChildNodeList[ArrayIndex];
}

void KTree::SetBaseNodeName(const std::string& name)
{
    fBaseNodeName = name;
}

std::string KTree::NodeName() const
{
    if (! fParentNode) {
        return fBaseNodeName;
    }

    if (fParentNode->fIsArrayContainer) {
        return fParentNode->NodeName();
    }

    for (unsigned i = 0; i < fParentNode->fChildNodeList.size(); i++) {
        if (fParentNode->fChildNodeList[i] == this) {
            return fParentNode->fChildNodeKeyList[i];
        }
    }

    return fBaseNodeName;
}

std::string KTree::NodePath() const
{
    if (fParentNode) {
        return fParentNode->NodePathOf(this);
    }
    else if (! fBaseNodeName.empty()) {
        return fBaseNodeName;
    }
    else {
        return "/";
    }
}

std::string KTree::NodePathOf(const KTree* ChildNode) const
{
    string NodeName;
    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        if (fChildNodeList[i] == ChildNode) {
            string Key = fChildNodeKeyList[i];
            if (! Key.empty()) {
                if (Key[0] == '@') {
                    NodeName = Key;
                }
                else {
                    NodeName = "/" + Key;
                }
            }
            else {
                ostringstream ArrayIndex;
                ArrayIndex << "[" << i << "]";
                NodeName = ArrayIndex.str();
            }
        }
    }
    if (NodeName.empty()) {
        return "";
    }

    if (fParentNode) {
        return fParentNode->NodePathOf(this) + NodeName;
    }
    else if (! fBaseNodeName.empty()) {
        return fBaseNodeName + "/" + NodeName;
    }
    else {
        return NodeName;
    }
}

void KTree::Clear()
{
    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        fChildNodeList[i]->fReferenceCount--;
        if (fChildNodeList[i]->fReferenceCount <= 0) {
            delete fChildNodeList[i];
        }
    }
    fChildNodeList.clear();
    fChildNodeKeyList.clear();
    fChildNodeKeyIndexTable.clear();
    
    fValue = KVariant();
    fIsArrayContainer = false;
}

void KTree::Import(const KTree* Tree)
{
    Clear();

    for (unsigned i = 0; i < Tree->fChildNodeList.size(); i++) {
        auto* Child = new KTree();
        Child->fParentNode = this;
        Child->fReferenceCount = 1;
        Child->Import(Tree->fChildNodeList[i]);
        fChildNodeList.push_back(Child);
        fChildNodeKeyList.push_back(Tree->fChildNodeKeyList[i]);
        fChildNodeKeyIndexTable[Tree->fChildNodeKeyList[i]] = i;
    }

    fValue = Tree->fValue;
    fIsArrayContainer = Tree->fIsArrayContainer;
}

void KTree::Dump(std::ostream& os) const 
{
    if (! fValue.IsVoid()) {
        os << NodePath() << ": ";
#if 1
        if (fValue.IsVoid()) {
            os << "(void) ";
        }
        else if (fValue.IsBool()) {
            os << "(bool) ";
        }
        else if (fValue.IsInteger()) {
            os << "(integer) ";
        }
        else if (fValue.IsNumeric()) {
            os << "(real) ";
        }
        else if (fValue.IsString()) {
            os << "(string) ";
        }
#endif
        if (fValue.IsString()) {
            string Str = fValue.AsString();
            if (Str.empty()) {
                os << "\"\"";
            }
            else if (isspace(*Str.begin()) || isspace(*Str.end())) {
                os << '"' << Str << '"';
            }
            else {
                os << Str;
            }
        }
        else {
            os << fValue;
        }
        os << endl;
    }

    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        fChildNodeList[i]->Dump(os);
    }
}


// depreciated //
std::vector<std::string> KTree::NodePathList() const
{
    vector<string> List;
    if ((! fValue.IsVoid()) && ! (fValue.As<std::string>().empty())) {
        List.push_back("");
    }
    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        string Key = fChildNodeKeyList[i];
        string NodeName;
        if (! Key.empty()) {
            NodeName = Key;
        }
        else {
            ostringstream ArrayPath;
            ArrayPath << "[" << i << "]";
            NodeName = ArrayPath.str();
        }
        vector<string> ChildList = fChildNodeList[i]->NodePathList();
        for (unsigned j = 0; j < ChildList.size(); j++) {
            if (ChildList[j].empty()) {
                List.push_back(NodeName);
            }
            else {
                if (ChildList[j][0] == '[') {
                    List.push_back(NodeName + ChildList[j]);
                }
                else {
                    List.push_back(NodeName + "/" + ChildList[j]);
                }
            }
        }
    }

    return List;
}
