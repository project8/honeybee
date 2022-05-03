// KTabreeFormat.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTabreeFormat_h__
#define KTabreeFormat_h__


#include <string>
#include <map>
#include "KTree.h"
#include "KTabree.h"
#include "KTabreeFormatProcessor.h"


namespace tabree {


/**
 * \brief Tabree I/O (with KTF format only)
 */
class KTabreeFormat {
  public:
    KTabreeFormat();
    virtual ~KTabreeFormat();
    virtual void EnableCsvHeader();
    virtual void SetDelimiter(char Delimiter);
    virtual void SetQuote(char Quote);
    virtual void SetLineTerminator(char LineTerminator);
    virtual void SetCommentHeader(char CommentHeader);
    virtual void SetTreeIndent(char TreeIndent);
    virtual void SetHeaderProcessor(KTabreeFormatHeaderProcessor* HeaderProcessor);
  public:
    virtual void Read(KTabree& Tabree, std::istream& Input) ;
    virtual void Write(const KTabree& Tabree, std::ostream& Output) ;
  protected:
    bool fIsCsvHeaderEnabled;
    char fDefaultDelimiter, fDefaultQuote, fLineTerminator;
    char fCommentHeader, fTreeIndent;
    KTabreeFormatHeaderProcessor* fHeaderProcessor;
  private:
    KTabreeFormatHeaderProcessor* fDefaultHeaderProcessor;
};


}
#endif
