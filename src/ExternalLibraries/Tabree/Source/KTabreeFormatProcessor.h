// KTabreeFormatProcessor.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTabreeFormatProcessor_h__
#define KTabreeFormatProcessor_h__


#include <iostream>
#include <string>
#include <vector>
#include "KTable.h"
#include "KTreeBuilder.h"


namespace tabree {


class KTabreeFormatLineExtractor {
  public:
    KTabreeFormatLineExtractor(std::istream& Input, char LineTerminator = '\0');
    virtual ~KTabreeFormatLineExtractor();
    virtual bool GetNext(std::string& Line);
  protected:
    std::istream& fInput;
    char fLineTerminator;
  private:
    char fLastChar;
    int fCRCount;
};


class KTabreeFormatTokenizer {
  public:
    KTabreeFormatTokenizer();
    virtual ~KTabreeFormatTokenizer();
    virtual char SetDelimiter(char Delimiter);
    virtual char SetQuote(char Quote);
    virtual void Tokenize(const std::string& Line, std::vector<std::string>& ElementList) const ;
  protected:
    char fDelimiter, fQuote;
};


class KTabreeFormatColumnParser: public KTabreeFormatTokenizer {
  public:
    KTabreeFormatColumnParser();
    ~KTabreeFormatColumnParser() override;
    virtual void SetColumnTypeList(const std::vector<std::string>& ColumnTypeList);
    virtual unsigned ProcessLine(KTable::TRow Row, std::string& Line, unsigned ColumnOffset) ;
  protected:
    enum TColumnType {
	ColumnType_Int,
	ColumnType_Float,
	ColumnType_String,
	fNumberOfColumnTypes
    };
    std::vector<TColumnType> fColumnTypeList;
};


class KTabreeFormatHeaderProcessor: public KTabreeFormatTokenizer {
  public:
    KTabreeFormatHeaderProcessor();
    ~KTabreeFormatHeaderProcessor() override;
    virtual void EnableCsvHeader();
    virtual void SetCommentHeader(char CommentHeader);
    virtual void SetTreeIndent(char TreeIndent);
  public:
    virtual void SetStorage(KTree* Tree);
    virtual bool ProcessFirstLine(const std::string& Line);
    virtual bool ProcessLine(const std::string& Line);
  protected:
    virtual bool ProcessCsvHeader(const std::string& Line);
    virtual bool ProcessRootTreeDescriptor(const std::string& Line);
  protected:
    KTreeBuilder* fTreeBuilder;
    bool fIsCsvHeaderEnabled;
    char fCommentHeader, fTreeIndent;
};

}
#endif
