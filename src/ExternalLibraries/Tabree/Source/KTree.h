// KTree.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTree_h__
#define KTree_h__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "KVariant.h"


namespace tabree {


/**
 * \brief Tree data structure
 */
class KTree {
  public:
    class KEmptyArray { public: virtual ~KEmptyArray(){} };
  public:
    KTree();
    KTree(const KTree& Tree);
    virtual KTree& operator=(const KTree& Tree);
    virtual KTree& operator=(const KEmptyArray& EmptyArray);
    virtual ~KTree();
  public:
    // node/leaf access //
    virtual std::string NodeName() const;
    virtual std::string NodePath() const;
    inline KVariant& Value();
    inline const KVariant& Value() const;
    template<typename T> inline operator T() const ;
    template<typename T> inline T As() const ;
    inline KTree& operator=(const KVariant& NewValue);
    inline bool IsVoid() const;
    inline bool IsLeaf() const;
    inline bool IsArray() const;
    inline bool IsEmpty() const;
    inline KVariant Or(const KVariant& DefaultValue) const;
  public:
    // node iteration //
    inline size_t Length() const;
    inline KTree& operator[](int Index);
    inline KTree& operator[](const std::string& Key) ;
    inline KTree& operator[](const char* Key) ;
    inline const KTree& operator[](int Index) const;
    inline const KTree& operator[](const std::string& Key) const ;
    inline const KTree& operator[](const char* Key) const ;
    inline const std::vector<std::string>& KeyList() const;
    inline const std::vector<KTree*>& ChildNodeList();
    virtual KTree* ParentNode();
    virtual const KTree* ParentNode() const;
  public:
    // node insertion //
    virtual KTree& AppendNode(const std::string& Key);
    virtual KTree& AppendPathNode(const std::string& Path);
    virtual void Link(KTree* Node, const std::string& Key = ""); // IMPORTANT: Linked node must not be a "Tree Head allocated on stack", where destruction cannot be controlled; child branches are okay
    virtual void Unlink(KTree* Node);
    inline KTree& operator()(const std::string& key, const KVariant& variant);
    inline KTree& operator()(const std::string& key, const KTree& tree);
  public:
    // misc //
    virtual void Dump(std::ostream& os = std::cout) const;
    virtual void SetBaseNodeName(const std::string& name);
  public:
    // depreciated //
    virtual std::vector<std::string> NodePathList() const;
  protected:
    virtual void Insert(const std::string& Key, KTree* Node);
    virtual KTree& Find(const std::string& Path) ;
    virtual const KTree& Find(const std::string& Path) const ;
    virtual KTree& FindArrayElement(unsigned Index) ;
    virtual const KTree& FindArrayElement(unsigned Index) const ;
    virtual std::string NodePathOf(const KTree* ChildNode) const;
    virtual void Clear();
    virtual void Import(const KTree* Tree);
  private:
    KVariant fValue;
    KTree* fParentNode;
    int fReferenceCount;
    bool fIsArrayContainer;
    std::vector<KTree*> fChildNodeList;
    std::vector<std::string> fChildNodeKeyList;
    std::map<std::string, unsigned> fChildNodeKeyIndexTable;
    std::string fBaseNodeName;
};



template<typename T> struct KTreeDecoder {
    static T As(const KTree& Tree) {
        try {
            return Tree.Value().As<T>();
        }
        catch (KException &e) {
            if (Tree.Value().IsVoid()) {
                throw KException() << "node has no value: " << Tree.NodePath();
            }
            throw e;
        }
    }
};



inline KVariant& KTree::Value()
{
    if (fIsArrayContainer && ! fChildNodeList.empty()) {
        return fChildNodeList[0]->Value();
    }

    return fValue;
}

inline const KVariant& KTree::Value() const
{
    if (fIsArrayContainer && ! fChildNodeList.empty()) {
        return fChildNodeList[0]->Value();
    }

    return fValue;
}

template<typename T> inline KTree::operator T() const 
{
    return tabree::KTreeDecoder<T>::As(*this);
}

template<typename T> inline T KTree::As() const 
{
    return tabree::KTreeDecoder<T>::As(*this);
}

inline KTree& KTree::operator=(const KVariant& NewValue)
{
    this->Value() = NewValue;
    return *this;
}

inline bool KTree::IsVoid() const
{
    return Value().IsVoid();
}

inline bool KTree::IsLeaf() const
{
    return (! fIsArrayContainer) && fChildNodeList.empty();
}

inline bool KTree::IsArray() const
{
    return fIsArrayContainer;
}

inline bool KTree::IsEmpty() const
{
    if (! Value().IsVoid()) {
        return false;
    }
    if (IsArray()) {
        // empty-array is not considered to be "empty"
        return false;
    }
    for (unsigned i = 0; i < fChildNodeList.size(); i++) {
        if (! fChildNodeList[i]->IsEmpty()) {
            return false;
        }
    }
    return true;
}

inline KVariant KTree::Or(const KVariant& DefaultValue) const
{
    return IsVoid() ? DefaultValue : Value();
}

inline size_t KTree::Length() const
{
    if (IsArray()) {
        return fChildNodeList.size();
    }
    else if (IsLeaf() && IsVoid()) {
        return 0;
    }
    else {
        return 1;
    }
}

inline KTree& KTree::operator[](int Index)
{
    // negative index is invalid; if passed, use it as "appending"
    return FindArrayElement( Index < 0 ? Length() : Index );
}

inline const KTree& KTree::operator[](int Index) const
{
    // negative index is invalid; if passed, use it as "appending"
    return FindArrayElement( Index < 0 ? Length() : Index );
}

inline KTree& KTree::operator[](const std::string& Key) 
{
    return Find(Key);
}

inline const KTree& KTree::operator[](const std::string& Key) const 
{
    return Find(Key);
}

inline KTree& KTree::operator[](const char* Key) 
{
    return Find(Key);
}

inline const KTree& KTree::operator[](const char* Key) const 
{
    return Find(Key);
}

inline const std::vector<std::string>& KTree::KeyList() const
{
    return fChildNodeKeyList;
}

inline const std::vector<KTree*>& KTree::ChildNodeList()
{
    return fChildNodeList;
}


inline std::ostream& operator<<(std::ostream&os, const KTree& Tree) 
{
    return os << Tree.Value();
}


inline KTree& KTree::operator()(const std::string& Key, const KVariant& Value)
{
    this->Find(Key).Value() = Value;
    return *this;
}

inline KTree& KTree::operator()(const std::string& Key, const KTree& Tree)
{
    this->Find(Key).Import(&Tree);
    return *this;
}

inline KTree make_tree(const std::string& Key, const KVariant& Value)
{
    return KTree()(Key, Value);
}

inline KTree make_tree(const std::string& Key, const KTree& Tree)
{
    return KTree()(Key, Tree);
}

}


// suger for convenient includes //
#include "KTreeWalker.h"
#include "KTreeSerializer.h"

#endif
