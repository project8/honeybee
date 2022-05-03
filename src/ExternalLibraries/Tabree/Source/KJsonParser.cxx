// KJsonParser.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <limits>
#include <cctype>
#include "KJsonParser.h"
#include "KTree.h"

using namespace std;
using namespace tabree;


KJsonTokenizer::KJsonTokenizer(std::istream& Input)
: fInput(Input)
{
}

KJsonTokenizer::~KJsonTokenizer()
{
}

bool KJsonTokenizer::GetNext(std::string& Token)
{
    if (! fTokenQueue.empty()) {
        Token = fTokenQueue.back();
        fTokenQueue.pop_back();
        return true;
    }

    return Tokenize(Token);
}

KJsonTokenizer& KJsonTokenizer::PushBack(const std::string& Token)
{
    fTokenQueue.push_back(Token);
    return *this;
}

bool KJsonTokenizer::Tokenize(std::string& Token)
{
    bool IsFirst = true;
    bool IsQuoted = false;
    bool IsEscaped = false;
    Token.clear();

    char Ch;
    while ((Ch = GetNextChar()) != '\0') {
        if (IsFirst) {
            if (isspace(Ch)) {
                continue;
            }
            IsFirst = false;
            if (Ch == '"') {
                IsQuoted = true;
                Token += Ch;
                continue;
            }
        }
        if (IsEscaped) {
            Token += Ch;
            IsEscaped = false;
            continue;
        }
        if (Ch == '\\') {
            IsEscaped = true;
            continue;
        }
        if (IsQuoted && (Ch == '"')) {
            Token += Ch;
            break;
        }
        if (IsQuoted) {
            Token += Ch;
            continue;
        }
        if (isspace(Ch)) {
            if (Token.empty()) {
                continue;
            }
            else {
                PushBackChar(Ch);
                break;
            }
        }
        if (
            (Ch == '[') || (Ch == ']') || 
            (Ch == '{') || (Ch == '}') || 
            (Ch == ',') || (Ch == ':')
        ){
            if (Token.empty()) {
                Token = Ch;
            }
            else {
                PushBackChar(Ch);
            }
            break;
        }

        Token += Ch;
    }

    return ! Token.empty();
}

char KJsonTokenizer::GetNextChar()
{
    if (! fCharQueue.empty()) {
        char Ch = fCharQueue.back();
        fCharQueue.pop_back();
        return Ch;
    }

    char Ch;
    if ((! fInput) || (! fInput.get(Ch))) {
        return '\0';
    }
    return Ch;
}

void KJsonTokenizer::PushBackChar(char Ch)
{
    fCharQueue.push_back(Ch);
}



KJsonParser::KJsonParser()
{
}

KJsonParser::~KJsonParser()
{
}

void KJsonParser::Parse(std::istream& Input, KTree& Tree) 
{
    string Token;
    KJsonTokenizer Tokenizer(Input);
    ParseElement(Tokenizer, Tree);

    if (Tokenizer.GetNext(Token)) {
        throw KException() << "JSON syntax error: extra token before EOF";
    }
}

void KJsonParser::ParseElement(KJsonTokenizer& Tokenizer, KTree& Tree) 
{
    string Token;
    if (! Tokenizer.GetNext(Token)) {
        return;
    }

    if (Token == "[") {
        ParseIndexedArray(Tokenizer.PushBack(Token), Tree);
    }
    else if (Token == "{") {
        ParseAssosiativeArray(Tokenizer.PushBack(Token), Tree);
    }
    else {
        if (Token.empty() || Token == "null") {
            Tree.Value() = KVariant();
        }
        else if (*Token.begin() == '"' && *Token.rbegin() == '"') {
            Tree.Value() = Token.substr(1, Token.size() - 2);
        }
        else if (Token == "true") {
            Tree.Value() = KVariant(true);
        }
        else if (Token == "false") {
            Tree.Value() = KVariant(false);
        }
        else if ((Token == "nan") || (Token == "NaN")) {
            Tree.Value() = KVariant(std::numeric_limits<double>::quiet_NaN());
        }
        else {
            KVariant value(Token);
            try {
                Tree.Value() = value.AsLong();
            }
            catch (KException &e) {
                try {
                    Tree.Value() = value.AsDouble();
                }
                catch (KException&e) {
                    Tree.Value() = value;
                }
            }
        }
    }
}

void KJsonParser::ParseIndexedArray(KJsonTokenizer& Tokenizer, KTree& Tree) 
{
    string Token;
    if (! Tokenizer.GetNext(Token) || (Token != "[")) {
        throw KException() << "JSON syntax error: '[' is expected";
    }

    Tree = KTree::KEmptyArray();

    int Index = 0;
    while (Tokenizer.GetNext(Token)) {
        if (Token == "]") {
            break;
        }

        // extension to JSON: allow multiple "words"
        string Text = Token;
        if (isalnum(Token[0]) || Token[0] == '_') {
            while (Tokenizer.GetNext(Token)) {
                if (! isalnum(Token[0]) && Token[0] != '_') {
                    Tokenizer.PushBack(Token);
                    break;
                }
                Text += " " + Token;
            }
        }

        Tokenizer.PushBack(Text);
        ParseElement(Tokenizer, Tree[Index]);
        Index++;

        Tokenizer.GetNext(Token);
        if (Token == ",") {
            continue;
        }
        else if (Token == "]") {
            break;
        }
        else {
            throw KException() << "JSON syntax error: ']' is expected";
        }
    }
}

void KJsonParser::ParseAssosiativeArray(KJsonTokenizer& Tokenizer, KTree& Tree) 
{
    string Token;
    if (! Tokenizer.GetNext(Token) || (Token != "{")) {
        throw KException() << "JSON syntax error: '{' is expected";
    }

    while (Tokenizer.GetNext(Token)) {
        if (Token == "}") {
            break;
        }

        string Key = Token;
        Tokenizer.GetNext(Token);
        if (Token != ":") {
            throw KException() << "JSON syntax error: ':' is expected";
        }
        if (! Key.empty() && *Key.begin() == '"' && *Key.rbegin() == '"') {
            Key = Key.substr(1, Key.size() - 2);
        }

        if (! Tokenizer.GetNext(Token)) {
            throw KException() << "JSON syntax error: unexpected EOF";
        }
        string Text = Token;
        
        // extension to JSON: allow multiple "words"
        if (isalnum(Token[0]) || Token[0] == '_') {
            while (Tokenizer.GetNext(Token)) {
                if (! isalnum(Token[0]) && Token[0] != '_') {
                    Tokenizer.PushBack(Token);
                    break;
                }
                Text += " " + Token;
            }
        }
        
        Tokenizer.PushBack(Text);
        ParseElement(Tokenizer, Tree[Key]);

        Tokenizer.GetNext(Token);
        if (Token == ",") {
            continue;
        }
        else if (Token == "}") {
            break;
        }
        else {
            throw KException() << "JSON syntax error: '}' is expected instead of '" << Token << "'";
        }
    }
}
