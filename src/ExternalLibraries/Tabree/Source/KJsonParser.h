// KJsonParser.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KJsonParser_h__
#define KJsonParser_h__

#include <iostream>
#include <string>
#include <deque>
#include "KTree.h"


namespace tabree {


class KJsonTokenizer {
  public:
    KJsonTokenizer(std::istream& Input);
    virtual ~KJsonTokenizer();
    virtual bool GetNext(std::string& Token);
    virtual KJsonTokenizer& PushBack(const std::string& Token);
  protected:
    virtual bool Tokenize(std::string& Token);
    virtual char GetNextChar();
    virtual void PushBackChar(char Ch);
  protected:
    std::istream& fInput;
    std::deque<std::string> fTokenQueue;
    std::deque<char> fCharQueue;
};


class KJsonParser {
  public:
    KJsonParser();
    virtual ~KJsonParser();
    virtual void Parse(std::istream& Input, KTree& Tree) ;
  protected:
    virtual void ParseElement(KJsonTokenizer& Tokenizer, KTree& Tree) ;
    virtual void ParseIndexedArray(KJsonTokenizer& Tokenizer, KTree& Tree) ;
    virtual void ParseAssosiativeArray(KJsonTokenizer& Tokenizer, KTree& Tree) ;
};


}
#endif
