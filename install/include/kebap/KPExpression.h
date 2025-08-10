// KPExpression.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPExpression_h__
#define __KPExpression_h__

#include <iostream>
#include <string>
#include <vector>
#include "KPTokenizer.h"
#include "KPObject.h"
#include "KPValue.h"
#include "KPOperator.h"
#include "KPBuiltinFunction.h"
#include "KPSymbolTable.h"


namespace kebap {

class KPExpression;
class KPFunctionCallNode;


class KPExpressionParser {
  public:
    KPExpressionParser(KPOperatorTable* OperatorTable);
    virtual ~KPExpressionParser();
    virtual KPExpression* Parse(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable, int PriorityRank = -1) ;
  public:
    virtual std::vector<KPExpression*> ParseExpressionList(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable, const std::string& StartToken, const std::string& StopToken, const std::string& SeparatorToken) ;
  public:
    virtual KPExpression* ParsePrepositional(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPExpression* ParseElement(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPExpression* ParsePostpositional(KPExpression* Expression, KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPExpression* ParseTypeCast(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPExpression* ParseTemporaryObjectCreation(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPFunctionCallNode* ParseFunctionCall(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPExpression* ParseListExpression(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    virtual KPExpression* ParseMethodInvocation(KPExpression* ObjectNode, KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
  protected:  
    KPOperatorTable* fOperatorTable;
};


class KPExpression {
  public:
    KPExpression();
    virtual ~KPExpression();
    virtual KPValue& Evaluate(KPSymbolTable* SymbolTable)  = 0;
    virtual void Dump(std::ostream &os, int IndentLevel = 0) const;
    virtual void SetLineNumber(long LineNumber);
    virtual std::string Position() const;
  protected:
    virtual void DumpThis(std::ostream &os) const = 0;
  protected:
    KPExpression* fLeftNode;
    KPExpression* fRightNode;
    long fLineNumber;
};


class KPOperatorNode: public KPExpression {
  public:
    KPOperatorNode(KPOperator* Operator, KPExpression* LeftNode, KPExpression* RightNode);
    ~KPOperatorNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    void DumpThis(std::ostream &os) const override;
  protected:
    KPOperator* fOperator;
    KPValue fValue;
};


class KPTypeCastNode: public KPExpression {
  public:
    KPTypeCastNode(const std::string& TypeName, KPExpression* RightNode);
    ~KPTypeCastNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    virtual void Convert(const KPValue& From, KPValue& To) ;
    void DumpThis(std::ostream &os) const override;
  protected:
    std::string fTypeName;
    KPValue fValue;
};


class KPLiteralNode: public KPExpression {
  public:
    KPLiteralNode(const KPValue& Value);
    ~KPLiteralNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    void DumpThis(std::ostream &os) const override;
  protected:
    KPValue fValue;
};


class KPVariableNode: public KPExpression {
  public:
    KPVariableNode(long VariableId);
    ~KPVariableNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    void DumpThis(std::ostream &os) const override;
  protected:
    long fVariableId;
};


class KPListNode: public KPExpression {
  public:
    KPListNode(const std::vector<KPExpression*>& KeyExpressionList, const std::vector<KPExpression*>& ValueExpressionList);
    ~KPListNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    void DumpThis(std::ostream &os) const override;
  protected:
    std::vector<KPExpression*> fKeyExpressionList;
    std::vector<KPExpression*> fValueExpressionList;
    KPValue fValue;
};


class KPFunctionCallNode: public KPExpression {
  public:
    KPFunctionCallNode(long FunctionId, std::vector<KPExpression*>& ArgumentExpressionList);
    ~KPFunctionCallNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  public:
    virtual void EvaluateArguments(KPSymbolTable* SymbolTable) ;
    virtual KPValue& EvaluateFunction(KPSymbolTable* SymbolTable) ;
  protected:
    virtual KPValue& EvaluateObjectFunction(KPValue* Variable, KPSymbolTable* SymbolTable) ;
    void DumpThis(std::ostream &os) const override;
  protected:
    KPValue fValue;
    long fFunctionId;
    KPBuiltinFunctionTable* fBuiltinFunctionTable;
    std::vector<KPExpression*> fArgumentExpressionList;
    std::vector<KPValue*> fArgumentList;
};


class KPMethodInvocationNode: public KPFunctionCallNode {
  public:
    KPMethodInvocationNode(KPExpression* ObjectExpression, long FunctionId, std::vector<KPExpression*>& ArgumentExpressionList);
    ~KPMethodInvocationNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    int fMethodId;
    std::string fMethodName;
    KPExpression* fObjectExpression;
    std::vector<KPExpression*> fArgumentExpressionList;
  private:
    KPObjectPrototype* fObject;
};


class KPPropertyAccessNode: public KPExpression {
  public:
    KPPropertyAccessNode(KPExpression* ObjectExpression, const std::string& PropertyName);
    ~KPPropertyAccessNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    void DumpThis(std::ostream &os) const override;
  protected:
    int fPropertyId;
    std::string fPropertyName;
    KPExpression* fObjectExpression;
    KPValue fValue;
  private:
    KPObjectPrototype* fObject;
};


class KPTemporaryObjectCreationNode: public KPFunctionCallNode {
  public:
    KPTemporaryObjectCreationNode(const std::string& TypeName, std::vector<KPExpression*>& ArgumentExpressionList);
    ~KPTemporaryObjectCreationNode() override;
    KPValue& Evaluate(KPSymbolTable* SymbolTable) override ;
  protected:
    std::string fTypeName;
    std::vector<KPExpression*> fArgumentExpressionList;
};


}
#endif
