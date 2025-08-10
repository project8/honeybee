// demo-evaluator-userfunction.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <kebap/Kebap.h>

using namespace std;
using namespace kebap;



class KMyFunctionObject: public KPObjectPrototype {
  public:
    KMyFunctionObject(void);
    virtual ~KMyFunctionObject();
    virtual KPObjectPrototype* Clone(void);
    virtual void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList);
    virtual int MethodIdOf(const std::string& MethodName);
    virtual int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue);
  protected:
    virtual int Factorial(vector<KPValue*>& ArgumentList, KPValue& ReturnValue);
    virtual int Fibonacci(vector<KPValue*>& ArgumentList, KPValue& ReturnValue);
  protected:
    enum {
	MethodId_Factorial = KPObjectPrototype::fNumberOfMethods,
	MethodId_Fibonacci,
	fNumberOfMethods
    };
};



KMyFunctionObject::KMyFunctionObject(void)
: KPObjectPrototype("MyFunction")
{
}

KMyFunctionObject::~KMyFunctionObject()
{
}

KPObjectPrototype* KMyFunctionObject::Clone(void)
{
    return new KMyFunctionObject();
}

void KMyFunctionObject::Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList)
{
}

int KMyFunctionObject::MethodIdOf(const std::string& MethodName)
{
    if (MethodName == "factorial") {
        return MethodId_Factorial;
    }
    else if (MethodName == "fibonacci") {
        return MethodId_Fibonacci;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KMyFunctionObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue)
{
    int Result = 0;
    
    switch (MethodId) {
      case MethodId_Factorial:
        Result = Factorial(ArgumentList, ReturnValue);
	break;
      case MethodId_Fibonacci:
        Result = Fibonacci(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KMyFunctionObject::Factorial(vector<KPValue*>& ArgumentList, KPValue& ReturnValue)
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "factorial(): invalid number of argument[s]";
    }
    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "factorial(): invalid argument";
    }
    long n = ArgumentList[0]->AsLong();

    long Result = 1;
    for (int i = 2; i <= n; i++) {
        Result *= i;
    }
    
    ReturnValue = KPValue(Result);
    return 1;
}

int KMyFunctionObject::Fibonacci(vector<KPValue*>& ArgumentList, KPValue& ReturnValue)
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "factorial(): invalid number of argument[s]";
    }
    if (! ArgumentList[0]->IsNumeric()) {
        throw KPException() << "factorial(): invalid argument";
    }
    long n = ArgumentList[0]->AsLong();

    long x0 = 0, x1 = 1, Result;
    if (n == 0) {
        Result = 0;
    }
    else if (n == 1) {
        Result = 1;
    }
    else {
        Result = 1;
        for (int i = 2; i <= n; i++) {
            Result = x0 + x1;
            x0 = x1;
            x1 = Result;
        }
    }
    
    ReturnValue = KPValue(Result);
    return 1;
}




class KPMyEvaluator: public KPEvaluator {
  public:
    KPMyEvaluator(const std::string& expression);
};


KPMyEvaluator::KPMyEvaluator(const std::string& expression)
: KPEvaluator(expression)
{
    fBuiltinFunctionTable->RegisterStaticObject(new KMyFunctionObject);
}



int main(int argc, char** argv)
{
    KPMyEvaluator f("factorial(x) / (1.0+fibonacci(x))");

    try {
        for (int x = 0; x < 10; x++) {
            cout << x << ":\t" << f(x) << endl;
        }
    }
    catch (KPException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    return 0;
}
