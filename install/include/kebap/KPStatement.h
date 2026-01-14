// KPStatement.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPStatement_h__
#define __KPStatement_h__

#include <string>
#include <vector>
#include <set>
#include <map>
#include "KPTokenizer.h"
#include "KPExpression.h"
#include "KPSymbolTable.h"


namespace kebap {

class KPStatement;
class KPStatementParser;


class KPStatementTable {
  public:
    KPStatementTable();
    virtual ~KPStatementTable();
    virtual void Merge(KPStatementTable* Source);
    virtual void AddStatement(KPStatement* Statement);
    virtual KPStatement* CreateStatement(const std::string& FirstToken);
  protected:
    std::map<std::string, KPStatement*> fStatementTable;
};


class KPCxxStatementTable: public KPStatementTable {
  public:    
    KPCxxStatementTable();
    ~KPCxxStatementTable() override;
};


class KPStatementParser {
  public:
    KPStatementParser(KPStatementTable* StatementTable, KPExpressionParser* ExpressionParser);
    virtual ~KPStatementParser();
    virtual KPStatement* Parse(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) ;
    KPExpressionParser* ExpressionParser() const;
  protected:
    KPStatementTable* fStatementTable;
    KPExpressionParser* fExpressionParser;
};


class KPStatement {
  public:
    enum TExecStatus {
	esNormal, esBreak, esContinue, esReturn, 
	esExit, esError
    };
    struct TExecResult {
        TExecResult();
        explicit TExecResult(KPValue& Value);
	TExecStatus ExecStatus;
	KPValue ReturnValue;
    };
  public:
    KPStatement();
    virtual ~KPStatement();
    virtual KPStatement* Clone() = 0;
    virtual std::string FirstToken() const = 0;
    virtual void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable)  = 0;
    virtual TExecResult Execute(KPSymbolTable* SymbolTable)  = 0;
};


class KPVariableDeclaration {
  public:
    KPVariableDeclaration(const std::string& TypeName = "");
    virtual ~KPVariableDeclaration();
    virtual void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) ;
    virtual void Execute(KPSymbolTable* SymbolTable) ;
    virtual long VariableId() const;
 protected:
    long fVariableId;
    std::string fTypeName;
    std::string fVariableName;
    KPValue* fInitialValue;
    KPExpression* fArrayLengthExpression;
    KPExpression* fInitializeExpression;
    std::vector<KPExpression*> fConstructorArgumentList;
    bool fIsArray;
};


class KPVariableDeclarationStatement: public KPStatement {
  public:
    KPVariableDeclarationStatement();
    ~KPVariableDeclarationStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    std::string fTypeName;
    std::string fPosition;
    std::vector<KPVariableDeclaration*> fVariableDeclarationList;
};


class KPExpressionStatement: public KPStatement {
  public:
    KPExpressionStatement();
    ~KPExpressionStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPExpression* fExpression;
};


class KPComplexStatement: public KPStatement {
  public:
    KPComplexStatement();
    ~KPComplexStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    std::vector<KPStatement*> fStatementList;
};


class KPEmptyStatement: public KPStatement {
  public:
    KPEmptyStatement();
    ~KPEmptyStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
};

class KPIfStatement: public KPStatement {
  public:
    KPIfStatement();
    ~KPIfStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPExpression* fConditionExpression;
    KPStatement* fTrueStatement;
    KPStatement* fFalseStatement;
};


class KPWhileStatement: public KPStatement {
  public:
    KPWhileStatement();
    ~KPWhileStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPExpression* fConditionExpression;
    KPStatement* fStatement;
};


class KPForStatement: public KPStatement {
  public:
    KPForStatement();
    ~KPForStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPStatement* fInitializeStatement;
    KPExpression* fConditionExpression;
    KPExpression* fIncrementExpression;
    KPStatement* fStatement;
};


class KPForeachStatement: public KPStatement {
  public:
    KPForeachStatement();
    ~KPForeachStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPExpression* fVariableExpression;
    KPExpression* fKeyExpression;
    KPExpression* fIndexExpression;
    KPExpression* fListExpression;
    KPStatement* fStatement;
};


class KPBreakStatement: public KPStatement {
  public:
    KPBreakStatement();
    ~KPBreakStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
};


class KPContinueStatement: public KPStatement {
  public:
    KPContinueStatement();
    ~KPContinueStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
};


class KPReturnStatement: public KPStatement {
  public:
    KPReturnStatement();
    ~KPReturnStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPExpression* fExpression;
};

class KPThrowStatement: public KPStatement {
  public:
    KPThrowStatement();
    ~KPThrowStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPExpression* fExceptionExpression;
};

class KPTryCatchStatement: public KPStatement {
  public:
    KPTryCatchStatement();
    ~KPTryCatchStatement() override;
    KPStatement* Clone() override;
    std::string FirstToken() const override;
    void Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) override ;
    TExecResult Execute(KPSymbolTable* SymbolTable) override ;
  protected:
    KPStatement* fTryStatement;
    KPVariableDeclaration* fArgumentDeclaration;
    KPStatement* fCatchStatement;
};


}
#endif
