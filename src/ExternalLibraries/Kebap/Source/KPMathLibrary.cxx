// KPMathLibrary.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <complex>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include "KPObject.h"
#include "KPMathLibrary.h"

using namespace std;
using namespace kebap;


template<class T> inline static T sqr(const T& x) { return x*x; }


KPMathObject::KPMathObject()
: KPObjectPrototype("Math")
{
}

KPMathObject::~KPMathObject()
{
}

KPObjectPrototype* KPMathObject::Clone()
{
    return new KPMathObject();
}

int KPMathObject::MethodIdOf(const std::string& MethodName)
{
    if (MethodName == "sin") {
        return MethodId_Sin;
    }
    else if (MethodName == "cos") {
        return MethodId_Cos;
    }
    else if (MethodName == "tan") {
        return MethodId_Tan;
    }
    else if (MethodName == "asin") {
        return MethodId_Asin;
    }
    else if (MethodName == "acos") {
        return MethodId_Acos;
    }
    else if (MethodName == "atan") {
        return MethodId_Atan;
    }
    else if (MethodName == "atan2") {
        return MethodId_Atan2;
    }
    else if (MethodName == "exp") {
        return MethodId_Exp;
    }
    else if (MethodName == "log") {
        return MethodId_Log;
    }
    else if (MethodName == "log10") {
        return MethodId_Log10;
    }
    else if (MethodName == "sqrt") {
        return MethodId_Sqrt;
    }
    else if (MethodName == "abs") {
        return MethodId_Abs;
    }
    else if (MethodName == "arg") {
        return MethodId_Arg;
    }
    else if (MethodName == "real") {
        return MethodId_Real;
    }
    else if (MethodName == "imag") {
        return MethodId_Imag;
    }
    else if (MethodName == "round") {
        return MethodId_Round;
    }
    else if (MethodName == "trunc") {
        return MethodId_Trunc;
    }
    else if (MethodName == "ceil") {
        return MethodId_Ceil;
    }
    else if (MethodName == "floor") {
        return MethodId_Floor;
    }
    else if (MethodName == "srand") {
        return MethodId_Srand;
    }
    else if (MethodName == "rand") {
        return MethodId_Rand;
    }
    else if (MethodName == "sinc") {
        return MethodId_Sinc;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPMathObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() == 1) && ArgumentList[0]->IsList()) {
	ReturnValue = KPValue(KPListValue());
	vector<KPValue>& InputList = ArgumentList[0]->AsValueList();
	vector<KPValue>& ResultList = ReturnValue.AsValueList();

	KPValue Result;
	vector<KPValue*> InputArgumentList;
	InputArgumentList.push_back(&InputList[0]);

	for (unsigned i = 0; i < InputList.size(); i++) {
	    InputArgumentList[0] = &InputList[i];
	    if (this->InvokeMethod(MethodId, InputArgumentList, Result)) {
		ResultList.push_back(Result);
	    }
	    else {
		break;
	    }
	}
	return ResultList.size();
    }


    int Result = 0;

    switch (MethodId) {
      case MethodId_Sin:
        Result = Sin(ArgumentList, ReturnValue);
        break;
      case MethodId_Cos:
        Result = Cos(ArgumentList, ReturnValue);
        break;
      case MethodId_Tan:
        Result = Tan(ArgumentList, ReturnValue);
        break;
      case MethodId_Asin:
        Result = Asin(ArgumentList, ReturnValue);
        break;  
      case MethodId_Acos:
        Result = Acos(ArgumentList, ReturnValue);
        break;
      case MethodId_Atan:
        Result = Atan(ArgumentList, ReturnValue);
        break;
      case MethodId_Atan2:
        Result = Atan2(ArgumentList, ReturnValue);
        break;
      case MethodId_Exp:
        Result = Exp(ArgumentList, ReturnValue);
        break;
      case MethodId_Log:
        Result = Log(ArgumentList, ReturnValue);
        break;
      case MethodId_Log10:
        Result = Log10(ArgumentList, ReturnValue);
        break;
      case MethodId_Sqrt:
        Result = Sqrt(ArgumentList, ReturnValue);
        break;
      case MethodId_Abs:
        Result = Abs(ArgumentList, ReturnValue);
        break;
      case MethodId_Arg:
        Result = Arg(ArgumentList, ReturnValue);
        break;
      case MethodId_Real:
        Result = Real(ArgumentList, ReturnValue);
        break;
      case MethodId_Imag:
        Result = Imag(ArgumentList, ReturnValue);
        break;
      case MethodId_Round:
        Result = Round(ArgumentList, ReturnValue);
        break;
      case MethodId_Trunc:
        Result = Trunc(ArgumentList, ReturnValue);
        break;
      case MethodId_Ceil:
        Result = Ceil(ArgumentList, ReturnValue);
        break;
      case MethodId_Floor:
        Result = Floor(ArgumentList, ReturnValue);
        break;
      case MethodId_Srand:
        Result = Srand(ArgumentList, ReturnValue);
        break;
      case MethodId_Rand:
        Result = Rand(ArgumentList, ReturnValue);
        break;
      case MethodId_Sinc:
        Result = Sinc(ArgumentList, ReturnValue);
        break;
      default:
	Result = 0;
    }

    return Result;
}

int KPMathObject::Sin(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "sin(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(sin(ArgumentList[0]->AsComplex()));
    }
    else {
	ReturnValue = KPValue(sin(ArgumentList[0]->AsDouble()));
    }

    return 1;
}

int KPMathObject::Cos(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "cos(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(cos(ArgumentList[0]->AsComplex()));
    }
    else {
	ReturnValue = KPValue(cos(ArgumentList[0]->AsDouble()));
    }

    return 1;
}

int KPMathObject::Tan(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "tan(): invalid number of argument[s]";
    }

    if (ArgumentList[0]->IsComplex()) {
	if (abs(cos(ArgumentList[0]->AsComplex())) == 0) {
	    throw KPException() << "tan(): invalid argument";
	}
	ReturnValue = KPValue(tan(ArgumentList[0]->AsComplex()));
    }
    else {
	if (cos(ArgumentList[0]->AsDouble()) == 0) {
	    throw KPException() << "tan(): invalid argument";
	}
	ReturnValue = KPValue(tan(ArgumentList[0]->AsDouble()));
    }

    return 1;
}

int KPMathObject::Asin(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "asin(): invalid number of argument[s]";
    }
    
    // no asin(complex) available in the standard library //

    double x = ArgumentList[0]->AsDouble();
    if ((x < -1.0) || (x > 1.0)) {
	throw KPException() << "asin(): invalid argument";
    }
    ReturnValue = KPValue(asin(x));

    return 1;
}

int KPMathObject::Acos(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "acos(): invalid number of argument[s]";
    }
    
    // no acos(complex) available in the standard library //

    double x = ArgumentList[0]->AsDouble();
    if ((x < -1.0) || (x > 1.0)) {
	throw KPException() << "acos(): invalid argument";
    }
    ReturnValue = KPValue(acos(x));

    return 1;
}

int KPMathObject::Atan(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "atan(): invalid number of argument[s]";
    }
    
    // no atan(complex) available in the standard library //

    ReturnValue = KPValue(atan(ArgumentList[0]->AsDouble()));

    return 1;
}

int KPMathObject::Atan2(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 2) {
        throw KPException() <<
	    "atan2(double y, double x): invalid number of argument[s]";
    }
    
    // no atan2(complex) available in the standard library //

    if ((! ArgumentList[0]->IsList()) && (! ArgumentList[1]->IsList())) {
	ReturnValue = KPValue(atan2(
            ArgumentList[0]->AsDouble(), ArgumentList[1]->AsDouble()
	));

	return 1;
    }

    vector<KPValue>& YList = ArgumentList[0]->AsValueList();
    vector<KPValue>& XList = ArgumentList[1]->AsValueList();
    if (YList.size() != XList.size()) {
	throw KPException() <<
	    "atan2(): inconsistent length of argument lists";
    }
    
    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& ResultList = ReturnValue.AsValueList();
    for (unsigned i = 0; i < XList.size(); i++) {
	ResultList.push_back(KPValue(atan2(
	    YList[i].AsDouble(), XList[i].AsDouble()
	)));
    }

    return ResultList.size();
}

int KPMathObject::Exp(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "exp(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(exp(ArgumentList[0]->AsComplex()));
    }
    else {
	ReturnValue = KPValue(exp(ArgumentList[0]->AsDouble()));
    }

    return 1;
}

int KPMathObject::Log(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "log(): invalid number of argument[s]";
    }

    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(log(ArgumentList[0]->AsComplex()));
    }
    else {
	double x = ArgumentList[0]->AsDouble();
	if (x <= 0) {
	    throw KPException() << "log(): invalid argument";
	}
	ReturnValue = KPValue(log(x));
    }

    return 1;
}

int KPMathObject::Log10(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "log10(): invalid number of argument[s]";
    }

    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(log10(ArgumentList[0]->AsComplex()));
    }
    else {
	double x = ArgumentList[0]->AsDouble();
	if (x <= 0) {
	    throw KPException() << "log10(): invalid argument";
	}
	ReturnValue = KPValue(log10(x));
    }

    return 1;
}

int KPMathObject::Sqrt(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "sqrt(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(sqrt(ArgumentList[0]->AsComplex()));
    }
    else {
	double x = ArgumentList[0]->AsDouble();
	if (x < 0) {
	    throw KPException() << "sqrt(): invalid argument";
	}
	ReturnValue = KPValue((double) sqrt(x));
    }

    return 1;
}

int KPMathObject::Abs(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "abs(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(abs(ArgumentList[0]->AsComplex()));
    }
    else if (ArgumentList[0]->IsLong()) {
	ReturnValue = KPValue((long) abs(ArgumentList[0]->AsLong()));
    }
    else {
	ReturnValue = KPValue(fabs(ArgumentList[0]->AsDouble()));
    }

    return 1;
}

int KPMathObject::Arg(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "arg(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue((double) arg(ArgumentList[0]->AsComplex()));
    }
    else {
	ReturnValue = KPValue(
	    (double) ((ArgumentList[0]->AsDouble() < 0) ? M_PI : 0)
	);
    }

    return 1;
}

int KPMathObject::Real(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "real(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(ArgumentList[0]->AsComplex().real());
    }
    else {
	ReturnValue = *ArgumentList[0];
    }

    return 1;
}

int KPMathObject::Imag(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "imag(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	ReturnValue = KPValue(ArgumentList[0]->AsComplex().imag());
    }
    else if (ArgumentList[0]->IsLong()) {
	ReturnValue = KPValue((long) 0);
    }
    else {
	ReturnValue = KPValue((double) 0);
    }

    return 1;
}

int KPMathObject::Round(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "round(): invalid number of argument[s]";
    }
    
    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "round(): invalid argument";
    }

    ReturnValue = KPValue(round(ArgumentList[0]->AsDouble()));

    return 1;
}

int KPMathObject::Trunc(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "trunc(): invalid number of argument[s]";
    }
    
    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "trunc(): invalid argument";
    }

    ReturnValue = KPValue(trunc(ArgumentList[0]->AsDouble()));

    return 1;
}

int KPMathObject::Ceil(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "ceil(): invalid number of argument[s]";
    }
    
    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "ceil(): invalid argument";
    }

    ReturnValue = KPValue(ceil(ArgumentList[0]->AsDouble()));

    return 1;
}

int KPMathObject::Floor(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "floor(): invalid number of argument[s]";
    }
    
    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "floor(): invalid argument";
    }

    ReturnValue = KPValue(floor(ArgumentList[0]->AsDouble()));

    return 1;
}

int KPMathObject::Srand(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "srand(int): invalid number of argument[s]";
    }

    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "srand(): invalid argument";
    }

    srand48(ArgumentList[0]->AsLong());

    return 1;
}

int KPMathObject::Rand(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    ReturnValue = KPValue((double) drand48());

    return 1;
}

int KPMathObject::Sinc(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "sinc(): invalid number of argument[s]";
    }
    
    if (ArgumentList[0]->IsComplex()) {
	complex<double> x = M_PI * ArgumentList[0]->AsComplex();
	complex<double> Sin = sin(x);
	complex<double> Sinc = abs(Sin - x) > 0 ? (Sin/x) : 1;
	ReturnValue = KPValue(Sinc);
    }
    else {
	double x = M_PI * ArgumentList[0]->AsDouble();
	double Sin = sin(x);
	double Sinc = fabs(Sin - x) > 0 ? (Sin/x) : 1;
	ReturnValue = KPValue(Sinc);
    }

    return 1;
}



KPListMathObject::KPListMathObject()
: KPObjectPrototype("ListMath")
{
}

KPListMathObject::~KPListMathObject()
{
}

KPObjectPrototype* KPListMathObject::Clone()
{
    return new KPListMathObject();
}

int KPListMathObject::MethodIdOf(const std::string& MethodName)
{
    if (MethodName == "length") {
        return MethodId_Length;
    }
    else if (MethodName == "min") {
        return MethodId_Min;
    }
    else if (MethodName == "max") {
        return MethodId_Max;
    }
    else if (MethodName == "sum") {
        return MethodId_Sum;
    }
    else if ((MethodName == "mean") || (MethodName == "average")) {
        return MethodId_Mean;
    }
    else if ((MethodName == "deviation") || (MethodName == "rms")) {
        return MethodId_Deviation;
    }
    else if (MethodName == "delta") {
        return MethodId_Delta;
    }
    else if (MethodName == "sigma") {
        return MethodId_Sigma;
    }
    else if (MethodName == "zeros") {
        return MethodId_Zeros;
    }
    else if (MethodName == "ones") {
        return MethodId_Ones;
    }
    else if (MethodName == "find") {
        return MethodId_Find;
    }
    else if (MethodName == "count") {
        return MethodId_Count;
    }
    else if (MethodName == "divide") {
        return MethodId_Divide;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPListMathObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;

    switch (MethodId) {
      case MethodId_Length:
        Result = Length(ArgumentList, ReturnValue);
        break;
      case MethodId_Min:
        Result = Min(ArgumentList, ReturnValue);
        break;
      case MethodId_Max:
        Result = Max(ArgumentList, ReturnValue);
        break;
      case MethodId_Sum:
        Result = Sum(ArgumentList, ReturnValue);
        break;
      case MethodId_Mean:
        Result = Mean(ArgumentList, ReturnValue);
        break;
      case MethodId_Deviation:
        Result = Deviation(ArgumentList, ReturnValue);
        break;
      case MethodId_Delta:
        Result = Delta(ArgumentList, ReturnValue);
        break;
      case MethodId_Sigma:
        Result = Sigma(ArgumentList, ReturnValue);
        break;
      case MethodId_Zeros:
        Result = Zeros(ArgumentList, ReturnValue);
        break;
      case MethodId_Ones:
        Result = Ones(ArgumentList, ReturnValue);
        break;
      case MethodId_Find:
        Result = Find(ArgumentList, ReturnValue);
        break;
      case MethodId_Count:
        Result = Count(ArgumentList, ReturnValue);
        break;
      case MethodId_Divide:
        Result = Divide(ArgumentList, ReturnValue);
        break;
      default:
	Result = 0;
    }

    return Result;
}

int KPListMathObject::Length(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "length(): invalid argument[s]";
    }
    
    const KPListValue& ListValue = ArgumentList[0]->AsConstList();
    const vector<KPValue>& ValueList = ListValue.ConstValueList();
    unsigned ListLength = ValueList.size();

    ReturnValue = KPValue((long) ListLength);

    return 1;
}

int KPListMathObject::Min(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "min(): invalid argument[s]";
    }
    
    KPListValue& ListValue = ArgumentList[0]->AsList();
    vector<KPValue>& ValueList = ListValue.ValueList();
    unsigned ListLength = ValueList.size();

    if (ListLength == 0) {
        throw KPException() << "min(): empty list";
    }

    //...BUG: check not only the first element ...//
    if (ValueList[0].IsList()) {
	ReturnValue = KPValue(KPListValue());
	vector<KPValue>& ResultList = ReturnValue.AsValueList();
	for (unsigned i = 0; i < ListLength; i++) {
	    KPValue SubReturnValue;
	    vector<KPValue*> SubArgumentList;
	    SubArgumentList.push_back(&ValueList[i]);
	    this->Min(SubArgumentList, SubReturnValue);
	    ResultList.push_back(SubReturnValue);
	}
    }
    else {
	double Value = ValueList[0].AsDouble();
	for (unsigned i = 1; i < ListLength; i++) {
	    Value = min(Value, ValueList[i].AsDouble());
	}
	ReturnValue = KPValue(Value);
    }

    return 1;
}

int KPListMathObject::Max(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "max(): invalid argument[s]";
    }
    
    KPListValue& ListValue = ArgumentList[0]->AsList();
    vector<KPValue>& ValueList = ListValue.ValueList();
    unsigned ListLength = ValueList.size();

    if (ListLength == 0) {
        throw KPException() << "max(): empty list";
    }

    //...BUG: check not only the first element ...//
    if (ValueList[0].IsList()) {
	ReturnValue = KPValue(KPListValue());
	vector<KPValue>& ResultList = ReturnValue.AsValueList();
	for (unsigned i = 0; i < ListLength; i++) {
	    KPValue SubReturnValue;
	    vector<KPValue*> SubArgumentList;
	    SubArgumentList.push_back(&ValueList[i]);
	    this->Max(SubArgumentList, SubReturnValue);
	    ResultList.push_back(SubReturnValue);
	}
    }
    else {
	double Value = ValueList[0].AsDouble();
	for (unsigned i = 1; i < ListLength; i++) {
	    Value = max(Value, ValueList[i].AsDouble());
	}
	ReturnValue = KPValue(Value);
    }

    return 1;
}

int KPListMathObject::Sum(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "sum(): invalid argument[s]";
    }
    
    KPListValue& ListValue = ArgumentList[0]->AsList();
    vector<KPValue>& ValueList = ListValue.ValueList();
    unsigned ListLength = ValueList.size();

    if (ListLength == 0) {
        throw KPException() << "sum(): empty list";
    }

    //...BUG: check not only the first element ...//
    if (ValueList[0].IsList()) {
	ReturnValue = KPValue(KPListValue());
	vector<KPValue>& ResultList = ReturnValue.AsValueList();
	for (unsigned i = 0; i < ListLength; i++) {
	    KPValue SubReturnValue;
	    vector<KPValue*> SubArgumentList;
	    SubArgumentList.push_back(&ValueList[i]);
	    this->Sum(SubArgumentList, SubReturnValue);
	    ResultList.push_back(SubReturnValue);
	}
    }
    else {
	double Value = 0;
	for (unsigned i = 0; i < ListLength; i++) {
	    Value += ValueList[i].AsDouble();
	}
	ReturnValue = KPValue(Value);
    }

    return 1;
}

int KPListMathObject::Mean(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "mean(): invalid argument[s]";
    }
    
    KPListValue& ListValue = ArgumentList[0]->AsList();
    vector<KPValue>& ValueList = ListValue.ValueList();
    unsigned ListLength = ValueList.size();

    if (ListLength == 0) {
        throw KPException() << "mean(): empty list";
    }

    //...BUG: check not only the first element ...//
    if (ValueList[0].IsList()) {
	ReturnValue = KPValue(KPListValue());
	vector<KPValue>& ResultList = ReturnValue.AsValueList();
	for (unsigned i = 0; i < ListLength; i++) {
	    KPValue SubReturnValue;
	    vector<KPValue*> SubArgumentList;
	    SubArgumentList.push_back(&ValueList[i]);
	    this->Mean(SubArgumentList, SubReturnValue);
	    ResultList.push_back(SubReturnValue);
	}
    }
    else {
	double Value = 0;
	for (unsigned i = 0; i < ValueList.size(); i++) {
	    Value += ValueList[i].AsDouble();
	}
	Value /= ValueList.size();
	ReturnValue = KPValue(Value);
    }

    return 1;
}

int KPListMathObject::Deviation(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "deviation(): invalid argument[s]";
    }
    
    KPListValue& ListValue = ArgumentList[0]->AsList();
    vector<KPValue>& ValueList = ListValue.ValueList();
    unsigned ListLength = ValueList.size();

    if (ListLength == 0) {
        throw KPException() << "deviation(): empty list";
    }

    //...BUG: check not only the first element ...//
    if (ValueList[0].IsList()) {
	ReturnValue = KPValue(KPListValue());
	vector<KPValue>& ResultList = ReturnValue.AsValueList();
	for (unsigned i = 0; i < ListLength; i++) {
	    KPValue SubReturnValue;
	    vector<KPValue*> SubArgumentList;
	    SubArgumentList.push_back(&ValueList[i]);
	    this->Deviation(SubArgumentList, SubReturnValue);
	    ResultList.push_back(SubReturnValue);
	}
    }
    else {
	double Sum = 0, SumOfSquared = 0;
	for (unsigned i = 0; i < ValueList.size(); i++) {
	    Sum += ValueList[i].AsDouble();
	    SumOfSquared += sqr(ValueList[i].AsDouble());
	}
	double SquaredMean = sqr(Sum / ListLength);
	double MeanOfSquared = SumOfSquared / ListLength;
	if (MeanOfSquared > SquaredMean) {
	    double Value = sqrt(MeanOfSquared - SquaredMean);
	    ReturnValue = KPValue(Value);
	}
	else {
	    ReturnValue = KPValue((double) 0);
	}
    }

    return 1;
}

int KPListMathObject::Delta(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "delta(): invalid argument[s]";
    }
    
    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& InputList = ArgumentList[0]->AsValueList();
    vector<KPValue>& ResultList = ReturnValue.AsValueList();
    unsigned ListLength = InputList.size();

    if (ListLength == 0) {
        throw KPException() << "delta(): empty list";
    }

    bool IsLong = InputList[0].IsLong();
    for (unsigned i = 1; i < ListLength; i++) {
	if (IsLong && ! InputList[i].IsLong()) {
	    IsLong = false;
	}
	if (IsLong) {
	    ResultList.push_back(KPValue(
	        InputList[i].AsLong() - InputList[i-1].AsLong()
	    ));
	}
	else {
	    ResultList.push_back(KPValue(
	        InputList[i].AsDouble() - InputList[i-1].AsDouble()
	    ));
	}
    }

    return 1;
}

int KPListMathObject::Sigma(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "sigma(): invalid argument[s]";
    }
    
    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& InputList = ArgumentList[0]->AsValueList();
    vector<KPValue>& ResultList = ReturnValue.AsValueList();

    ResultList.push_back(KPValue((long) 0));

    bool IsLong = true;
    long LongSum = 0;
    double DoubleSum = 0;
    for (unsigned i = 0; i < InputList.size(); i++) {
	if (IsLong && ! InputList[i].IsLong()) {
	    DoubleSum = LongSum;
	    IsLong = false;
	}
	if (IsLong) {
	    LongSum += InputList[i].AsLong();
	    ResultList.push_back(KPValue(LongSum));
	}
	else {
	    DoubleSum += InputList[i].AsDouble();
	    ResultList.push_back(KPValue(DoubleSum));
	}
    }

    return 1;
}

int KPListMathObject::Zeros(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsLong())) {
        throw KPException() << "zeros(): invalid argument[s]";
    }
    
    int Size = ArgumentList[0]->AsLong();
    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& ResultList = ReturnValue.AsValueList();

    for (int i = 0; i < Size; i++) {
	ResultList.push_back(KPValue((long) 0));
    }

    return 1;
}

int KPListMathObject::Ones(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsLong())) {
        throw KPException() << "ones(): invalid argument[s]";
    }
    
    int Size = ArgumentList[0]->AsLong();
    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& ResultList = ReturnValue.AsValueList();

    for (int i = 0; i < Size; i++) {
	ResultList.push_back(KPValue((long) 1));
    }

    return 1;
}

int KPListMathObject::Find(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "find(): invalid argument[s]";
    }
    
    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& ResultList = ReturnValue.AsValueList();

    vector<KPValue>& InputList = ArgumentList[0]->AsValueList();
    for (unsigned i = 0; i < InputList.size(); i++) {
	if (InputList[i].AsBool()) {
	    ResultList.push_back(KPValue((long) i));
	}
    }

    return 1;
}

int KPListMathObject::Count(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsList())) {
        throw KPException() << "count(): invalid argument[s]";
    }
    
    long Result = 0;

    vector<KPValue>& InputList = ArgumentList[0]->AsValueList();
    for (unsigned i = 0; i < InputList.size(); i++) {
	if (InputList[i].AsBool()) {
	    Result++;
	}
    }

    ReturnValue = KPValue(Result);

    return 1;
}

int KPListMathObject::Divide(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (
	(ArgumentList.size() != 2) || 
	(! ArgumentList[0]->IsList()) || (! ArgumentList[1]->IsLong())
    ){
        throw KPException() << "divide(list src, int length): invalid argument[s]";
    }
    
    vector<KPValue>& InputList = ArgumentList[0]->AsValueList();

    int SectionLength = ArgumentList[1]->AsLong();
    if (SectionLength < 1) {
        throw KPException() << "divide(list src, int length): invalid length";
    }

    int NumberOfSections = InputList.size() / SectionLength;
    if ((unsigned) (SectionLength * NumberOfSections) < InputList.size()) {
	NumberOfSections += 1;
    }

    ReturnValue = KPValue(KPListValue());
    vector<KPValue>& ResultList = ReturnValue.AsValueList();

    unsigned Index = 0;
    for (int i = 0; i < NumberOfSections; i++) {
	KPValue SectionListValue = KPValue(KPListValue());
	vector<KPValue>& SectionList = SectionListValue.AsValueList();
	for (int j = 0; j < SectionLength; j++) {
	    if (Index >= InputList.size()) {
		break;
	    }
	    SectionList.push_back(InputList[Index++]);
	}
	ResultList.push_back(SectionListValue);
    }

    return 1;
}
