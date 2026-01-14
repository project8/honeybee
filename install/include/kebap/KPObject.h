// KPObject.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPObject_h__
#define __KPObject_h__

#include <string>
#include <vector>
#include <map>
#include "KPValue.h"


namespace kebap {

class KPObjectPrototype;
class KPOperator;
class KPSymbolTable;


class KPObjectPrototypeTable {
  public:
    KPObjectPrototypeTable();
    virtual ~KPObjectPrototypeTable();
    virtual void Merge(KPObjectPrototypeTable* Source);
    virtual void RegisterClass(const std::string& ClassName, KPObjectPrototype* ObjectPrototype);
    virtual void RegisterClass(KPObjectPrototype* ObjectPrototype);
    virtual KPObjectPrototype* CreateInstance(const std::string& ClassName);
    virtual KPObjectPrototype* ReferenceClass(const std::string& ClassName);
    virtual bool IsRegisteredClassName(const std::string& Name) const;
  protected:
    std::map<std::string, KPObjectPrototype*> fPrototypeTable;
};


class KPObjectPrototype {
  public:
    KPObjectPrototype(const std::string& InternalClassName);
    virtual ~KPObjectPrototype();
    virtual KPObjectPrototype* Clone() = 0;
    virtual void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList) ;
    virtual void Destruct() ;
    virtual int MethodIdOf(const std::string& MethodName);
    virtual int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    virtual int InvokeMethodByName(const std::string& MethodName, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    virtual int PropertyIdOf(const std::string& PropertyName);
    virtual int GetProperty(int PropertyId, KPValue& ReturnValue) ;
    virtual int GetPropertyByName(const std::string& PropertyName, KPValue& ReturnValue) ;
    virtual KPValue& EvaluateOperator(KPOperator* Operator, KPValue& LeftValue, KPValue& RightValue, KPSymbolTable* SymbolTable, KPValue& Result) ;
    virtual std::string InternalClassName() const;
    virtual std::string ObjectName() const;
    virtual void SetObjectName(const std::string& ObjectName);
  public:
    int fReferenceCount;
  protected:
    std::string fInternalClassName;
    std::string fObjectName;
  protected:
    enum {
	MethodId_Undefined = 0,
	fNumberOfMethods
    };
    enum {
	PropertyId_Undefined = 0,
	fNumberOfProperties
    };
};


}
#endif
