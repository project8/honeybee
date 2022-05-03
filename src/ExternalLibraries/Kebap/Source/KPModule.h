// KPModule.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPModule_h__
#define __KPModule_h__

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "KPTokenizer.h"
#include "KPExpression.h"
#include "KPSymbolTable.h"
#include "KPStatement.h"
#include "KPFunction.h"


namespace kebap {

class KPModuleEntry;

class KPModule {
  public:
    KPModule();
    virtual ~KPModule();
    virtual void Merge(KPModule* Source);
    virtual void AddEntry(KPModuleEntry* EntryPrototype);
    virtual KPModuleEntry* CreateEntry(KPTokenizer* Tokenizer);
    virtual void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) ;
    virtual KPValue Execute(KPSymbolTable* SymbolTable) ;
    virtual KPValue Execute(const std::string& EntryName, KPSymbolTable* SymbolTable) ;
    virtual KPValue Execute(const std::string& EntryName, const std::vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) ;
    virtual void ExecuteBareStatements(KPSymbolTable* SymbolTable) ;
    virtual KPModuleEntry* GetEntry(const std::string& EntryName);
    virtual const std::vector<KPModuleEntry*>& EntryList() const;
    virtual const std::vector<std::string>& EntryNameList() const;
  protected:
    std::vector<KPModuleEntry*> fEntryPrototypeList;
    std::vector<KPModuleEntry*> fEntryList;
    std::map<std::string, KPModuleEntry*> fEntryTable;
    std::vector<std::string> fEntryNameList;
    std::vector<KPStatement*> fBareStatementList;
  private:
    unsigned fNumberOfProcessedBareStatements;
};


class KPCxxModule: public KPModule {
  public:
    KPCxxModule();
    ~KPCxxModule() override;    
};


class KPModuleEntry {
  public:
    KPModuleEntry(const std::string& EntryTypeName);
    virtual ~KPModuleEntry();
    virtual KPModuleEntry* Clone() = 0;
    virtual bool HasEntryWordsOf(KPTokenizer* Tokenizer) = 0;
    virtual void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable)  = 0;
    virtual KPValue Execute(const std::vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) ;
    virtual const std::string& EntryTypeName() const;
    virtual const std::string& EntryName() const;
  protected:
    virtual void SetEntryName(const std::string& EntryName);
  private:
    std::string fEntryTypeName;
    std::string fEntryName;
};


class KPFunctionEntry: public KPModuleEntry {
  public:
    KPFunctionEntry();
    ~KPFunctionEntry() override;
    KPModuleEntry* Clone() override;
    bool HasEntryWordsOf(KPTokenizer* Tokenizer) override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    KPValue Execute(const std::vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) override ;
  protected:
    KPFunction* fFunction;
};


class KPIncludeEntry: public KPModuleEntry {
  public:
    KPIncludeEntry();
    ~KPIncludeEntry() override;
    KPModuleEntry* Clone() override;
    bool HasEntryWordsOf(KPTokenizer* Tokenizer) override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
  protected:
    std::ifstream* fInputFile;
};


}
#endif
