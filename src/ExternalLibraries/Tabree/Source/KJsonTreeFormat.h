// KJsonTreeFormat.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KJsonTreeFormat_h__
#define KJsonTreeFormat_h__

#include <string>
#include <iostream>
#include "KTreeFormat.h"


namespace tabree {


/**
 * \brief Tree I/O in JSON format
 */
class KJsonTreeFormat: public KTreeFormat {
  public:
    KJsonTreeFormat();
    ~KJsonTreeFormat() override;
    void Read(KTree& tree, std::istream& input) override ;
    void Write(const KTree& tree, std::ostream& output) override ;
};


}
#endif
