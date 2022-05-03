// KPBuiltinFunction.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include <map>
#include "KPObject.h"
#include "KPValue.h"
#include "KPBuiltinFunction.h"

using namespace std;
using namespace kebap;


KPBuiltinFunctionTable::KPBuiltinFunctionTable()
{
}

KPBuiltinFunctionTable::~KPBuiltinFunctionTable()
{
    for (unsigned i = 0; i < fPrototypeList.size(); i++) {
	delete fPrototypeList[i];
    }
}

void KPBuiltinFunctionTable::Merge(KPBuiltinFunctionTable* Source)
{
    for (unsigned i = 0; i < Source->fPrototypeList.size(); i++) {
	RegisterStaticObject(Source->fPrototypeList[i]->Clone());
    }
}

void KPBuiltinFunctionTable::RegisterStaticObject(KPObjectPrototype* Prototype)
{
    fPrototypeList.push_back(Prototype);
}

void KPBuiltinFunctionTable::RegisterFunctionId(const std::string& FunctionName, long FunctionId)
{
    if (fClassIdTable.count(FunctionId) > 0) {
	return;
    }

    int ClassId, MethodId;
    for (ClassId = (int) fPrototypeList.size() - 1; ClassId >= 0; ClassId--) {
	MethodId = fPrototypeList[ClassId]->MethodIdOf(FunctionName);
	if (MethodId > 0) {
	    fClassIdTable[FunctionId] = ClassId;
	    fMethodIdTable[FunctionId] = MethodId;
	    break;
	}
    }

    fMethodNameTable[FunctionId] = FunctionName;
}

int KPBuiltinFunctionTable::Execute(long FunctionId, vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;

    auto ClassIdEntry = fClassIdTable.find(FunctionId);
    if (ClassIdEntry != fClassIdTable.end()) {
	long ClassId = (*ClassIdEntry).second;
	long MethodId = fMethodIdTable[FunctionId];

	Result = fPrototypeList[ClassId]->InvokeMethod(
	    MethodId, ArgumentList, ReturnValue
	);
    }

    if (Result == 0) {
	string MethodName = fMethodNameTable[FunctionId];
	for (int i = (int) fPrototypeList.size() - 1; i >= 0; i--) {
	    Result = fPrototypeList[i]->InvokeMethodByName(
		MethodName, ArgumentList, ReturnValue
	    );
	    if (Result != 0) {
		break;
	    }
	}
    }

    return Result;
}
