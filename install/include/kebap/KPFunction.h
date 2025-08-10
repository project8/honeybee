// KPFunction.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPFunction_h__
#define __KPFunction_h__

#include <string>
#include <vector>
#include "KPTokenizer.h"
#include "KPExpression.h"
#include "KPSymbolTable.h"
#include "KPStatement.h"


namespace kebap {

class KPFunction {
  public:
    KPFunction();
    virtual ~KPFunction();
    virtual std::string Name();
    virtual void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable)  = 0;
    virtual KPValue Execute(const std::vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) ;
  protected:
    virtual void ProcessArguments(const std::vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) ;
  protected:
    virtual void SetName(const std::string& Name);
    virtual void SetReturnValue(KPValue* ReturnValue);
    virtual void AddArgumentDeclaration(KPVariableDeclaration* ArgumentDeclaration);
    virtual void SetStatement(KPStatement* Statement);
  private:
    std::string fName;
    std::vector<KPVariableDeclaration*> fArgumentDeclarationList;
    KPStatement* fStatement;
    KPValue* fReturnValue;
};



class KPCxxFunction: public KPFunction {
  public:
    KPCxxFunction();
    ~KPCxxFunction() override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
  protected:
    virtual void ParseArgumentDeclaration(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) ;
};


}
#endif
