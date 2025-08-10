// KTreeFormat.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTreeFormat_h__
#define KTreeFormat_h__

#include <iostream>
#include "KTree.h"


namespace tabree {


/**
 * \brief Tree I/O format, abstract for various formats
 */
class KTreeFormat {
  public:
    KTreeFormat();
    virtual ~KTreeFormat();
    virtual void Read(KTree& tree, std::istream& input)  = 0;
    virtual void Write(const KTree& tree, std::ostream& output)  = 0;
  protected:
    virtual void FillNodeValue(KTree& tree, const std::string& text, bool isJsonEnabled=true);
};


}
#endif
