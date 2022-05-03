// KPEvaluator.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include "KPTokenizer.h"
#include "KPOperator.h"
#include "KPExpression.h"
#include "KPStatement.h"
#include "KPMathLibrary.h"
#include "KPEvaluator.h"

using namespace std;
using namespace kebap;


KPEvaluator::KPEvaluator(const std::string& Expression)
{
    fExpressionString = Expression;

    fTokenTable = new KPCxxTokenTable();
    fOperatorTable = new KPCxxOperatorTable();
    fExpressionParser = new KPExpressionParser(fOperatorTable);

    fObjectPrototypeTable = new KPObjectPrototypeTable();
    fBuiltinFunctionTable = new KPBuiltinFunctionTable();
    fBuiltinFunctionTable->RegisterStaticObject(new KPMathObject());
    fSymbolTable = (
        new KPSymbolTable(fObjectPrototypeTable, fBuiltinFunctionTable)
    );
    fSymbolTable->RegisterVariable("pi", KPValue(3.141592));
    fSymbolTable->RegisterVariable("e", KPValue(2.718281828));

    long variableId = fSymbolTable->RegisterVariable("x", KPValue(0.0));
    fVariableX = fSymbolTable->GetVariable(variableId);

    fExpression = nullptr;
}

KPEvaluator::~KPEvaluator()
{
    delete fExpression;

    delete fSymbolTable;
    delete fBuiltinFunctionTable;
    delete fObjectPrototypeTable;
    delete fExpressionParser;
    delete fOperatorTable;
    delete fTokenTable;
}

void KPEvaluator::SetParameter(const std::string& Name, double Value)
{
    GetVariable(Name)->AssignDouble(Value);
}

KPValue* KPEvaluator::GetVariable(const std::string& Name)
{
    long VariableId = fSymbolTable->NameToId(Name);
    KPValue* Variable = fSymbolTable->GetVariable(VariableId);

    if (! Variable) {
        Variable = new KPValue(0.0);
        fSymbolTable->RegisterVariable(VariableId, Variable);
    }
    
    return Variable;
}

double KPEvaluator::Evaluate(double X) 
{
    if (! fExpression) {
        istringstream is(fExpressionString);
        KPTokenizer tokenizer(is, fTokenTable);
        try {
            fExpression = fExpressionParser->Parse(&tokenizer, fSymbolTable);
        }
        catch (KPException &e) {
            fExpression = new KPLiteralNode(KPValue(0.0));
            throw e;
        }
    }

    fVariableX->AssignDouble(X);

    return fExpression->Evaluate(fSymbolTable).AsDouble();
}
