// KPObject.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "KPObject.h"
#include "KPOperator.h"
#include "KPSymbolTable.h"

using namespace std;
using namespace kebap;


KPObjectPrototypeTable::KPObjectPrototypeTable()
{
}

KPObjectPrototypeTable::~KPObjectPrototypeTable()
{
    map<string, KPObjectPrototype*>::iterator PrototypeIterator;
    for (
        PrototypeIterator = fPrototypeTable.begin();
        PrototypeIterator != fPrototypeTable.end();
        PrototypeIterator++
    ){
        KPObjectPrototype* Prototype = (*PrototypeIterator).second;
        delete Prototype;
    }
}

void KPObjectPrototypeTable::Merge(KPObjectPrototypeTable* Source)
{
    map<string, KPObjectPrototype*>& SourceTable = Source->fPrototypeTable;
    map<string, KPObjectPrototype*>::iterator PrototypeIterator;
    for (
        PrototypeIterator = SourceTable.begin();
        PrototypeIterator != SourceTable.end();
        PrototypeIterator++
    ){
	string ClassName = (*PrototypeIterator).first;
        KPObjectPrototype* Prototype = (*PrototypeIterator).second;
	RegisterClass(ClassName, Prototype->Clone());
    }
}

void KPObjectPrototypeTable::RegisterClass(const string& ClassName, KPObjectPrototype* ObjectPrototype)
{
    if (fPrototypeTable.count(ClassName) > 0) {
	delete fPrototypeTable[ClassName];
    }
	
    fPrototypeTable[ClassName] = ObjectPrototype;
}

void KPObjectPrototypeTable::RegisterClass(KPObjectPrototype* ObjectPrototype)
{
    string ClassName = ObjectPrototype->InternalClassName();
    RegisterClass(ClassName, ObjectPrototype);
}

KPObjectPrototype* KPObjectPrototypeTable::CreateInstance(const string& ClassName)
{
    if (fPrototypeTable.count(ClassName) == 0) {
        return nullptr;
    }
    
    KPObjectPrototype* Prototype = fPrototypeTable[ClassName];
    KPObjectPrototype* Instance = Prototype->Clone();

    return Instance;
}

KPObjectPrototype* KPObjectPrototypeTable::ReferenceClass(const string& ClassName)
{
    if (fPrototypeTable.count(ClassName) == 0) {
        return nullptr;
    }
    
    return fPrototypeTable[ClassName];
}

bool KPObjectPrototypeTable::IsRegisteredClassName(const string& Name) const
{
    if (fPrototypeTable.count(Name) == 0) {
        return false;
    }
    else {
        return true;
    }
}



KPObjectPrototype::KPObjectPrototype(const string& InternalClassName)
: fInternalClassName(InternalClassName)
{
    fReferenceCount = 0;

    ostringstream os;
    os << InternalClassName << "@" << hex << (void*) this;
    fObjectName = os.str();
}

KPObjectPrototype::~KPObjectPrototype()
{
}

string KPObjectPrototype::InternalClassName() const
{
    return fInternalClassName;
}

string KPObjectPrototype::ObjectName() const
{
    return fObjectName;
}

void KPObjectPrototype::SetObjectName(const string& ObjectName)
{
    fObjectName = ObjectName;
}

void KPObjectPrototype::Construct(const string& ClassName, vector<KPValue*>& ArgumentList) 
{
}

void KPObjectPrototype::Destruct() 
{
}

int KPObjectPrototype::MethodIdOf(const std::string& MethodName)
{
    return MethodId_Undefined;
}

int KPObjectPrototype::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    return 0;    
}

int KPObjectPrototype::InvokeMethodByName(const std::string& MethodName, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    return 0;
}

int KPObjectPrototype::PropertyIdOf(const std::string& PropertyName)
{
    return PropertyId_Undefined;
}

int KPObjectPrototype::GetProperty(int PropertyId, KPValue& ReturnValue) 
{
    return 0;
}

int KPObjectPrototype::GetPropertyByName(const std::string& PropertyName, KPValue& ReturnValue) 
{
    return 0;
}

KPValue& KPObjectPrototype::EvaluateOperator(KPOperator* Operator, KPValue& LeftValue, KPValue& RightValue, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    return Operator->Evaluate(LeftValue, RightValue, SymbolTable, Result);
}

