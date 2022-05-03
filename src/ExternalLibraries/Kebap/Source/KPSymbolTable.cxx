// KPSymbolTable.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <complex>
#include <vector>
#include "KPValue.h"
#include "KPObject.h"
#include "KPFunction.h"
#include "KPBuiltinFunction.h"
#include "KPSymbolTable.h"

using namespace std;
using namespace kebap;


KPNameTable* KPNameTable::fInstance = nullptr;

KPNameTable* KPNameTable::GetInstance()
{
    if (fInstance == nullptr) {
        fInstance = new KPNameTable();
    }

    return fInstance;
}

KPNameTable::KPNameTable()
{
    fNextId = 1;
}

KPNameTable::~KPNameTable()
{
}

long KPNameTable::NameToId(const string& Name)
{
    if (fIdTable.count(Name) == 0) {
        fIdTable[Name] = fNextId;
	fNextId++;
    }
    
    return fIdTable[Name];
}

string KPNameTable::IdToName(long Id)
{
    map<string, long>::iterator Entry;
    for (
        Entry = fIdTable.begin();
        Entry != fIdTable.end();
        Entry++
    ){
        if ((*Entry).second == Id) {
            return (*Entry).first;
        }
    }
    
    return string("");
}



KPSymbolTable::KPSymbolTable(KPObjectPrototypeTable* ObjectPrototypeTable, KPBuiltinFunctionTable* BuiltinFunctionTable)
{
    fNameTable = KPNameTable::GetInstance();

    fObjectPrototypeTable = ObjectPrototypeTable;
    fBuiltinFunctionTable = BuiltinFunctionTable;

    fOriginalFunctionTable = new map<long, KPFunction*>;
    fFunctionTable = fOriginalFunctionTable;

    fVariableCount = 0;
    fCurrentBlockDepth = 0;
}
    
KPSymbolTable::KPSymbolTable(KPSymbolTable* SymbolTable, int ImportDepth)
{
    fOriginalFunctionTable = nullptr;

    fVariableCount = 0;
    fCurrentBlockDepth = 0;

    Import(SymbolTable, ImportDepth);
}

KPSymbolTable::~KPSymbolTable()
{
    delete fOriginalFunctionTable;

    // delete from the tail for dependencies //
    for (int i = (int) fGlobalVariableList.size() - 1; i >= 0; i--) {
        KPValue* Variable = fGlobalVariableList[i];
	Variable->Destroy();
	if (Variable->IsArrayPointer()) {
	    KPValue* ElementArray = Variable->AsPointer();
	    delete[] ElementArray;
	}
	delete Variable;
    }
}

bool KPSymbolTable::IsTypeName(const string& Symbol) const
{
    if (
	(Symbol == "int") || (Symbol == "long") ||
	(Symbol == "float") || (Symbol == "double") ||
	(Symbol == "complex") ||
	(Symbol == "string")  || (Symbol == "list") ||
	(Symbol == "pointer") || 
	(Symbol == "variant") || (Symbol == "var") ||
	(Symbol == "bool")
    ){
        return true;
    }

    else if (fObjectPrototypeTable != nullptr) {
        return fObjectPrototypeTable->IsRegisteredClassName(Symbol);
    }
    
    return false;
}

KPValue* KPSymbolTable::CreateObject(const string& TypeName, int Length)
{
    KPValue* Value;
    if (Length > 0) {
	Value = new KPValue[Length];
    }
    else {
	Value = new KPValue;
    }
    
    for (int i = 0; i < ((Length > 0) ? Length : 1); i++) {
	if (TypeName == "void") {
	    Value[i] = KPValue();
	}
	else if (TypeName == "bool") {
	    Value[i] = KPValue(false);
	}
	else if ((TypeName == "int") || (TypeName == "long")) {
	    Value[i] = KPValue((long) 0);
	}
	else if ((TypeName == "float") || (TypeName == "double")) {
	    Value[i] = KPValue((double) 0);
	}
	else if (TypeName == "complex") {
	    Value[i] = KPValue(complex<double>(0));
	}
	else if (TypeName == "string") {
	    Value[i] = KPValue(string(""));
	}
	else if (TypeName == "list") {
	    Value[i] = KPValue(KPListValue());
	}
	else if ((TypeName == "pointer") || (TypeName == "*")) {
	    Value[i] = KPValue((KPValue*) nullptr);
	}
	else if ((TypeName == "variant") || (TypeName == "var")) {
	    KPVariant Variant;
	    Value[i] = KPValue(Variant);
	}
	else {
	    KPObjectPrototype* Object;
	    Object = fObjectPrototypeTable->CreateInstance(TypeName);
	    if (Object) {
		Value[i] = KPValue(Object);
	    }
	    else {
		(Length > 0) ? delete[] Value : delete Value;
		return nullptr;
	    }
	}

	Value[i].SetArrayLength(Length - i);
    }

    return Value;
}

void KPSymbolTable::EnterBlock()
{
    fVariableCountList.push_back(fVariableCount);

    fVariableCount = 0;
    fCurrentBlockDepth++;
}

void KPSymbolTable::ExitBlock() 
{
    for (; fVariableCount > 0; fVariableCount--) {
        KPValue* Variable = fVariableEntryList.back().second;

	Variable->Destroy();
	if (Variable->IsArrayPointer()) {
	    KPValue* ElementArray = Variable->AsPointer();
	    delete[] ElementArray;
	}
	delete Variable;

        fVariableEntryList.pop_back();
    }

    fVariableCount = fVariableCountList.back();
    fVariableCountList.pop_back();
    fCurrentBlockDepth--;
}

long KPSymbolTable::NameToId(const string& Name)
{
    return fNameTable->NameToId(Name);
}

string KPSymbolTable::IdToName(long Id)
{
    return fNameTable->IdToName(Id);
}

long KPSymbolTable::RegisterVariable(const std::string& Name, const KPValue& InitialValue)
{
    long VariableId = NameToId(Name);

    KPValue* Variable;
    if (InitialValue.IsObject()) {
	KPObjectPrototype* Object = InitialValue.AsObject()->Clone();
	Variable = new KPValue(Object);
    }
    else {
	Variable = new KPValue(InitialValue);
    }
	
    return RegisterVariable(VariableId, Variable);
}

long KPSymbolTable::RegisterVariable(const string& Name, KPValue* Variable)
{
    long VariableId = NameToId(Name);
    return RegisterVariable(VariableId, Variable);
}

long KPSymbolTable::RegisterVariable(long VariableId, KPValue* Variable)
{
    Variable->SetLeftValueFlag();
    Variable->Refer();

    string VariableName = IdToName(VariableId);

    pair<long, KPValue*> VariableEntry(VariableId, Variable);
    fVariableEntryList.push_back(VariableEntry);
    fVariableCount++;

    if (fCurrentBlockDepth == 0) {
	fGlobalVariableList.push_back(Variable);
    }

    return VariableId;
}

KPValue* KPSymbolTable::GetVariable(long VariableId)
{
    KPValue* Variable = nullptr;

    vector<pair<long, KPValue*> >::reverse_iterator VariableEntry;

    for (
	VariableEntry = fVariableEntryList.rbegin();
	VariableEntry != fVariableEntryList.rend();
	VariableEntry++
    ){
	if ((*VariableEntry).first == VariableId) {
	    Variable = (*VariableEntry).second;
	    break;
	}
    }

    return Variable;
}
    
long KPSymbolTable::Import(KPSymbolTable* SymbolTable, int Depth)
{
    long NumberOfImported = 0;
    vector<long>::const_iterator VariableCountIterator;
    vector<pair<long, KPValue*> >::const_iterator VariableEntryIterator;

    VariableCountIterator = SymbolTable->fVariableCountList.begin();
    VariableEntryIterator = SymbolTable->fVariableEntryList.begin();
    
    for (int i = 0; i < Depth; i++) {
        long NumberOfVariables;
	if (VariableCountIterator != SymbolTable->fVariableCountList.end()) {
	    NumberOfVariables = *VariableCountIterator;
	}
	else {
	    NumberOfVariables = SymbolTable->fVariableCount;
	}

	for (int j = 0; j < NumberOfVariables; j++) {
            fVariableEntryList.push_back(*VariableEntryIterator);

	    // Imported variables are not refer()ed //
            //VariableEntryIterator->second->Refer();

            VariableEntryIterator++;
            NumberOfImported++;
        }
	
        fVariableCountList.push_back(NumberOfVariables);

	if (VariableCountIterator != SymbolTable->fVariableCountList.end()) {
	    VariableCountIterator++;
	}
	else {
	    break;
	}
    }
        
    fNameTable = SymbolTable->fNameTable;
    fObjectPrototypeTable = SymbolTable->fObjectPrototypeTable;
    fBuiltinFunctionTable = SymbolTable->fBuiltinFunctionTable;
    fFunctionTable = SymbolTable->fFunctionTable;

    return NumberOfImported;
}

void KPSymbolTable::RegisterFunction(long FunctionId, KPFunction* Function)
{
    (*fFunctionTable)[FunctionId] = Function;
}
                   
KPFunction* KPSymbolTable::GetFunction(long FunctionId)
{
    if (fFunctionTable->count(FunctionId) == 0) {
        return nullptr;
    }

    return (*fFunctionTable)[FunctionId];
}

KPObjectPrototypeTable* KPSymbolTable::ObjectPrototypeTable()
{
    return fObjectPrototypeTable;
}

KPBuiltinFunctionTable* KPSymbolTable::BuiltinFunctionTable()
{
    return fBuiltinFunctionTable;
}
