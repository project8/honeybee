// KPTokenTable.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <set>
#include <cctype>
#include "KPTokenTable.h"

using namespace std;
using namespace kebap;


static const char* KeywordList[] = {
    "int", "long", "float", "double", "string", 
    "list", "pointer", "variant", "var",
    "true", "false"
};

static const char* SeparatorList[] = {
    ";", ",",
    "(", ")",
    "{", "}",
    "[", "]"
};

static const char* CxxKeywordList[] = {
    "auto", "break", "case", "catch", "char", "class", "const",
    "continue", "default", "delete", "do", "double", "else", "enum",
    "extern", "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "namespace", "new", "operator", "private", "protected",
    "public", "register", "return", "short", "signed", "static",
    "struct", "switch", "templete", "this", "throw", "try", "typedef", 
    "typename", "union", "unsigned", "virtual", "void", "volatile", "while"
};

static const char* CxxSeparatorList[] = {
    ";", ",",
    "(", ")",
    "{", "}",
    "[", "]"
};

static const char* CxxOperatorList[] = {
    "+", "-", "*", "/", "%", "=",
    "++", "--",
    "&&", "||", "!",
    ">", "<", ">=", "<=", "==", "!=",
    "+=", "-=", "*=", "/=", "%=",
    ".", "->", "::",
    "&", "|", "&=", "|=",
    "^", "~", "^=", "~=",
    ">>", "<<", ">>=", "<<=",
    "?", ":",
    "new", "delete", "sizeof",
    "typeof", "<+>", "<&>", "<+>=", "=>", "**",
    "#!"
};


KPTokenTable::KPTokenTable()
{
    int NumberOfKeywords = sizeof(KeywordList) / sizeof(KeywordList[0]);
    for (int i = 0; i < NumberOfKeywords; i++) {
        AddKeyword(KeywordList[i]);
    }

    int NumberOfSeparators = sizeof(SeparatorList) / sizeof(SeparatorList[0]);
    for (int i = 0; i < NumberOfSeparators; i++) {
        AddSeparator(SeparatorList[i]);
    }
}

KPTokenTable::~KPTokenTable()
{
}

void KPTokenTable::Merge(KPTokenTable* Source)
{
    fKeywordSet.insert(
	Source->fKeywordSet.begin(), Source->fKeywordSet.end()
    );

    fSeparatorSet.insert(
	Source->fSeparatorSet.begin(), Source->fSeparatorSet.end()
    );

    fOperatorSet.insert(
	Source->fOperatorSet.begin(), Source->fOperatorSet.end()
    );

    fAlphabetSet.insert(
	Source->fAlphabetSet.begin(), Source->fAlphabetSet.end()
    );

    fFollowerAlphabetSet.insert(
	Source->fFollowerAlphabetSet.begin(), Source->fFollowerAlphabetSet.end()
    );

    fCommentLimiterTable.insert(
	Source->fCommentLimiterTable.begin(), Source->fCommentLimiterTable.end()
    );
}

void KPTokenTable::AddKeyword(const string& Keyword)
{
    fKeywordSet.insert(Keyword);
}

void KPTokenTable::AddOperator(const string& Operator)
{
    fOperatorSet.insert(Operator);

    // Definition of operators which the tokenizer recognizes is:
    //    Operator:
    //       OperatorCharacter
    //       Operator OperatorCharaceter
    //
    // To let the tokenizer recognize long operators, 
    // the following 'sub'-operators have to be registered.

    if (! isalpha(Operator[0]) && Operator[0] != 'f') {
	size_t Length = Operator.size();
	if (Length > 1) {
	    string SubOperator = Operator.substr(0, Length - 1);
	    AddOperator(SubOperator);
	}
    }
}

void KPTokenTable::AddSeparator(const string& Separator)
{
    fSeparatorSet.insert(Separator);
}

void KPTokenTable::AddAlphabet(char Alphabet)
{
    fAlphabetSet.insert(Alphabet);
}

void KPTokenTable::AddFollowerAlphabet(char Alphabet)
{
    fFollowerAlphabetSet.insert(Alphabet);
}

void KPTokenTable::AddCommentLimiter(const string& Limiter, const string& Delimiter)
{
    AddOperator(Limiter);
    fCommentLimiterTable[Limiter] = Delimiter;
}

bool KPTokenTable::IsKeyword(const string& TokenString) const
{
    return (fKeywordSet.count(TokenString) > 0);
}

bool KPTokenTable::IsOperator(const string& TokenString) const
{
    return (fOperatorSet.count(TokenString) > 0);
}

bool KPTokenTable::IsSeparator(const string& TokenString) const
{
    return (fSeparatorSet.count(TokenString) > 0);
}

bool KPTokenTable::IsAlphabet(const char& Character) const
{
    return isalpha(Character) || (fAlphabetSet.count(Character) > 0);
}

bool KPTokenTable::IsFollowerAlphabet(const char& Character) const
{
    return isalnum(Character) || (fAlphabetSet.count(Character) > 0) || (fFollowerAlphabetSet.count(Character) > 0);
}

bool KPTokenTable::IsCommentLimiter(const string& TokenString) const
{
    return (fCommentLimiterTable.count(TokenString) > 0);
}

string KPTokenTable::CommentDelimiterFor(const string& Limiter) const
{
    map<string, string>::const_iterator Iterator;
    Iterator = fCommentLimiterTable.find(Limiter);

    if (Iterator != fCommentLimiterTable.end()) {
	return (*Iterator).second;
    }
    else {
	return string("");
    }
}



KPCxxTokenTable::KPCxxTokenTable()
{
    int i;
    int NumberOfKeywords = sizeof(CxxKeywordList) / sizeof(CxxKeywordList[0]);
    for (i = 0; i < NumberOfKeywords; i++) {
        AddKeyword(CxxKeywordList[i]);
    }

    int NumberOfSeparators = sizeof(CxxSeparatorList) / sizeof(CxxSeparatorList[0]);
    for (i = 0; i < NumberOfSeparators; i++) {
        AddSeparator(CxxSeparatorList[i]);
    }

    int NumberOfOperators = sizeof(CxxOperatorList) / sizeof(CxxOperatorList[0]);
    for (i = 0; i < NumberOfOperators; i++) {
        AddOperator(CxxOperatorList[i]);
    }

    AddAlphabet('_');

    AddCommentLimiter("/*", "*/");
    AddCommentLimiter("//", "\n");
    AddCommentLimiter("#!", "\n");
}


KPCxxTokenTable::~KPCxxTokenTable()
{
}
