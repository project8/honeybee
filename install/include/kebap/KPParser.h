// KPParser.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPParser_h__
#define __KPParser_h__

#include <string>
#include <iostream>
#include "KPTokenizer.h"
#include "KPOperator.h"
#include "KPExpression.h"
#include "KPStatement.h"
#include "KPModule.h"
#include "KPStandardLibrary.h"
#include "KPMathLibrary.h"


namespace kebap {

class KPParser {
  public:
    KPParser();
    virtual ~KPParser();
    virtual void Merge(KPParser* Source);
    virtual void Parse(std::istream& SourceStream) ;
    virtual KPValue Execute(const std::string& EntryName = "") ;
    virtual KPValue Execute(const std::string& EntryName, const std::vector<KPValue*>& ArgumentList) ;
    virtual bool HasEntryOf(const std::string& EntryName) const;
    virtual KPSymbolTable* GetSymbolTable();
    virtual KPTokenTable* GetTokenTable();
    virtual KPExpressionParser* GetExpressionParser();
    virtual KPStatementParser* GetStatementParser();
    virtual KPModule* GetModule();
    virtual void SetLineNumberOffset(long LineNumberOffset);
  protected:
    virtual void OnConstruct() {}
    virtual KPObjectPrototypeTable* CreateObjectPrototypeTable();
    virtual KPBuiltinFunctionTable* CreateBuiltinFunctionTable();
    virtual KPTokenTable* CreateTokenTable();
    virtual KPOperatorTable* CreateOperatorTable();
    virtual KPStatementTable* CreateStatementTable();
    virtual KPModule* CreateModule();
  private:
    virtual void Construct();
  protected:
    KPObjectPrototypeTable* fObjectPrototypeTable;
    KPBuiltinFunctionTable* fBuiltinFunctionTable;
    KPTokenTable* fTokenTable;
    KPOperatorTable* fOperatorTable;
    KPStatementTable* fStatementTable;
    KPSymbolTable* fSymbolTable;
    KPExpressionParser* fExpressionParser;
    KPStatementParser* fStatementParser;
    KPModule* fModule;
    long fLineNumberOffset;
  private:
    bool fIsConstructed;
};


class KPStandardParser: public KPParser {
  public:
    KPStandardParser();
    KPStandardParser(int argc, char** argv);
    ~KPStandardParser() override;
  protected:
    KPObjectPrototypeTable* CreateObjectPrototypeTable() override;
    KPBuiltinFunctionTable* CreateBuiltinFunctionTable() override;
    KPTokenTable* CreateTokenTable() override;
    KPOperatorTable* CreateOperatorTable() override;
    KPStatementTable* CreateStatementTable() override;
    KPModule* CreateModule() override;
  protected:
    int fArgc;
    char** fArgv;
};


}
#endif

