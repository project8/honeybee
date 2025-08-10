// KPToken.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPToken_h__
#define __KPToken_h__

#include <string>
#include <complex>
#include "KPException.h"


namespace kebap {

class KPToken {
  public:
    enum TTokenType {
        TokenType_Keyword,
        TokenType_Identifier,
        TokenType_Bool,
        TokenType_Integer, 
        TokenType_Floating, 
        TokenType_Complex, 
        TokenType_Separator,
        TokenType_Operator,
        TokenType_Quote, 
	TokenType_Comment,
	TokenType_WhiteSpace,
        TokenType_Empty, 
        TokenType_Unknown
    };
    enum TNumberSuffixFlag {
	NumberSuffix_Unsigned = 0x01,
	NumberSuffix_Long = 0x02,
	NumberSuffix_Imaginary = 0x04
    };
  public:
    KPToken();
    KPToken(const std::string& TokenString, TTokenType Type, unsigned NumberSuffixFlags, long LineNumber);
    KPToken(const KPToken& Token);
    virtual ~KPToken();
    virtual KPToken& operator=(const KPToken& Token);
    virtual bool IsKeyword() const;
    virtual bool IsIdentifier() const;
    virtual bool IsBool() const;
    virtual bool IsInteger() const;
    virtual bool IsFloating() const;
    virtual bool IsComplex() const;
    virtual bool IsSeparator() const;
    virtual bool IsOperator() const;
    virtual bool IsQuote() const;
    virtual bool IsEmpty() const;
    virtual bool IsComment() const;
    virtual bool IsWhiteSpace() const;
    virtual bool Is(const std::string& String) const;
    virtual bool IsNot(const std::string& String) const;
    virtual std::string AsString() const;
    virtual bool AsBool() const ;
    virtual long AsLong() const ;
    virtual double AsDouble() const ;
    virtual std::complex<double> AsComplex() const ;
    virtual KPToken& RemoveQuotation(char Quoter = '\0');
    virtual KPToken& MustBe(const std::string& ExpectedString) ;
    virtual KPToken& MustBe(TTokenType ExpectedTokenType) ;
    virtual void ThrowUnexpected(const std::string& Expected = "") ;
    virtual std::string Position() const;
  protected:
    std::string fTokenString;
    TTokenType fType;
    unsigned fNumberSuffixFlags;
    long fLineNumber;
};


}
#endif
