// KInifileTreeFormat.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KInifileTreeFormat_h__
#define KInifileTreeFormat_h__

#include <string>
#include <iostream>
#include "KTreeFormat.h"


namespace tabree {


/**
 * \brief Tree I/O in Ini-file format (extended)
 */
class KInifileTreeFormat: public KTreeFormat {
  public:
    KInifileTreeFormat();
    ~KInifileTreeFormat() override;
    void Read(KTree& tree, std::istream& input) override ;
    void Write(const KTree& tree, std::ostream& output) override ;
    virtual void SetBreakChar(char Char);
  protected:
    char fBreakChar;
};

}
#endif
