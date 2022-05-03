// KPToken.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <string>
#include "KPToken.h"

using namespace std;
using namespace kebap;


KPToken::KPToken()
{
    fType = TokenType_Empty;
    fNumberSuffixFlags = 0;

    fLineNumber = 0;
}

KPToken::KPToken(const string& TokenString, TTokenType Type, unsigned NumberSuffixFlags, long LineNumber)
{
    fType = Type;
    fNumberSuffixFlags = NumberSuffixFlags;
    fTokenString = TokenString;

    fLineNumber = LineNumber;
}

KPToken::KPToken(const KPToken& Token)
{
    fType = Token.fType;
    fNumberSuffixFlags = Token.fNumberSuffixFlags;
    fTokenString = Token.fTokenString;

    fLineNumber = Token.fLineNumber;
}

KPToken::~KPToken()
{
}

KPToken& KPToken::operator=(const KPToken& Token)
{
    fType = Token.fType;
    fNumberSuffixFlags = Token.fNumberSuffixFlags;
    fTokenString = Token.fTokenString;

    fLineNumber = Token.fLineNumber;

    return *this;
}

bool KPToken::IsKeyword() const
{
    return (fType == KPToken::TokenType_Keyword);
}

bool KPToken::IsIdentifier() const
{
    return (fType == KPToken::TokenType_Identifier);
}

bool KPToken::IsBool() const
{
    return (fType == KPToken::TokenType_Bool);
}

bool KPToken::IsInteger() const
{
    return (
	(fType == KPToken::TokenType_Integer) &&
	((fNumberSuffixFlags & KPToken::NumberSuffix_Imaginary) == 0)
    );
}

bool KPToken::IsFloating() const
{
    return (
	(fType == KPToken::TokenType_Floating) &&
	((fNumberSuffixFlags & KPToken::NumberSuffix_Imaginary) == 0)
    );
}

bool KPToken::IsComplex() const
{
    return (
	(
	    (fType == KPToken::TokenType_Integer) ||
	    (fType == KPToken::TokenType_Floating) 
	) &&
	(fNumberSuffixFlags & KPToken::NumberSuffix_Imaginary)
    );
}

bool KPToken::IsSeparator() const
{
    return (fType == KPToken::TokenType_Separator);
}

bool KPToken::IsOperator() const
{
    return (fType == KPToken::TokenType_Operator);
}

bool KPToken::IsQuote() const
{
    return (fType == KPToken::TokenType_Quote);
}

bool KPToken::IsEmpty() const
{
    return (fType == KPToken::TokenType_Empty);
}

bool KPToken::IsComment() const
{
    return (fType == KPToken::TokenType_Comment);    
}

bool KPToken::IsWhiteSpace() const
{
    return (fType == KPToken::TokenType_WhiteSpace);
}

bool KPToken::Is(const string& String) const
{
    return (fTokenString == String);
}

bool KPToken::IsNot(const string& String) const
{
    return (fTokenString != String);
}

string KPToken::AsString() const
{
    return fTokenString;
}

bool KPToken::AsBool() const 
{
    if (! IsBool()) {
        throw KPException() << Position() << "bool value is expected.";
    }

    return fTokenString != "false";
}

long KPToken::AsLong() const 
{
    if (fType != KPToken::TokenType_Integer) {
        throw KPException() << Position() << "integer is expected.";
    }

    long LongValue;
    if ((fTokenString.size() > 2) && (tolower(fTokenString[1]) == 'x')) {
	// Hex Number
	istringstream ValueStream(fTokenString.substr(2, string::npos));
	unsigned long HexValue;
	if (! (ValueStream >> hex >> HexValue)) {
	    throw KPException() << Position() << "bad hex integer literal";
	}
	LongValue = HexValue;
    }
    else if ((fTokenString.size() > 2) && (tolower(fTokenString[1]) == 'b')) {
	// Bin Number
	LongValue = 0;
	for (unsigned i = 0; i < fTokenString.size() - 2; i++) {
	    char Ch = fTokenString[fTokenString.size()-i-1];
	    if (Ch == '1') {
		LongValue += 0x01ul << i;
	    }
	    else if (Ch == '0') {
		;
	    }
	    else {
		throw KPException() << Position() << "bad bin integer literal";
	    }
	}
    }
    else {
	// Dec Number
	istringstream ValueStream(fTokenString);
	if (! (ValueStream >> LongValue)) {
	    throw KPException() << Position() << "bad interger literal";
	}
    }
    
    return LongValue;
}

double KPToken::AsDouble() const 
{
    double DoubleValue;
    if (fType == KPToken::TokenType_Integer) {
	// the token might be a HEX number, so use AsLong() conversion
	DoubleValue = (double) AsLong();
    }
    else if (fType == KPToken::TokenType_Floating) {
	istringstream ValueStream(fTokenString);
	if (! (ValueStream >> DoubleValue)) {
            throw KPException() << Position() << "bad floating number literal";
	}
    }    
    else {
        throw KPException() << Position() << "floating number is expected.";
    }
    
    return DoubleValue;
}

complex<double> KPToken::AsComplex() const 
{
    if (! IsComplex()) {
        throw KPException() << Position() << "complex number is expected.";
    }

    return complex<double>(0, AsDouble());
}

KPToken& KPToken::RemoveQuotation(char Quoter)
{
    if (fTokenString.size() > 1) {
	string::iterator Head = fTokenString.begin();
	string::iterator Tail = fTokenString.end() - 1;
    
	if (Quoter == '\0') {
	    Quoter = *Head;
	}

	if ((*Head == Quoter) && (*Tail == Quoter)) {
	    fTokenString.erase(fTokenString.begin());
	    fTokenString.erase(fTokenString.end() - 1);
	}
    }
    
    return *this;
}

string KPToken::Position() const
{
    if (fLineNumber == 0) {
	return string("");
    }

    ostringstream Stream;
    if (fLineNumber > 0) {
	Stream << "line " << fLineNumber << ": ";
    }

    return Stream.str();
}

KPToken& KPToken::MustBe(const string& ExpectedString) 
{
    if (fTokenString != ExpectedString) {
	KPException e;
	e << Position();
	e << "invalid token: \"" << AsString() << "\"";
	e << " (\"" + ExpectedString + "\" is expected)";
        throw e;
    }

    return *this;
}

KPToken& KPToken::MustBe(TTokenType ExpectedTokenType) 
{
    if (fType != ExpectedTokenType) {
        string Expected = R"(???)";
        switch (ExpectedTokenType) {
          case TokenType_Keyword:
            Expected = "keyword";
            break;
          case TokenType_Identifier:
            Expected = "identifier";
            break;
          case TokenType_Integer: 
            Expected = "integer";
            break;
          case TokenType_Floating: 
            Expected = "floating";
            break;
          case TokenType_Separator:
            Expected = "separator";
            break;
          case TokenType_Operator:
            Expected = "operator";
            break;
          case TokenType_Quote: 
            Expected = "quote";
            break;
          default:
            ;
        }
        
        KPException e;
        e << Position();
	e << "invalid token: \"" << AsString() << "\"";
	e << " (\"" << Expected << "\" is expected)";
	throw e;
    }

    return *this;
}

void KPToken::ThrowUnexpected(const string& Expected) 
{
    KPException e;
    e << Position();
    
    if (! IsEmpty()) {
	e << "unexpected token: \"" << AsString() << "\"";
    }
    else {
	e << "unexpected end-of-file";
    }

    if (! Expected.empty()) {
	e << " (" << Expected << " is expected)";
    }

    throw e;
}
