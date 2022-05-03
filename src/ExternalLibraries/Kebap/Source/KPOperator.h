// KPOperator.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPOperator_h__
#define __KPOperator_h__

#include <string>
#include <vector>
#include <map>
#include "KPObject.h"
#include "KPValue.h"
#include "KPTokenizer.h"
#include "KPSymbolTable.h"


namespace kebap {

class KPExpressionParser;
class KPExpression;
class KPOperator;

class KPOperatorPriority {
  public:
    explicit KPOperatorPriority(int PriorityRank = -1);
    KPOperatorPriority(const std::string& BaseOperatorSymbol, int PriorityOffset, int TemporaryPriorityRank = -1);
    KPOperatorPriority(const KPOperatorPriority& Priority);
    virtual ~KPOperatorPriority();
    virtual KPOperatorPriority& operator= (const KPOperatorPriority& Priority);
    virtual void SetPriorityRank(int PriorityRank);
    virtual int PriorityRank() const;
    virtual const std::string& BaseOperatorSymbol() const;
    virtual int PriorityOffset() const;
  protected:
    int fPriorityRank;
    std::string fBaseOperatorSymbol;
    int fPriorityOffset;
};


class KPOperatorTable {
  public:
    KPOperatorTable();
    virtual ~KPOperatorTable();
    virtual void Merge(KPOperatorTable* Source);
    virtual void AddOperator(KPOperator* Operator, int PriorityRank);
    virtual void AddOperator(KPOperator* Operator, const KPOperatorPriority& Priority);
    virtual void AddPrepositionalOperator(KPOperator* Operator);
    virtual void AddPostpositionalOperator(KPOperator* Operator);
    virtual void AddElementaryOperator(KPOperator* Operator);
    virtual KPOperator* CreateOperator(const std::string& Symbol);
    virtual KPOperator* CreatePrepositionalOperator(const std::string& Symbol);
    virtual KPOperator* CreatePostpositionalOperator(const std::string& Symbol);
    virtual KPOperator* CreateElementaryOperator(const std::string& Symbol);
    virtual const KPOperatorPriority& PriorityOf(const std::string& Symbol);
    virtual int PriorityRankOf(const std::string& Symbol);
    virtual int HighestPriorityRank() const;
    virtual int LowestPriorityRank() const;
  protected:
    std::map<std::string, KPOperator*> fOperatorTable;
    std::map<std::string, KPOperator*> fPrepositionalOperatorTable;
    std::map<std::string, KPOperator*> fPostpositionalOperatorTable;
    std::map<std::string, KPOperator*> fElementaryOperatorTable;
    std::map<std::string, KPOperatorPriority> fPriorityTable;
    std::map<std::string, int> fPriorityRankTable;
    int fHighestPriorityRank;
    int fLowestPriorityRank;
};


class KPCxxOperatorTable: public KPOperatorTable {
  public:
    KPCxxOperatorTable();
    ~KPCxxOperatorTable() override;
};


class KPOperator {
  public:
    KPOperator();
    virtual ~KPOperator();
    virtual KPOperator* Clone() const = 0;
    virtual std::string Symbol() const = 0;
    virtual std::string Name() const = 0;
    virtual void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) ;
    virtual KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  = 0;
    virtual bool IsLeftAssociative() const;
    virtual KPExpression* InternalExpression(int Index = 0);
  protected:
    virtual KPValue& EvaluateList(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) ;
};


class KPOperatorNew: public KPOperator {
  public:
    KPOperatorNew();
    ~KPOperatorNew() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
  protected:
    std::string fTypeName;
    KPExpression* fLengthExpression;
    std::vector<KPExpression*> fArgumentList;
};


class KPOperatorDelete: public KPOperator {
  public:
    KPOperatorDelete();
    ~KPOperatorDelete() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
  protected:
    bool fIsForArray;
};


class KPOperatorVariableAccess: public KPOperator {
  public:
    KPOperatorVariableAccess();
    ~KPOperatorVariableAccess() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    static KPSymbolTable& LocalSymbolTable();
  protected:
    static KPSymbolTable fLocalSymbolTable;
    KPExpression* fVariableNameExpression;
    std::string fVariableName;
    long fVariableId;
};


class KPOperatorListGenerate: public KPOperator {
  public:
    KPOperatorListGenerate();
    ~KPOperatorListGenerate() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
  protected:
    KPExpression* fStartValueExpression;
    KPExpression* fEndValueExpression;
    KPExpression* fStepValueExpression;
    std::string fSeparator;
};


class KPOperatorSizeOf: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorTypeOf: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorKeys: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorPointerReference: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorAddress: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorIncrement: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorDecrement: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorPostpositionalIncrement: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorPostpositionalDecrement: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorSignPlus: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorSignMinus: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorNot: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorBitReverse: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorMultiple: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorDivide: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorModulo: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorAdd: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorSubtract: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorConcatenate: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorLeftShift: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorRightShift: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorGreaterThan: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorLessThan: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorGreaterEqual: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorLessEqual: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorEqual: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorNotEqual: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorAnd: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorOr: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorBitAnd: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorBitXor: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorBitOr: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorListAnd: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorAssign: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignSum: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignDifference: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignProduct: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignQuotient: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignRemainder: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignBitAnd: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignBitOr: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignBitXor: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignRightShift: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignLeftShift: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorAssignConcatenation: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    bool IsLeftAssociative() const override;
};


class KPOperatorFactorial: public KPOperator {
  public:
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
};


class KPOperatorPower: public KPOperator {
  public:
    KPOperatorPower();
    ~KPOperatorPower() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
  protected:
    KPExpression* fPowerExpression;
};


class KPOperatorFunctionCall: public KPOperator {
  public:
    KPOperatorFunctionCall();
    ~KPOperatorFunctionCall() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    virtual void SetArgumentExpressionList(std::vector<KPExpression*>* ArgumentExpressionList);
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    KPExpression* InternalExpression(int Index = 0) override;
  protected:
    std::vector<KPExpression*>* fArgumentExpressionList;
    std::vector<KPExpression*> fMyArgumentExpressionList;
};


class KPOperatorArrayReference: public KPOperator {
  public:
    KPOperatorArrayReference();
    ~KPOperatorArrayReference() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    KPExpression* InternalExpression(int Index = 0) override;
  protected:
    virtual KPValue& ListItemOf(KPValue& ListValue, int Index, KPValue& Result) ;
    virtual KPValue& PartialListOf(KPValue& ListValue, KPValue& IndexListValue, KPValue& Result) ;
    virtual KPValue& StringItemOf(KPValue& StringValue, int Index, KPValue& Result) ;
  protected:
    KPExpression* fIndexExpression;
};


class KPOperatorTableReference: public KPOperator {
  public:
    KPOperatorTableReference();
    ~KPOperatorTableReference() override;
    KPOperator* Clone() const override;
    std::string Symbol() const override;
    std::string Name() const override;
    void Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) override ;
    KPValue& Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) override ;
    KPExpression* InternalExpression(int Index = 0) override;
  protected:
    virtual KPValue& PartialListOf(KPValue& ListValue, KPValue& IndexListValue, KPValue& Result) ;
  protected:
    KPExpression* fIndexExpression;
};


}
#endif
