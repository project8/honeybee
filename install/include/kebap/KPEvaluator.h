// KPEvaluator.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPEvaluator_h__
#define __KPEvaluator_h__

#include <string>
#include "KPException.h"


namespace kebap {

class KPTokenTable;
class KPOperatorTable;
class KPExpressionParser;
class KPObjectPrototypeTable;
class KPBuiltinFunctionTable;
class KPSymbolTable;
class KPExpression;
class KPValue;


class KPEvaluator {
  public:
    class TParameterAccessor {
      public:
        TParameterAccessor(KPValue* Variable): fVariable(Variable) {}
        double operator=(double Value) {
            return fVariable->AssignDouble(Value), Value;
        }
        operator double() const {
            return fVariable->AsDouble();
        }
      protected:
        KPValue* fVariable;
    };
  public:
    KPEvaluator(const std::string& Expression);
    virtual ~KPEvaluator();
    virtual double Evaluate(double X) ;
    virtual void SetParameter(const std::string& Name, double Value);
    virtual KPValue* GetVariable(const std::string& Name);
    inline double operator()(double X)  { 
        return Evaluate(X); 
    }
    inline TParameterAccessor operator[](const std::string& Name) {
        return TParameterAccessor(GetVariable(Name));
    }
  protected:
    KPTokenTable *fTokenTable;
    KPOperatorTable *fOperatorTable;
    KPExpressionParser *fExpressionParser;
    KPObjectPrototypeTable* fObjectPrototypeTable;
    KPBuiltinFunctionTable* fBuiltinFunctionTable;
    KPSymbolTable* fSymbolTable;
  private:
    std::string fExpressionString;
    KPExpression* fExpression;
    KPValue* fVariableX;
};


}
#endif
