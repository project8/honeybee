// KTabreeFile.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTabreeFile_h__
#define KTabreeFile_h__

#include <string>
#include "KTabree.h"
#include "KTabreeFormat.h"

namespace tabree {


/**
 * \brief Convenient Tabree File I/O (thin wrapper to KTabreeFormat)
 */
class KTabreeFile {
  public:
    KTabreeFile(const std::string& fileName);
    virtual ~KTabreeFile();
    virtual void Read(KTabree& tabree) ;
    virtual void Write(KTabree& tabree) ;
  protected:
    std::string fFileName;
    KTabreeFormat* fFormat;
};


/**
 * \brief Convenient Tabree File I/O for CSV format
 */
class KCsvTabreeFile: public KTabreeFile {
  public:
    KCsvTabreeFile(const std::string& fileName);
};



/**
 * \brief Convenient interface to Tabree-Embedded (thin wrapper to KTabreeFormat)
 */
class KTabreeEmbedded {
  public:
    KTabreeEmbedded(const char* embeddedTabree);
    virtual ~KTabreeEmbedded();
    virtual void Read(KTabree& tabree) ;
  protected:
    const char* fEmbeddedTabree;
};

}
#endif
