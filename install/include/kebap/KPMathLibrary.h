// KPMathLibrary.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPMathLibrary_h__
#define __KPMathLibrary_h__

#include <string>
#include <vector>
#include "KPObject.h"


namespace kebap {

class KPMathObject: public KPObjectPrototype {
  public:
    KPMathObject();
    ~KPMathObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_Sin = KPObjectPrototype::fNumberOfMethods,
	MethodId_Cos,
	MethodId_Tan,
	MethodId_Asin,
	MethodId_Acos,
	MethodId_Atan,
	MethodId_Atan2,
	MethodId_Exp,
	MethodId_Log,
	MethodId_Log10,
	MethodId_Sqrt,
	MethodId_Abs,
	MethodId_Arg,
	MethodId_Real,
	MethodId_Imag,
	MethodId_Trunc,
	MethodId_Round,
	MethodId_Ceil,
	MethodId_Floor,
	MethodId_Srand,
	MethodId_Rand,
	MethodId_Sinc,
	fNumberOfMethods
    };
  protected:
    int Sin(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Cos(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Tan(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Asin(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Acos(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Atan(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Atan2(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Exp(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Log(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Log10(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Sqrt(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Abs(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Arg(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Real(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Imag(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Round(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Trunc(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Ceil(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Floor(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Srand(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Rand(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Sinc(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
};



class KPListMathObject: public KPObjectPrototype {
  public:
    KPListMathObject();
    ~KPListMathObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_Length = KPObjectPrototype::fNumberOfMethods,
	MethodId_Min,
	MethodId_Max,
	MethodId_Sum,
	MethodId_Mean,
	MethodId_Deviation,
	MethodId_Delta,
	MethodId_Sigma,
	MethodId_Zeros,
	MethodId_Ones,
	MethodId_Find,
	MethodId_Count,
	MethodId_Divide,
	fNumberOfMethods
    };
  protected:
    int Length(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Min(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Max(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Sum(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Mean(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Deviation(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Delta(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Sigma(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Zeros(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Ones(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Find(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Count(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Divide(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
};


}
#endif
