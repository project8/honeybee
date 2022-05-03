// KXpvpTreeFormat.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KXpvpTreeFormat_h__
#define KXpvpTreeFormat_h__

#include <string>
#include <iostream>
#include "KTreeFormat.h"


namespace tabree {


/**
 * \brief Tree I/O in XPath-Value Pair format (original)
 */
class KXpvpTreeFormat: public KTreeFormat {
  public:
    KXpvpTreeFormat();
    ~KXpvpTreeFormat() override;
    void Read(KTree& tree, std::istream& input) override ;
    void Write(const KTree& tree, std::ostream& output) override ;
};


}
#endif
