// KPBuiltinFunction.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPBuiltinFunction_h__
#define __KPBuiltinFunction_h__

#include <string>
#include <vector>
#include <map>
#include "KPObject.h"
#include "KPValue.h"
#include "KPSymbolTable.h"


namespace kebap {

class KPBuiltinFunctionTable {
  public:
    KPBuiltinFunctionTable();
    virtual ~KPBuiltinFunctionTable();
    virtual void Merge(KPBuiltinFunctionTable* Source);
    virtual void RegisterStaticObject(KPObjectPrototype* Prototype);
    virtual void RegisterFunctionId(const std::string& FunctionName, long FunctionId);
    virtual int Execute(long FunctionId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
  protected:
    std::vector<KPObjectPrototype*> fPrototypeList;
    std::map<long, int> fClassIdTable;
    std::map<long, int> fMethodIdTable;
    std::map<long, std::string> fMethodNameTable;
};


}
#endif
