// KPSymbolTable.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPSymbolTable_h__
#define __KPSymbolTable_h__

#include <string>
#include <vector>
#include <map>
#include "KPException.h"

namespace kebap {

class KPObjectPrototype;
class KPValue;
class KPFunction;
class KPObjectPrototypeTable;
class KPBuiltinFunctionTable;


class KPNameTable {
  public:
    static KPNameTable* GetInstance();
    ~KPNameTable();
    long NameToId(const std::string& Name);
    std::string IdToName(long Id);
  protected:
    KPNameTable();
    static KPNameTable* fInstance;
  protected:
    std::map<std::string, long> fIdTable;
    long fNextId;
};


class KPSymbolTable {
  public:
    KPSymbolTable(KPObjectPrototypeTable* ObjectPrototypeTable = nullptr, KPBuiltinFunctionTable* BuiltinFunctionTable = nullptr);
    KPSymbolTable(KPSymbolTable* SymbolTable, int ImportDepth = 1);
    virtual ~KPSymbolTable();
    virtual bool IsTypeName(const std::string& Symbol) const;
    virtual KPValue* CreateObject(const std::string& TypeName, int Length = 0);
    virtual void EnterBlock();
    virtual void ExitBlock() ;
    virtual long NameToId(const std::string& Name);
    virtual std::string IdToName(long Id);
    virtual long RegisterVariable(const std::string& Name, const KPValue& InitialValue);
    virtual long RegisterVariable(const std::string& Name, KPValue* Variable);
    virtual long RegisterVariable(long VariableId, KPValue* Variable);
    virtual KPValue* GetVariable(long VariableId);
    virtual void RegisterFunction(long FunctionId, KPFunction* Function);
    virtual KPFunction* GetFunction(long FunctionId);
    virtual long Import(KPSymbolTable* SymbolTable, int Depth = 1);
    virtual KPObjectPrototypeTable* ObjectPrototypeTable();
    virtual KPBuiltinFunctionTable* BuiltinFunctionTable();
  protected:
    KPNameTable* fNameTable;
    KPObjectPrototypeTable* fObjectPrototypeTable;
    KPBuiltinFunctionTable* fBuiltinFunctionTable;
    int fCurrentBlockDepth;
    long fVariableCount;
    std::vector<long> fVariableCountList;
    std::vector<std::pair<long, KPValue*> > fVariableEntryList;
    std::vector<KPValue*> fGlobalVariableList;
    std::map<long, KPFunction*>* fFunctionTable;
    std::map<long, KPFunction*>* fOriginalFunctionTable;
};


}
#endif
