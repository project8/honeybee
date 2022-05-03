// KPTokenizer.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cctype>
#include "KPToken.h"
#include "KPTokenTable.h"
#include "KPTokenizer.h"

using namespace std;
using namespace kebap;


KPInputBuffer::KPInputBuffer(istream& InputStream)
: fInputStream(InputStream)
{
    fInputStream.unsetf(ios::skipws);
    fLineNumber = 1;

    fChildInputBuffer = nullptr;
}

KPInputBuffer::~KPInputBuffer()
{
}

bool KPInputBuffer::GetNext(char& Character)
{
    if (! fUngetStack.empty()) {
	Character = fUngetStack.top();
	fUngetStack.pop();
	return true;
    }

    if (fChildInputBuffer) {
	if (fChildInputBuffer->GetNext(Character)) {
	    return true;
	}
	else {
	    delete fChildInputBuffer;
	    fChildInputBuffer = nullptr;
	}
    }

    if (! (fInputStream >> Character)) {
	Character = 0;
	return false;
    }

    if (Character == '\n') {
	fLineNumber++;
    }

    return true;
}

void KPInputBuffer::Unget(char Character)
{
    if (fChildInputBuffer) {
	fChildInputBuffer->Unget(Character);
    }
    else {
	fUngetStack.push(Character);
    }
}

long KPInputBuffer::LineNumber() 
{
    if (fChildInputBuffer) {
	return fChildInputBuffer->LineNumber();
    }
    else {
	return fLineNumber;
    }
}

void KPInputBuffer::SetChildInput(istream& InputStream)
{
    this->SetChildInputBuffer(new KPInputBuffer(InputStream));
}

void KPInputBuffer::SetChildInputBuffer(KPInputBuffer* InputBuffer)
{
    if (fChildInputBuffer) {
	fChildInputBuffer->SetChildInputBuffer(InputBuffer);
    }
    else {
	fChildInputBuffer = InputBuffer;
    }
}

bool KPInputBuffer::AbortChildInput()
{
    if (fChildInputBuffer == nullptr) {
	return false;
    }

    if (! fChildInputBuffer->AbortChildInput()) {
	delete fChildInputBuffer;
	fChildInputBuffer = nullptr;
    }

    return true;
}



KPTokenizer::KPTokenizer(istream& InputStream, const KPTokenTable* TokenTable)
{
    fMyInputBuffer = new KPInputBuffer(InputStream);
    fInputBuffer = fMyInputBuffer;
    fTokenTable = TokenTable;

    fIsCommentSkippingEnabled = true;
    fIsWhiteSpaceSkippingEnabled = true;
    fIsEscapeSequenceProcessingEnabled = true;

    fLineNumberOffset = 0;
}

KPTokenizer::KPTokenizer(KPInputBuffer* InputBuffer, const KPTokenTable* TokenTable)
{
    fMyInputBuffer = nullptr;
    fInputBuffer = InputBuffer;
    fTokenTable = TokenTable;

    fIsCommentSkippingEnabled = true;
    fIsWhiteSpaceSkippingEnabled = true;
    fIsEscapeSequenceProcessingEnabled = true;

    fLineNumberOffset = 0;
}

KPTokenizer::~KPTokenizer()
{
    delete fMyInputBuffer;
}

void KPTokenizer::SetTokenTable(const KPTokenTable* TokenTable)
{
    fTokenTable = TokenTable;
}

void KPTokenizer::SetCommentSkipping(bool IsEnabled)
{
    fIsCommentSkippingEnabled = IsEnabled;
}

void KPTokenizer::SetWhiteSpaceSkipping(bool IsEnabled)
{
    fIsWhiteSpaceSkippingEnabled = IsEnabled;
}

void KPTokenizer::SetEscapeSequenceProcessing(bool IsEnabled)
{
    fIsEscapeSequenceProcessingEnabled = IsEnabled;
}

KPToken KPTokenizer::Next()
{
    KPToken Token;

    if (fUngetStack.empty()) {
	Token = ParseNext();
    }
    else {
	Token = fUngetStack.top();
	fUngetStack.pop();
    }

    if (fIsWhiteSpaceSkippingEnabled && Token.IsWhiteSpace()) {
        return Next();
    }
    if (fIsCommentSkippingEnabled && Token.IsComment()) {
        return Next();
    }

    return Token;
}

KPToken KPTokenizer::GetLine(char Terminator)
{
    string TokenString;
    KPToken::TTokenType TokenType = KPToken::TokenType_Unknown;

    char Character;
    while ((fInputBuffer->GetNext(Character)) && (Character != Terminator)) {
	TokenString += Character;
    }

    return KPToken(TokenString, TokenType, 0, LineNumber());
}

bool KPTokenizer::GetChar(char& Character)
{
    return fInputBuffer->GetNext(Character);
}

void KPTokenizer::UngetChar(char Character)
{
    fInputBuffer->Unget(Character);
}

KPTokenizer& KPTokenizer::SkipWhiteSpace()
{
    char Character;
    while ((fInputBuffer->GetNext(Character)) && isspace(Character)) {
	;
    }

    if (Character != 0) {
	fInputBuffer->Unget(Character);
    }

    return *this;
}

long KPTokenizer::LineNumber() const
{
    return fLineNumberOffset + fInputBuffer->LineNumber();
}

void KPTokenizer::SetLineNumber(int LineNumber)
{
    fLineNumberOffset = LineNumber - this->LineNumber();
}

KPInputBuffer* KPTokenizer::InputBuffer()
{
    return fInputBuffer;
}

void KPTokenizer::Unget(KPToken &Token)
{
    fUngetStack.push(Token);
}

KPToken KPTokenizer::LookAhead(int n)
{
    KPToken Token;
    vector<KPToken> PrecedingTokenList;
    
    for (int i = 0; i < n; i++) {
        Token = Next();
        PrecedingTokenList.push_back(Token);
    }
    for (int j = n - 1; j >= 0; j--) {
        Unget(PrecedingTokenList[j]);
    }

    return Token;
}

bool KPTokenizer::ConvertEscape(char& Character) const
{
    if (! fIsEscapeSequenceProcessingEnabled) {
	return false;
    }

    switch (Character) {
      case 'a':
	Character = '\a'; break;
      case 'b':
        Character = '\b'; break;
      case 'f':
        Character = '\f'; break;
      case 'n':
        Character = '\n'; break;
      case 'r':
        Character = '\r'; break;
      case 't':
        Character = '\t'; break;
      case 'v':
        Character = '\v'; break;
      case '\\':
        break;
      case '\"':
        break;
      case '\'':
        break;
      case '\?':
        break;
      default:
	return false;
    }

    return true;
}

KPToken KPTokenizer::ParseNext()
{
    string TokenString;
    KPToken::TTokenType TokenType = KPToken::TokenType_Empty;
    unsigned NumberSuffixFlags = 0;

    char Character;
    if (fInputBuffer->GetNext(Character)) {
        TokenString += Character;
    }
    else {
        return KPToken("", KPToken::TokenType_Empty, 0, LineNumber());
    }

    char NextCharacter = 0;
    if (fInputBuffer->GetNext(NextCharacter)) {
        fInputBuffer->Unget(NextCharacter);
    }
    
    if (isspace(Character)) {
	ParseWhiteSpace(TokenString, TokenType);
    }
    else if (fTokenTable->IsAlphabet(Character)) {
        ParseIdentifier(TokenString, TokenType);
    }
    else if (isdigit(Character)) {
        ParseNumber(TokenString, TokenType);

        if (TokenType == KPToken::TokenType_Integer) {
            while (fInputBuffer->GetNext(Character)) {
                if (toupper(Character) == 'I') {
                    NumberSuffixFlags |= KPToken::NumberSuffix_Imaginary;
                }
                else if (toupper(Character) == 'U') {
                    NumberSuffixFlags |= KPToken::NumberSuffix_Unsigned;
                }
                else if (toupper(Character) == 'L') {
                    NumberSuffixFlags |= KPToken::NumberSuffix_Long;
                }
                else {
                    fInputBuffer->Unget(Character);
                    break;
                }
                TokenString += Character;
            }
	}
    }
    else if ((Character == '.') && isdigit(NextCharacter)) {
        ParseFloating(TokenString, TokenType);
    }
    else if ((Character == '\"') || (Character == '\'')) {
        ParseQuote(TokenString, TokenType);
    }
    else if (fTokenTable->IsOperator(TokenString)){
        ParseOperator(TokenString, TokenType);
    }
    else if (fTokenTable->IsSeparator(TokenString)){
        TokenType = KPToken::TokenType_Separator;
    }
    else {
        TokenType = KPToken::TokenType_Unknown;
    }

    return KPToken(
	TokenString, TokenType, NumberSuffixFlags, LineNumber()
    );
}

void KPTokenizer::ParseIdentifier(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    while (fInputBuffer->GetNext(Character)) {
        if (fTokenTable->IsFollowerAlphabet(Character)) {
            TokenString += Character;
        }
        else {
            fInputBuffer->Unget(Character);
            break;
        }
    }

    if (fTokenTable->IsCommentLimiter(TokenString)) {
        ParseComment(TokenString, TokenType);
    }
    else if (fTokenTable->IsKeyword(TokenString)) {
	if ((TokenString == "true") || (TokenString == "false")) {
	    TokenType = KPToken::TokenType_Bool;
	}
	else {
	    TokenType = KPToken::TokenType_Keyword;
	}
    }
    else if (fTokenTable->IsOperator(TokenString)) {
        TokenType = KPToken::TokenType_Operator;
    }
    else {
        TokenType = KPToken::TokenType_Identifier;
    }
}

void KPTokenizer::ParseNumber(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    while (fInputBuffer->GetNext(Character)) {
        if (isdigit(Character)) {
            TokenString += Character;
            continue;
        }

        char NextCharacter = 0;
        if (fInputBuffer->GetNext(NextCharacter)) {
            fInputBuffer->Unget(NextCharacter);
        }
        
        if (toupper(Character) == 'X') {
            if ((TokenString == "0") && isxdigit(NextCharacter)) {
                TokenString += Character;
                ParseHexNumber(TokenString, TokenType);
                return;
            }
        }
        else if (toupper(Character) == 'B') {
            if ((TokenString == "0") && isdigit(NextCharacter)) {
                TokenString += Character;
                ParseBinNumber(TokenString, TokenType);
                return;
            }
        }
        else if (Character == '.') {
            TokenString += Character;
            ParseFloating(TokenString, TokenType);
            return;
        }
        else if (toupper(Character) == 'E') {
	    unsigned OriginalLength = TokenString.size();

	    fInputBuffer->Unget(Character);
	    ParseFloating(TokenString, TokenType);

	    if (TokenString.size() > OriginalLength) {
		return;
	    }
	    else {
		break;
	    }
        }

        fInputBuffer->Unget(Character);
        break;
    }    

    TokenType = KPToken::TokenType_Integer;
}

void KPTokenizer::ParseHexNumber(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    while (fInputBuffer->GetNext(Character)) {
        if (isxdigit(Character)) {
            TokenString += Character;
        }
        else {
            fInputBuffer->Unget(Character);
            break;
        }
    }    

    TokenType = KPToken::TokenType_Integer;
}

void KPTokenizer::ParseBinNumber(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    while (fInputBuffer->GetNext(Character)) {
        if (isdigit(Character)) {
            TokenString += Character;
        }
        else {
            fInputBuffer->Unget(Character);
            break;
        }
    }    

    TokenType = KPToken::TokenType_Integer;
}

void KPTokenizer::ParseFloating(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    while (fInputBuffer->GetNext(Character)) {

        if (isdigit(Character)) {
            TokenString += Character;
	    continue;
        }
        else if (toupper(Character) != 'E') {
            fInputBuffer->Unget(Character);
	    break;
	}
        else {
	    char NextCharacter = 0;
	    char NextNextCharacter = 0;
	    if (fInputBuffer->GetNext(NextCharacter)) {
		if (fInputBuffer->GetNext(NextNextCharacter)) {
		    fInputBuffer->Unget(NextNextCharacter);
		}
		fInputBuffer->Unget(NextCharacter);
	    }
        
            bool NextIsSign = (
                (NextCharacter == '-') || (NextCharacter == '+')
            );
            
            if (
                isdigit(NextCharacter) ||
                (NextIsSign && isdigit(NextNextCharacter))
            ){
                TokenString += Character;
                ParseFloatingExponent(TokenString, TokenType);
                break;
            }

            fInputBuffer->Unget(Character);
            break;
        }
    }    

    TokenType = KPToken::TokenType_Floating;
}

void KPTokenizer::ParseFloatingExponent(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    if (fInputBuffer->GetNext(Character)) {
        if ((Character == '+') || (Character == '-')) {
            TokenString += Character;
        }
        else {
            fInputBuffer->Unget(Character);
        }
    }
    
    while (fInputBuffer->GetNext(Character)) {
        if (isdigit(Character)) {
            TokenString += Character;
        }
        else {
            fInputBuffer->Unget(Character);
            break;
        }
    }    

    TokenType = KPToken::TokenType_Floating;
}

void KPTokenizer::ParseOperator(string& TokenString, KPToken::TTokenType& TokenType)
{
    string NextString = TokenString;
    char Character;
    while (fInputBuffer->GetNext(Character)) {
        NextString += Character;
        if (fTokenTable->IsCommentLimiter(NextString)) {
            TokenString += Character;
            ParseComment(TokenString, TokenType);
            return;
        }
        
        if (fTokenTable->IsOperator(NextString)) {
            TokenString += Character;
        }
        else {
            fInputBuffer->Unget(Character);
            break;
        }
    }

    TokenType = KPToken::TokenType_Operator;
}

void KPTokenizer::ParseQuote(string& TokenString, KPToken::TTokenType& TokenType)
{
    char Quoting = TokenString[0];
    char Character;
    while (fInputBuffer->GetNext(Character)) {

        if (Character == '\\') {
            if (fInputBuffer->GetNext(Character)) {
                if (! ConvertEscape(Character)) {
		    TokenString += '\\';
		}
                TokenString += Character;
            }
            else {
                TokenString += '\\';
            }
        }
        else {
            TokenString += Character;
            if (Character == Quoting) {
                break;
            }
        }
        
    }

    TokenType = KPToken::TokenType_Quote;
}

void KPTokenizer::ParseComment(string& TokenString, KPToken::TTokenType& TokenType)
{
    string Limiter = TokenString;
    string Delimiter = fTokenTable->CommentDelimiterFor(Limiter);

    unsigned DelimiterLength = Delimiter.size();
    unsigned DelimiterOffset = TokenString.size();
    unsigned MatchLength = 0;
    
    char Character;
    while (MatchLength < DelimiterLength) {
	if (DelimiterOffset + MatchLength < TokenString.size()) {
	    Character = TokenString[DelimiterOffset + MatchLength];
	}
	else {
	    if (! fInputBuffer->GetNext(Character)) {
		break;
	    }
	    TokenString += Character;
	}

	if (Character == Delimiter[MatchLength]) {
	    MatchLength++;
	}
	else {
	    DelimiterOffset++;
	    MatchLength = 0;
	}
    }

    TokenType = KPToken::TokenType_Comment;
}

void KPTokenizer::ParseWhiteSpace(std::string& TokenString, KPToken::TTokenType& TokenType)
{
    char Character;
    while (fInputBuffer->GetNext(Character)) {
        if (isspace(Character)) {
	    TokenString += Character;
	}
	else {
            fInputBuffer->Unget(Character);
            break;
        }
    }

    TokenType = KPToken::TokenType_WhiteSpace;
}
