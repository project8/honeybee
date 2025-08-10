// KPTokenTable.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPTokenTable_h__
#define __KPTokenTable_h__

#include <string>
#include <set>
#include <map>


namespace kebap {

class KPTokenTable {
  public:
    KPTokenTable();
    virtual ~KPTokenTable();
    virtual void Merge(KPTokenTable* Source);
    virtual void AddKeyword(const std::string& Keyword);
    virtual void AddOperator(const std::string& Operator);    
    virtual void AddSeparator(const std::string& Separator);
    virtual void AddAlphabet(char Alphabet);
    virtual void AddFollowerAlphabet(char Alphabet);
    virtual void AddCommentLimiter(const std::string& Limiter, const std::string& Delimiter);
    virtual bool IsKeyword(const std::string& TokenString) const;
    virtual bool IsOperator(const std::string& TokenString) const;
    virtual bool IsSeparator(const std::string& TokenString) const;
    virtual bool IsAlphabet(const char& Character) const;
    virtual bool IsFollowerAlphabet(const char& Character) const;
    virtual bool IsCommentLimiter(const std::string& TokenString) const;
    virtual std::string CommentDelimiterFor(const std::string& CommentLimiter) const;
  protected:
    std::set<std::string> fKeywordSet;
    std::set<std::string> fOperatorSet;
    std::set<std::string> fSeparatorSet;
    std::set<char> fAlphabetSet;
    std::set<char> fFollowerAlphabetSet;
    std::map<std::string, std::string> fCommentLimiterTable;
};


class KPCxxTokenTable: public KPTokenTable {
  public:
    KPCxxTokenTable();
    ~KPCxxTokenTable() override;
};


}
#endif
