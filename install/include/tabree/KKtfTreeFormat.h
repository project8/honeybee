// KKtfTreeFormat.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KKtfTreeFormat_h__
#define KKtfTreeFormat_h__

#include <string>
#include <iostream>
#include "KTreeFormat.h"


namespace tabree {

/**
 * \brief Tree I/O in KTF format
 */
class KKtfTreeFormat: public KTreeFormat {
  public:
    KKtfTreeFormat();
    ~KKtfTreeFormat() override;
    virtual void SetHeadingChar(char Char);
    virtual void SetIndentChar(char Char);
    void Read(KTree& tree, std::istream& input) override ;
    void Write(const KTree& tree, std::ostream& output) override ;
  protected:
    char fHeadingChar;
    char fIndentChar;
};

}
#endif
