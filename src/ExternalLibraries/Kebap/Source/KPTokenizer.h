// KPTokenizer.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPTokenizer_h__
#define __KPTokenizer_h__

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "KPToken.h"
#include "KPTokenTable.h"


namespace kebap {

class KPInputBuffer {
  public:
    explicit KPInputBuffer(std::istream& InputStream);
    virtual ~KPInputBuffer();
    virtual bool GetNext(char& Character);
    virtual void Unget(char Character);
    virtual long LineNumber();
    virtual void SetChildInput(std::istream& InputStream);
    virtual void SetChildInputBuffer(KPInputBuffer* InputBuffer);
    virtual bool AbortChildInput();
  protected:
    std::istream& fInputStream;
    std::stack<char, std::vector<char> > fUngetStack;
    long fLineNumber;
    KPInputBuffer* fChildInputBuffer;
};

    
class KPTokenizer {
  public:
    KPTokenizer(std::istream& SourceStream, const KPTokenTable* TokenTable);
    KPTokenizer(KPInputBuffer* InputBuffer, const KPTokenTable* TokenTable);
    virtual ~KPTokenizer();
    virtual KPToken Next();
    virtual void Unget(KPToken &Token);
    virtual KPToken LookAhead(int n = 1);
    virtual KPToken GetLine(char Terminator = '\n');
    virtual bool GetChar(char& Character);
    virtual void UngetChar(char Character);
    virtual KPTokenizer& SkipWhiteSpace();
    virtual KPInputBuffer* InputBuffer();
    virtual long LineNumber() const;
    virtual void SetLineNumber(int LineNumber);
    virtual void SetTokenTable(const KPTokenTable* TokenTable);
    virtual void SetCommentSkipping(bool IsEnabled);
    virtual void SetWhiteSpaceSkipping(bool IsEnabled);
    virtual void SetEscapeSequenceProcessing(bool IsEnabled);
  protected:
    virtual KPToken ParseNext();
    virtual bool ConvertEscape(char& Character) const;
    virtual void ParseIdentifier(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseNumber(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseHexNumber(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseBinNumber(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseFloating(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseFloatingExponent(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseOperator(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseQuote(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseComment(std::string& TokenString, KPToken::TTokenType& TokenType);
    virtual void ParseWhiteSpace(std::string& TokenString, KPToken::TTokenType& TokenType);
  protected:
    KPInputBuffer *fInputBuffer, *fMyInputBuffer;
    std::stack<KPToken, std::vector<KPToken> > fUngetStack;
    const KPTokenTable* fTokenTable;
    bool fIsCommentSkippingEnabled;
    bool fIsWhiteSpaceSkippingEnabled;
    bool fIsEscapeSequenceProcessingEnabled;
    int fLineNumberOffset;
};


}
#endif
