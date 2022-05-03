// KPValue.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <sstream>
#include <string>
#include <complex>
#include <cctype>
#include <cstdlib>
#include "KPObject.h"
#include "KPValue.h"

using namespace std;
using namespace kebap;

//#define DEBUG(x) x
#define DEBUG(x)


KPValue::KPValue()
{
    fType = ValueType_Void;

    fArrayLength = 0;
    fIsVariant = false;
    fIsLeftValue = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(bool BoolValue)
{
    fType = ValueType_Bool;
    fPrimitiveValue.fBoolValue = BoolValue;

    fArrayLength = 0;
    fIsVariant = false;
    fIsLeftValue = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(long LongValue)
{
    fType = ValueType_Long;
    fPrimitiveValue.fLongValue = LongValue;

    fArrayLength = 0;
    fIsVariant = false;
    fIsLeftValue = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(double DoubleValue)
{
    fType = ValueType_Double;
    fPrimitiveValue.fDoubleValue = DoubleValue;

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(complex<double> ComplexValue)
{
    fType = ValueType_Complex;
    fPrimitiveValue.fComplexValue = new complex<double>(ComplexValue);

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(const string& StringValue)
{
    fType = ValueType_String;
    fPrimitiveValue.fStringValue = new string(StringValue);

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(KPObjectPrototype* ObjectValue)
{
    fType = ValueType_Object;
    fPrimitiveValue.fObjectValue = ObjectValue;

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(KPValue* PointerValue)
{
    fType = ValueType_Pointer;
    fPrimitiveValue.fPointerValue = PointerValue;

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(const KPVariant& VariantValue)
{
    fType = ValueType_Void;

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = true;
    fIsArrayPointer = false;
}

KPValue::KPValue(const KPListValue& ListValue)
{
    fType = ValueType_List;
    fPrimitiveValue.fListValue = new KPListValue(ListValue);

    fArrayLength = 0;
    fIsLeftValue = false;
    fIsVariant = false;
    fIsArrayPointer = false;
}

KPValue::KPValue(const KPValue& Value)
{
    fType = Value.fType;
    if (fType == ValueType_Complex) {
	fPrimitiveValue.fComplexValue = new complex<double>(
	    *Value.fPrimitiveValue.fComplexValue
	);
    }
    else if (fType == ValueType_String) {
	fPrimitiveValue.fStringValue = new string(
	    *Value.fPrimitiveValue.fStringValue
	);
    }
    else if (fType == ValueType_List) {
	fPrimitiveValue.fListValue = new KPListValue(
	    *Value.fPrimitiveValue.fListValue
	);
    }
    else {
	fPrimitiveValue = Value.fPrimitiveValue;
    }

    fArrayLength = Value.fArrayLength;
    fIsLeftValue = Value.fIsLeftValue;
    fIsVariant = Value.fIsVariant;
    fIsArrayPointer = Value.fIsArrayPointer;
}

KPValue& KPValue::operator=(const KPValue& Value)
{
    if (fType != Value.fType) {
	if (fType == ValueType_Complex) {
	    delete fPrimitiveValue.fComplexValue;
	}
	else if (fType == ValueType_String) {
	    delete fPrimitiveValue.fStringValue;
	}
	else if (fType == ValueType_List) {
	    delete fPrimitiveValue.fListValue;
	}
	fType = Value.fType;

	if (Value.fType == ValueType_Complex) {
	    fPrimitiveValue.fComplexValue = new complex<double>(
		*Value.fPrimitiveValue.fComplexValue
	    );
	}
	else if (Value.fType == ValueType_String) {
	    fPrimitiveValue.fStringValue = new string(
		*Value.fPrimitiveValue.fStringValue
	    );
	}
	else if (Value.fType == ValueType_List) {
	    fPrimitiveValue.fListValue = new KPListValue(
		*Value.fPrimitiveValue.fListValue
	    );
	}
	else {
	    fPrimitiveValue = Value.fPrimitiveValue;
	}
    }
    else {
	if (fType == ValueType_Complex) {
	    *fPrimitiveValue.fComplexValue = (
		*Value.fPrimitiveValue.fComplexValue
            );
	}
	else if (fType == ValueType_String) {
	    *fPrimitiveValue.fStringValue = (
		*Value.fPrimitiveValue.fStringValue
            );
	}
	else if (fType == ValueType_List) {
	    *fPrimitiveValue.fListValue = (
		*Value.fPrimitiveValue.fListValue
	    );
	}
	else {
	    fPrimitiveValue = Value.fPrimitiveValue;
	}
    }

    fArrayLength = Value.fArrayLength;
    fIsLeftValue = Value.fIsLeftValue;
    fIsVariant = Value.fIsVariant;
    fIsArrayPointer = Value.fIsArrayPointer;

    return *this;
}

KPValue::~KPValue()
{
    if (fType == ValueType_Complex) {
	delete fPrimitiveValue.fComplexValue;
    }
    else if (fType == ValueType_String) {
	delete fPrimitiveValue.fStringValue;
    }
    else if (fType == ValueType_List) {
	delete fPrimitiveValue.fListValue;
    }
}

void KPValue::Refer()
{
    DEBUG(cout << "refering " << TypeName());
    DEBUG(if (fIsArrayPointer) { cout << "[]"; });
    DEBUG(cout << ", " << AsString() << ": ");

    if (fIsArrayPointer) {
	DEBUG(cout << "array elements refered" << endl);
	KPValue* Element = this->AsPointer();
	int Length = Element->ArrayLength();
	for (int i = 0; i < Length; i++) {
	    Element[i].Refer();
	}
	return;
    }

    if (fType == ValueType_List) {
	DEBUG(cout << "list: enter recursive" << endl);
	for (unsigned i = 0; i < fPrimitiveValue.fListValue->ListSize(); i++) {
	    (*fPrimitiveValue.fListValue)[i].Refer();
	}
	return;
    }

    if (fType == ValueType_Object) {
	fPrimitiveValue.fObjectValue->fReferenceCount++;
	DEBUG(cout << "ref=" << fPrimitiveValue.fObjectValue->fReferenceCount);
    }
    
    DEBUG(cout << endl);
}

void KPValue::Unrefer()
{
    DEBUG(cout << "unrefering " << TypeName());
    DEBUG(if (fIsArrayPointer) { cout << "[]"; });
    DEBUG(cout << ", " << AsString() << ": ");

    if (fIsArrayPointer) {
	DEBUG(cout << "array elements unrefered" << endl);
	KPValue* Element = this->AsPointer();
	int Length = Element->ArrayLength();
	for (int i = 0; i < Length; i++) {
	    Element[i].Unrefer();
	}
	return;
    }

    if (fType == ValueType_List) {
	DEBUG(cout << "list: enter recursive" << endl);
	for (unsigned i = 0; i < fPrimitiveValue.fListValue->ListSize(); i++) {
	    (*fPrimitiveValue.fListValue)[i].Unrefer();
	}
	return;
    }

    if (fType == ValueType_Object) {
	fPrimitiveValue.fObjectValue->fReferenceCount--;
	DEBUG(cout << "ref=" << fPrimitiveValue.fObjectValue->fReferenceCount);
    }
    
    DEBUG(cout << endl);
}

void KPValue::Destroy()
{
    DEBUG(cout << "destroying " << TypeName());
    DEBUG(if (fIsArrayPointer) { cout << "[]"; });
    DEBUG(cout << ", " << AsString() << ": ");

    if (fIsArrayPointer) {
	DEBUG(cout << "array elements destroyed" << endl);
	KPValue* Element = this->AsPointer();
	int Length = Element->ArrayLength();
	for (int i = 0; i < Length; i++) {
	    Element[i].Destroy();
	}
	return;
    }

    if (fType == ValueType_List) {
	DEBUG(cout << "list: enter recursive" << endl);
	for (unsigned i = 0; i < fPrimitiveValue.fListValue->ListSize(); i++) {
	    (*fPrimitiveValue.fListValue)[i].Destroy();
	}
	return;
    }

    DEBUG(cout << "enter Unref()" << endl);
    Unrefer();

    if (fType == ValueType_Object) {
	if (fPrimitiveValue.fObjectValue->fReferenceCount < 0) {
	    cerr << "INTERNAL ERROR: Object doubly destroyed: ";
	    cerr << TypeName() << ": " << AsString() << endl;
	}
	if (fPrimitiveValue.fObjectValue->fReferenceCount == 0) {
	    DEBUG(cout << "deleting " << TypeName() << ", " << AsString() << endl);
	    fPrimitiveValue.fObjectValue->Destruct();
	    delete fPrimitiveValue.fObjectValue;
	    fPrimitiveValue.fObjectValue = nullptr;
	}
    }
}

void KPValue::SetName(string Name)
{
     if (IsObject()) {
	 fPrimitiveValue.fObjectValue->SetObjectName(Name);
     }
}

void KPValue::Assign(const KPValue& Value) 
{
    if ((fType == ValueType_List) && (Value.fType == ValueType_List)) {
	bool IsVariant = fIsVariant;
	bool IsLeftValue = fIsLeftValue;
	int ArrayLength = fArrayLength;
	this->Destroy();

	const KPListValue& SourceListValue = Value.AsConstList();
	const vector<KPValue>& SourceList = SourceListValue.ConstValueList();
	this->operator=(KPValue(KPListValue(SourceList.size())));
	KPListValue& TargetListValue = AsList();
	
	for (unsigned i = 0; i < SourceList.size(); i++) {
	    TargetListValue[i].Assign(SourceList[i]);
	}
	if (SourceListValue.HasKeyIndex()) {
	    const vector<string>& KeyList = SourceListValue.KeyList();
	    for (unsigned i = 0; i < SourceList.size(); i++) {
		TargetListValue.SetKey(i, KeyList[i]);
	    }
	}

	fIsVariant = IsVariant;
	fArrayLength = ArrayLength;
	if (IsLeftValue) {
	    SetLeftValueFlag();
	}

	return;
    }

    if (fIsVariant && (Value.fType == ValueType_List)) {
	bool IsLeftValue = fIsLeftValue;
	int ArrayLength = fArrayLength;
	this->Destroy();

	this->operator=(KPValue(KPListValue()));

	fIsVariant = true;
	fIsLeftValue = IsLeftValue;
	fArrayLength = ArrayLength;

	this->Assign(Value);

	return;
    }

    if (fIsVariant) {
	bool IsLeftValue = fIsLeftValue;
	int ArrayLength = fArrayLength;
	this->Destroy();

	this->operator=(Value);

	if (fIsArrayPointer) {
	    // duplicate the array; will be deleted by SymbolTable //
	    KPValue* OldPointer = fPrimitiveValue.fPointerValue;
	    int Length = OldPointer->fArrayLength;
	    auto* NewPointer = new KPValue[Length];
	    for (int i = 0; i < Length; i++) {
		NewPointer[i] = OldPointer[i];
	    }
	    fPrimitiveValue.fPointerValue = NewPointer;
	}

	fIsVariant = true;
	fIsLeftValue = IsLeftValue;
	fArrayLength = ArrayLength;
	this->Refer();

	return;
    }

    // not variant //

    if (fType == ValueType_Void) {
	;
    }
    else if (fType == ValueType_Bool) {
        fPrimitiveValue.fLongValue = Value.AsBool();
    }
    else if (fType == ValueType_Long) {
        fPrimitiveValue.fLongValue = Value.AsLong();
    }
    else if (fType == ValueType_Double) {
        fPrimitiveValue.fDoubleValue = Value.AsDouble();
    }
    else if (fType == ValueType_Complex) {
        *fPrimitiveValue.fComplexValue = Value.AsComplex();
    }
    else if (fType == ValueType_String) {
        *fPrimitiveValue.fStringValue = Value.AsString();
    }
    else if (fType == ValueType_Object) {
        throw KPException() << "KPValue::Assign(): assigning object to object is not allowed";
    }
    else if (fType == ValueType_Pointer) {
        fPrimitiveValue.fPointerValue = Value.AsPointer();
    }
    else if (fType == ValueType_List) {
        throw KPException() << "KPValue::Assign(): list value expected";
    }
    else {
        throw KPException() << "KPValue::Assign(): unknown value type (internal error)";
    }
}

// high-speed version for intensive numerical calculation //
void KPValue::AssignDouble(double Value) 
{
    if (fType == ValueType_Double) {
        fPrimitiveValue.fDoubleValue = Value;
    }
    else {
        this->Assign(KPValue(Value));
    }
}

bool KPValue::IsVoid() const
{
    return (fType == ValueType_Void);
}

bool KPValue::IsBool() const
{
    return (fType == ValueType_Bool);
}

bool KPValue::IsLong() const
{
    return ((fType == ValueType_Long) || (fType == ValueType_Void));
}

bool KPValue::IsDouble() const
{
    return (fType == ValueType_Double);
}

bool KPValue::IsComplex() const
{
    return (fType == ValueType_Complex);
}

bool KPValue::IsString() const
{
    return (fType == ValueType_String);
}

bool KPValue::IsObject() const
{
    return (fType == ValueType_Object);
}

bool KPValue::IsPointer() const
{
    return (fType == ValueType_Pointer);
}

bool KPValue::IsList() const
{
    return (fType == ValueType_List);
}

bool KPValue::IsNumeric() const
{
    return (IsLong() || IsDouble() || IsComplex() || IsVoid());
}

bool KPValue::IsReal() const
{
    return (IsLong() || IsDouble() || IsVoid());
}

bool KPValue::IsObject(const string& InternalClassName) const
{
    return (
	(fType == ValueType_Object) &&
	(fPrimitiveValue.fObjectValue->InternalClassName() == InternalClassName)
    );
}

bool KPValue::AsBool() const 
{
    bool Result;

    switch (fType) {
      case ValueType_Void:
	Result = false;
        break;
      case ValueType_Bool:
	Result = fPrimitiveValue.fBoolValue;
        break;
      case ValueType_Long:
	Result = (AsLong() != 0);
        break;
      case ValueType_Double:
	Result = (AsDouble() != 0);
        break;
      case ValueType_Complex:
	Result = (AsComplex() != complex<double>(0));
        break;
      case ValueType_String:
	Result = (! AsString().empty());
        break;
      case ValueType_Object:
	Result = true;
        break;
      case ValueType_Pointer:
	Result = (AsPointer() != nullptr);
        break;
      case ValueType_List:
	Result = (AsConstList().ListSize() > 0);
	break;
      default:
          throw KPException() << "unknown value type (internal)";
    }
    
    return Result;
}

long KPValue::AsLong() const 
{
    long Value;
    string Remaining;
    bool IsConversionSuccessful = false;
    
    switch (fType) {
      case ValueType_Void:
	Value = 0;
        IsConversionSuccessful = true;
        break;
      case ValueType_Bool:
        Value = (fPrimitiveValue.fBoolValue ? 1 : 0);
        IsConversionSuccessful = true;
        break;
      case ValueType_Long:
        Value = fPrimitiveValue.fLongValue;
        IsConversionSuccessful = true;
        break;
      case ValueType_Double:
        Value = (long) fPrimitiveValue.fDoubleValue;
        IsConversionSuccessful = true;
        break;
      case ValueType_Complex:
        break;
      case ValueType_String:
	if (
	    (fPrimitiveValue.fStringValue->size() > 2) && 
	    (tolower((*fPrimitiveValue.fStringValue)[1]) == 'x')
	){
	    // Hex Number
	    istringstream ValueStream(fPrimitiveValue.fStringValue->substr(2, string::npos));
	    unsigned long HexValue;
	    if (ValueStream >> hex >> HexValue) {
		getline(ValueStream, Remaining);
		IsConversionSuccessful = Remaining.empty();
	    }
	    Value = HexValue;
	}
        else {
	    // Dec Number
	    istringstream ValueStream(*(fPrimitiveValue.fStringValue));
	    if (ValueStream >> Value) {
		getline(ValueStream, Remaining);
		IsConversionSuccessful = Remaining.empty();
	    }
        }
        break;
      case ValueType_Object:
        break;
      case ValueType_Pointer:
        Value = (long) fPrimitiveValue.fPointerValue;
        IsConversionSuccessful = true;
        break;
      case ValueType_List:
        Value = (long) fPrimitiveValue.fListValue->ValueList().size();
        IsConversionSuccessful = true;
        break;
      default:
        throw KPException() << "unknown value type (internal)";
    }

    if (! IsConversionSuccessful) {	
        throw KPException() << "integer value is expected: value = \"" << AsString() << "\"";
    }
    
    return Value;
}

double KPValue::AsDouble() const 
{
    double Value;
    string Remaining;
    bool IsConversionSuccessful = false;
    
    switch (fType) {
      case ValueType_Void:
	Value = 0;
        IsConversionSuccessful = true;
        break;
      case ValueType_Bool:
        Value = (fPrimitiveValue.fBoolValue ? 1.0 : 0.0);
        IsConversionSuccessful = true;
        break;
      case ValueType_Long:
        Value = (double) fPrimitiveValue.fLongValue;
        IsConversionSuccessful = true;
        break;
      case ValueType_Double:
        Value = fPrimitiveValue.fDoubleValue;
        IsConversionSuccessful = true;
        break;
      case ValueType_Complex:
        break;
      case ValueType_String:
        {
	    istringstream ValueStream(*(fPrimitiveValue.fStringValue));
	    if (ValueStream >> Value) {
		getline(ValueStream, Remaining);
		IsConversionSuccessful = Remaining.empty();
	    }
	}
        break;
      case ValueType_Object:
        break;
      case ValueType_Pointer:
        break;
      case ValueType_List:
        break;
      default:
        throw KPException() << "unknown value type (internal)";
    }

    if (! IsConversionSuccessful) {
        throw KPException() << "floating value is expected: value = \"" << AsString() << "\"";
    }
    
    return Value;
}

complex<double> KPValue::AsComplex() const 
{
    complex<double> Value;
    string Remaining;
    bool IsConversionSuccessful = false;
    
    switch (fType) {
      case ValueType_Void:
	Value = complex<double>((double) 0);
        IsConversionSuccessful = true;
        break;
      case ValueType_Bool:
	Value = complex<double>((fPrimitiveValue.fBoolValue ? 1.0 : 0.0));
        IsConversionSuccessful = true;
        break;
      case ValueType_Long:
	Value = complex<double>((double) fPrimitiveValue.fLongValue);
        IsConversionSuccessful = true;
        break;
      case ValueType_Double:
	Value = complex<double>(fPrimitiveValue.fDoubleValue);
        IsConversionSuccessful = true;
        break;
      case ValueType_Complex:
        Value = *fPrimitiveValue.fComplexValue;
        IsConversionSuccessful = true;
        break;
      case ValueType_String:
        break;
      case ValueType_Object:
        break;
      case ValueType_Pointer:
        break;
      case ValueType_List:
        break;
      default:
        throw KPException() << "unknown value type (internal)";
    }

    while (fType == ValueType_String) {
	double Real, Imag = 0;
	char Punctuator;
	istringstream is(fPrimitiveValue.fStringValue->c_str());
	if (!(is >> Real)) {
	    break;
	}
	if (is >> Punctuator) {
	    if (toupper(Punctuator) == 'I') {
		Imag = Real;
		Real = 0;
	    }
	    else if ((Punctuator == '+') || (Punctuator == '-')) {
		if (!(is >> Imag)) {
		    break;
		}
		if (Punctuator == '-') {
		    Imag *= -1;
		}
		if (! (is >> Punctuator) || (toupper(Punctuator) != 'I')) {
		    break;
		}
	    }
	}
	string Remaining;
	getline(is, Remaining);
	IsConversionSuccessful = Remaining.empty();

	Value = complex<double>(Real, Imag);
	break;
    }

    if (! IsConversionSuccessful) {
        throw KPException() << "complex value is expected: value = \"" << AsString() << "\"";
    }
    
    return Value;
}

string KPValue::AsString() const 
{
    ostringstream BufferStream;    
    string Value;
    
    switch (fType) {
      case ValueType_Void:
	Value = "";
        break;
      case ValueType_Bool:
        Value = (fPrimitiveValue.fBoolValue ? "true" : "false");
        break;
      case ValueType_Long:
	BufferStream << fPrimitiveValue.fLongValue;
        Value = BufferStream.str();
        break;
      case ValueType_Double:
        BufferStream << fPrimitiveValue.fDoubleValue;
        Value = BufferStream.str();
        break;
      case ValueType_Complex:
        {
	    double Real = fPrimitiveValue.fComplexValue->real();
	    double Imag = fPrimitiveValue.fComplexValue->imag();
#if 1
	    double R = abs(*fPrimitiveValue.fComplexValue);
	    if (fabs(Real / R) < 1e-15) { Real = 0; }
	    if (fabs(Imag / R) < 1e-15) { Imag = 0; }
#endif
	    if (Real != 0) {
		BufferStream << Real;
	    }
	    if (Imag < 0) {
		BufferStream << "-";
	    }
	    else if (Real != 0) {
		BufferStream << "+";
	    }
	    BufferStream << fabs(Imag) << "i";
	}
        Value = BufferStream.str();
        break;
      case ValueType_String:
        Value = *fPrimitiveValue.fStringValue;
        break;
      case ValueType_Object:
	BufferStream << AsObject()->InternalClassName();
        BufferStream << "@" << AsObject();
        Value = BufferStream.str();
        break;
      case ValueType_Pointer:
        BufferStream << fPrimitiveValue.fPointerValue;
        Value = BufferStream.str();
        break;
      case ValueType_List:
	Value = fPrimitiveValue.fListValue->AsString();
	break;
      default:
        throw KPException() << "unknown value type (internal)";
    }
    
    return Value;
}

string& KPValue::AsStringReference() const 
{
    if (fType != ValueType_String) {
        throw KPException() << "string value is expected";
    }

    return *fPrimitiveValue.fStringValue;
}

KPObjectPrototype* KPValue::AsObject() const 
{
    if (fType != ValueType_Object) {
        throw KPException() << "object value is expected";
    }
    
    return fPrimitiveValue.fObjectValue;
}

KPValue* KPValue::AsPointer() const 
{
    if ((fType == ValueType_Long) && (AsLong() == 0)) {
        return (KPValue*) fPrimitiveValue.fLongValue;
    }
    else if (fType != ValueType_Pointer) {
        throw KPException() << "pointer value is expected";
    }
    
    return fPrimitiveValue.fPointerValue;
}

KPListValue& KPValue::AsList() 
{
    if (fType != ValueType_List) {
        throw KPException() << "list value is expected";
    }

    return *fPrimitiveValue.fListValue;
}

const KPListValue& KPValue::AsConstList() const 
{
    if (fType != ValueType_List) {
        throw KPException() << "list value is expected";
    }

    return *fPrimitiveValue.fListValue;
}

vector<KPValue>& KPValue::AsValueList() 
{
    if (fType != ValueType_List) {
        throw KPException() << "list value is expected";
    }

    return fPrimitiveValue.fListValue->ValueList();
}

void KPValue::SetLeftValueFlag()
{
    fIsLeftValue = true;

    if (fType == ValueType_List) {
	vector<KPValue>& ValueList = fPrimitiveValue.fListValue->ValueList();
	for (unsigned i = 0; i < ValueList.size(); i++) {
	    ValueList[i].SetLeftValueFlag();
	}
    }
}

bool KPValue::IsLeftValue() const
{
    return fIsLeftValue;
}

void KPValue::SetVariantFlag()
{
    fIsVariant = true;
}

bool KPValue::IsVariant() const
{
    return fIsVariant;
}

void KPValue::SetArrayPointerFlag()
{
    fIsArrayPointer = true;
}

bool KPValue::IsArrayPointer() const
{
    return fIsArrayPointer;
}

void KPValue::SetArrayLength(int ArrayLength)
{
    fArrayLength = ArrayLength;
}

int KPValue::ArrayLength() const
{
    return fArrayLength;
}

string KPValue::TypeName() const
{
    string Value;
    switch (fType) {
      case ValueType_Bool:
	Value = "bool";
        break;
      case ValueType_Long:
	Value = "int";
        break;
      case ValueType_Double:
	Value = "float";
        break;
      case ValueType_Complex:
	Value = "complex";
        break;
      case ValueType_String:
	Value = "string";
        break;
      case ValueType_Object:
        Value = "object:" + AsObject()->InternalClassName();
        break;
      case ValueType_Pointer:
	Value = "pointer";
        break;
      case ValueType_List:
	Value = "list";
	break;
      case ValueType_Void:
	Value = "void";
	break;
      default:
        throw KPException() << "unknown value type (internal)";
    }
    
    return Value;
}

void KPValue::Dump(ostream& os) const
{
    os << TypeName();
    if (fIsVariant) {
	os << "{variant}";
    }
    if (fIsLeftValue) {
	os << "{left}";
    }
    if (fIsArrayPointer) {
	KPValue* Element = this->AsPointer();
	int Length = Element->ArrayLength();
	os << "[" << Length << "]";
    }
    os << " " << AsString();
}



KPListValue::KPListValue()
{
    fKeyList = nullptr;
    fKeyIndexTable = nullptr;
    fIndexKeyTable = nullptr;
}

KPListValue::KPListValue(unsigned InitialCapacity)
{
    fValueList.reserve(InitialCapacity);

    fKeyList = nullptr;
    fKeyIndexTable = nullptr;
    fIndexKeyTable = nullptr;
}

KPListValue::KPListValue(const vector<KPValue>& ValueList)
: fValueList(ValueList)
{
    fKeyList = nullptr;
    fKeyIndexTable = nullptr;
    fIndexKeyTable = nullptr;
}

KPListValue::KPListValue(const KPListValue& ListValue)
{
    fValueList = ListValue.fValueList;

    if (ListValue.fKeyList) {
	fKeyList = new vector<string>(*ListValue.fKeyList);
	fKeyIndexTable = new map<string, unsigned>(*ListValue.fKeyIndexTable);
	fIndexKeyTable = new map<unsigned, string>(*ListValue.fIndexKeyTable);
    }
    else {
	fKeyList = nullptr;
	fKeyIndexTable = nullptr;
	fIndexKeyTable = nullptr;
    }
}

KPListValue& KPListValue::operator=(const KPListValue& ListValue)
{
    delete fIndexKeyTable;
    delete fKeyIndexTable;
    delete fKeyList;

    fValueList = ListValue.fValueList;
    
    if (ListValue.fKeyList) {
	fKeyList = new vector<string>(*ListValue.fKeyList);
	fKeyIndexTable = new map<string, unsigned>(*ListValue.fKeyIndexTable);
	fIndexKeyTable = new map<unsigned, string>(*ListValue.fIndexKeyTable);
    }
    else {
	fKeyList = nullptr;
	fKeyIndexTable = nullptr;
	fIndexKeyTable = nullptr;
    }

    return *this;
}

KPListValue::~KPListValue()
{
    delete fIndexKeyTable;
    delete fKeyIndexTable;
    delete fKeyList;
}

unsigned KPListValue::ListSize() const
{
    return fValueList.size();
}

bool KPListValue::HasKeyIndex() const
{
    return (fKeyList != nullptr);
}

void KPListValue::AppendList(const KPListValue& ListValue)
{
    unsigned StartIndex = fValueList.size();
    fValueList.insert(
	fValueList.end(), 
	ListValue.ConstValueList().begin(), ListValue.ConstValueList().end()
    );

    if (ListValue.HasKeyIndex()) {
	const vector<string>& KeyList = ListValue.KeyList();
	for (unsigned i = 0; i < KeyList.size(); i++) {
	    SetKey(StartIndex + i, KeyList[i]);
	}
    }
}

unsigned KPListValue::AppendValue(const KPValue& Value)
{
    unsigned Index = fValueList.size();

    fValueList.push_back(Value);
    fValueList[Index].SetLeftValueFlag();
    fValueList[Index].SetVariantFlag();

    return Index;
}

vector<KPValue>& KPListValue::ValueList()
{
    return fValueList;
}

const vector<KPValue>& KPListValue::ConstValueList() const
{
    return fValueList;
}

const vector<string>& KPListValue::KeyList() const
{
    if (fKeyList == nullptr) {
	fKeyList = new vector<string>();
	fKeyIndexTable = new map<string, unsigned>();
	fIndexKeyTable = new map<unsigned, string>();
    }

    return *fKeyList;
}

void KPListValue::SetKey(unsigned Index, const string& Key)
{
    if (fKeyList == nullptr) {
	KeyList();
    }

    if (fKeyIndexTable->count(Key) == 0) {
	fKeyList->push_back(Key);
    }
    (*fKeyIndexTable)[Key] = Index;
    (*fIndexKeyTable)[Index] = Key;
}

string KPListValue::KeyOf(unsigned Index)
{
    if (fIndexKeyTable && fIndexKeyTable->count(Index) > 0) {
	return (*fIndexKeyTable)[Index];
    }
    else {
	return "";
    }
}

unsigned KPListValue::IndexOf(const string& Key)
{
    if (fKeyIndexTable->count(Key) == 0) {
	ValueOf(Key);
    }

    return (*fKeyIndexTable)[Key];
}

KPValue& KPListValue::ValueOf(unsigned Index)
{
    if (Index >= fValueList.size()) {
	unsigned OldSize = fValueList.size();
	KPVariant Variant;
	fValueList.resize(Index + 1, KPValue(Variant));

	for (unsigned i = OldSize; i < fValueList.size(); i++) {
	    fValueList[i].SetLeftValueFlag();
	}
    }

    return fValueList[Index];
}

KPValue& KPListValue::ValueOf(const string& Key)
{
    if (fKeyList == nullptr) {
	KeyList();
    }

    unsigned Index;
    auto Iterator = fKeyIndexTable->find(Key);
    if (Iterator != fKeyIndexTable->end()) {
	Index = (*Iterator).second;
    }
    else {
	KPVariant Variant;
	KPValue Value(Variant);
	Value.SetLeftValueFlag();

	Index = fValueList.size();
	fValueList.push_back(Value);

	SetKey(Index, Key);
    }

    return fValueList[Index];
}

string KPListValue::AsString() const
{
    string StringValue;

    StringValue = "{ ";
    for (unsigned i = 0; i < fValueList.size(); i++) {
	if (fIndexKeyTable) {
	    string Key = (*fIndexKeyTable)[i];
	    if (! Key.empty()) {
		StringValue += "\"" + Key + "\" => ";
	    }
	}

	if (fValueList[i].IsString()) {
	    StringValue += "\"" + fValueList[i].AsString() + "\"";
	}
	else {
	    StringValue += fValueList[i].AsString();
	}

	if (i < fValueList.size() - 1) {
	    StringValue += ", ";
	}
    }
    StringValue += " }";

    return StringValue;
}
