// KTreeBuilder.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef KTreeBuilder_h__
#define KTreeBuilder_h__


#include <iostream>
#include <string>
#include <deque>
#include "KTree.h"


namespace tabree {


class KTreeBuilder {
  public:
    KTreeBuilder(KTree* Tree);
    virtual ~KTreeBuilder();
    virtual void DisableInlineJson();
    virtual void AddNode(int Depth, const std::string& Key, const std::string& Value) ;
    virtual void AppendLine(const std::string& Line) ;
  protected:
    bool fIsInlineJsonEnabled;
  private:
    int fCurrentDepth;
    KTree* fLastNode;
    std::deque<KTree*> fCurrentParentStack;
    std::deque<int> fDepthStepStack;
};


}
#endif
