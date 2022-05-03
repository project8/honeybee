// KTreeFile.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTreeFile_h__
#define KTreeFile_h__

#include <string>
#include "KTree.h"
#include "KTreeFormat.h"

#include "KKtfTreeFormat.h"
#include "KJsonTreeFormat.h"
#include "KInifileTreeFormat.h"
#include "KXpvpTreeFormat.h"


namespace tabree {


/**
 * \brief Convenient Tree File I/O (thin wrapper to KTreeFormat), with file type recognition. A special file name of "|", with a format extension (e.g. "|.ktf") is used for stdin/stdout.
 */
class KTreeFile {
  public:
    KTreeFile(const std::string& fileName);
    virtual ~KTreeFile();
    virtual void Read(KTree& tree) ;
    virtual void Write(const KTree& tree) ;
  protected:
    std::string fFileName;
    KTreeFormat* fFormat;
};


}
#endif
