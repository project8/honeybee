// KTreeWalker.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef KTreeWalker_h__
#define KTreeWalker_h__

#include <string>
#include "KTree.h"


namespace tabree {

class KTreeHandler;


/**
 * \brief Tree node iteration, SAX-style callback with DOM-style-object parameter
 */
class KTreeWalker {
  public:
    KTreeWalker(KTreeHandler* Handler);
    virtual ~KTreeWalker();
    virtual void Process(KTree& Tree, std::string RootNodeName = "") ;
  protected:
    virtual void ProcessNode(const std::string& Name, KTree& Node) ;
  private:
    KTreeHandler* fHandler;
};


/** 
 * \brief Abstract handler for Tree node iteration (KTreeWalker), SAX-style handler with DOM-style-object parameter
 */
class KTreeHandler {
  public:
    KTreeHandler() {}
    virtual ~KTreeHandler() {}
    virtual bool StartTree(KTree& /*Node*/)  { return true; }
    virtual void EndTree(KTree& /*Node*/)  {}
    virtual bool StartNode(const std::string& /*Name*/, KTree& /*Node*/, KTree& /*AttributeList*/)  { return true; }
    virtual void EndNode(const std::string& /*Name*/, KTree& /*Node*/)  {}
    virtual bool StartArray(const std::string& /*Name*/, KTree& /*Node*/)  { return true; }
    virtual void EndArray(const std::string& /*Name*/, KTree& /*Node*/)  {}
};


}
#endif
