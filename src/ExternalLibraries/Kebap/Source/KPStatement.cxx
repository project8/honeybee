// KPStatement.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <sstream>
#include <set>
#include <map>
#include "KPTokenizer.h"
#include "KPObject.h"
#include "KPValue.h"
#include "KPOperator.h"
#include "KPExpression.h"
#include "KPSymbolTable.h"
#include "KPStatement.h"

using namespace std;
using namespace kebap;


KPStatementTable::KPStatementTable()
{
}

KPStatementTable::~KPStatementTable()
{
    map<string, KPStatement*>::iterator StatementIterator;
    for (
	StatementIterator = fStatementTable.begin();
	StatementIterator != fStatementTable.end();
	StatementIterator++
    ){
	delete (*StatementIterator).second;
    }
}

void KPStatementTable::Merge(KPStatementTable* Source)
{
    map<string, KPStatement*>::iterator StatementIterator;
    for (
	StatementIterator = Source->fStatementTable.begin();
	StatementIterator != Source->fStatementTable.end();
	StatementIterator++
    ){
	KPStatement* Statement = (*StatementIterator).second;
	AddStatement(Statement->Clone());
    }
}

void KPStatementTable::AddStatement(KPStatement* Statement)
{
    string FirstToken = Statement->FirstToken();

    // if the same statement has already registered,
    // it will be overwritten.
    if (fStatementTable.count(FirstToken) > 0) {
	delete fStatementTable[FirstToken];
    }

    fStatementTable[FirstToken] = Statement;
}

KPStatement* KPStatementTable::CreateStatement(const string& FirstToken)
{
    if (fStatementTable.count(FirstToken) == 0) {
        return nullptr;
    }
    else {
        return fStatementTable[FirstToken]->Clone();
    }
}



KPCxxStatementTable::KPCxxStatementTable()
{
    AddStatement(new KPComplexStatement());
    AddStatement(new KPEmptyStatement());
    AddStatement(new KPIfStatement());
    AddStatement(new KPWhileStatement());
    AddStatement(new KPForStatement());
    AddStatement(new KPForeachStatement());
    AddStatement(new KPBreakStatement());
    AddStatement(new KPContinueStatement());
    AddStatement(new KPReturnStatement());
    AddStatement(new KPThrowStatement());
    AddStatement(new KPTryCatchStatement());
}

KPCxxStatementTable::~KPCxxStatementTable()
{
}



KPStatementParser::KPStatementParser(KPStatementTable* StatementTable, KPExpressionParser* ExpressionParser)
{
    fStatementTable = StatementTable;
    fExpressionParser = ExpressionParser;
}

KPStatementParser::~KPStatementParser()
{
}

KPStatement* KPStatementParser::Parse(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    KPStatement* Statement = nullptr;

    KPToken Token = Tokenizer->LookAhead();
    Statement = fStatementTable->CreateStatement(Token.AsString());

    if (Statement == nullptr) {
        if (
	    (SymbolTable->IsTypeName(Token.AsString())) &&
	    (Tokenizer->LookAhead(2).IsNot("("))
	){
            Statement = new KPVariableDeclarationStatement();
        }
        else {
            Statement = new KPExpressionStatement();
        }
    }

    try {
        Statement->Parse(Tokenizer, this, SymbolTable);
    }
    catch (KPException& e) {
        delete Statement;
        throw e;
    }

    return Statement;
}

KPExpressionParser* KPStatementParser::ExpressionParser() const
{
    return fExpressionParser;
}



KPStatement::TExecResult::TExecResult()
: ExecStatus(esNormal)
{
}

KPStatement::TExecResult::TExecResult(KPValue& Value)
: ExecStatus(esNormal), ReturnValue(Value)
{
}

KPStatement::KPStatement()
{
}

KPStatement::~KPStatement()
{
}



KPVariableDeclaration::KPVariableDeclaration(const string& TypeName)
{
    fTypeName = TypeName;

    fInitialValue = nullptr;
    fInitializeExpression = nullptr;
    fArrayLengthExpression = nullptr;
    fIsArray = false;
}

KPVariableDeclaration::~KPVariableDeclaration()
{
    if (fConstructorArgumentList.size() > 0) {
        for (unsigned i = 0; i < fConstructorArgumentList.size(); i++) {
            delete fConstructorArgumentList[i];
	}
    }

    delete fArrayLengthExpression;
    delete fInitializeExpression;
    delete fInitialValue;
}

void KPVariableDeclaration::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPToken Token;
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    if (fTypeName.empty()) {
	fTypeName = (Token = Tokenizer->Next()).AsString();
	if (! SymbolTable->IsTypeName(fTypeName)) {
	    Token.ThrowUnexpected("type name");
	}
    }

    while ((Token = Tokenizer->Next()).Is("*")) {
	fTypeName = "pointer";
    }

    if (! Token.IsIdentifier()) {
	Token.ThrowUnexpected("variable name");
    }
    fVariableName = Token.AsString();
    fVariableId = SymbolTable->NameToId(fVariableName);
    
    // array declaration //
    if (Tokenizer->LookAhead().Is("[")) {
	Tokenizer->Next();
	if (Tokenizer->LookAhead().IsNot("]")) {
	    fArrayLengthExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
	}
	fIsArray = true;
	Tokenizer->Next().MustBe("]");
    }

    // initialization-arguments //
    if (Tokenizer->LookAhead().Is("(")) {
	fConstructorArgumentList = ExpressionParser->ParseExpressionList(
	    Tokenizer, SymbolTable, "(", ")", ","
	);
    }

    // initialization-expression //
    if (Tokenizer->LookAhead().Is("=")) {
	Tokenizer->Next();
        fInitializeExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    }
}

void KPVariableDeclaration::Execute(KPSymbolTable* SymbolTable) 
{
    // initialization-argument evaluation //
    vector<KPValue*> ArgumentList;
    if (fConstructorArgumentList.size() > 0) {
 	for (unsigned i = 0; i < fConstructorArgumentList.size(); i++) {
	    ArgumentList.push_back(
		&fConstructorArgumentList[i]->Evaluate(SymbolTable)
	    );
	}
    }

    // initialization-expression evaluation //
    KPValue InitializeValue;
    long InitializeValueListLength = -1;
    if (fInitializeExpression != nullptr) {
	InitializeValue = fInitializeExpression->Evaluate(SymbolTable);
	if (InitializeValue.IsList()) {
	    InitializeValueListLength = InitializeValue.AsValueList().size();
	}
    }

    // array length evaluation and consistency check //
    long ArrayLength = -1; // positive value for array
    if (fArrayLengthExpression != nullptr) {
	ArrayLength = fArrayLengthExpression->Evaluate(SymbolTable).AsLong();
	if (
	    (InitializeValueListLength >= 0) && 
	    (ArrayLength != InitializeValueListLength)
	){
	    throw KPException() << "inconsistent initialization list size";
	}
    }
    else if (fIsArray) {
	if (InitializeValueListLength < 0) {
	    throw KPException() << "array size is expected";
	}
	ArrayLength = InitializeValueListLength;
    }

    // variable creation //
    KPValue* Variable = SymbolTable->CreateObject(fTypeName, ArrayLength);
    if (fIsArray) {
	for (int i = 0; i < ArrayLength; i++) {
	    Variable[i].SetLeftValueFlag();

	    ostringstream os;
	    os << fVariableName << "[" << i << "]";
	    Variable[i].SetName(os.str());
	}
    }
    else {
	Variable->SetName(fVariableName);
    }

    try {
	// initialization-argument processing //
	for (int i = 0; i < (ArrayLength > 0 ? ArrayLength : 1); i++) {
	    if (Variable[i].IsObject()) {
		Variable[i].AsObject()->Construct(fTypeName, ArgumentList);
	    }
	    else {
		if (ArgumentList.size() > 1) {
		    throw KPException() << "too many initial-value arguments";
		}
		else if (! ArgumentList.empty()) {
		    Variable[i].Assign(*ArgumentList[0]);
		    Variable[i].Unrefer();
		}
	    }
	}
	
	// initialization-expression processing //
	for (int i = 0; i < (ArrayLength > 0 ? ArrayLength : 1); i++) {
	    if (fInitializeExpression != nullptr) {
		if (! fIsArray) {
		    Variable[i].Assign(InitializeValue);
		}
		else {
		    Variable[i].Assign(InitializeValue.AsValueList()[i]);
		}
		Variable[i].Unrefer();
	    }
	}
    }
    catch (KPException &e) {
	// note reference-count at this point is zero //
	Variable->Refer();
	Variable->Destroy();

	if (fIsArray) {
	    delete[] Variable;
	}
	else {
	    delete Variable;
	}
	throw;
    }

    // array pointer setup //
    if (fIsArray) {
	Variable = new KPValue(Variable);
	Variable->SetArrayPointerFlag();
	Variable->SetName(fVariableName);
    }

    // symbol registration: only if declaration is successful //
    SymbolTable->RegisterVariable(fVariableId, Variable);
}

long KPVariableDeclaration::VariableId() const
{
    return fVariableId;
}



KPVariableDeclarationStatement::KPVariableDeclarationStatement()
{
}

KPVariableDeclarationStatement::~KPVariableDeclarationStatement()
{
    for (unsigned i = 0; i < fVariableDeclarationList.size(); i++) {
	delete fVariableDeclarationList[i];
    }
}

KPStatement* KPVariableDeclarationStatement::Clone()
{
    return new KPVariableDeclarationStatement();
}

string KPVariableDeclarationStatement::FirstToken() const
{
    return string("");
}

void KPVariableDeclarationStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPToken Token = Tokenizer->Next();
    fPosition = Token.Position();
    fTypeName = Token.AsString();

    if (! SymbolTable->IsTypeName(fTypeName)) {
        Token.ThrowUnexpected("type name");
    }

    try {
	KPVariableDeclaration* VariableDeclaration;
	do {
	    VariableDeclaration = new KPVariableDeclaration(fTypeName);
	    VariableDeclaration->Parse(Tokenizer, StatementParser, SymbolTable);
	    fVariableDeclarationList.push_back(VariableDeclaration);
	} while ((Token = Tokenizer->Next()).Is(","));

	Token.MustBe(";");
    }
    catch (KPException &e) {
	for (unsigned i = 0; i < fVariableDeclarationList.size(); i++) {
	    delete fVariableDeclarationList[i];
	}
	fVariableDeclarationList.erase(
	    fVariableDeclarationList.begin(),
	    fVariableDeclarationList.end()
	);
	throw;
    }
}

KPStatement::TExecResult KPVariableDeclarationStatement::Execute(KPSymbolTable* SymbolTable) 
{
    try {
	for (unsigned i = 0; i < fVariableDeclarationList.size(); i++) {
	    fVariableDeclarationList[i]->Execute(SymbolTable);
	}
    }
    catch (KPException &e) {
	throw KPException() << fPosition << e.what();
    }

    return TExecResult();
}



KPExpressionStatement::KPExpressionStatement()
{
    fExpression = nullptr;
}

KPExpressionStatement::~KPExpressionStatement()
{
    delete fExpression;
}

KPStatement* KPExpressionStatement::Clone()
{
    return new KPExpressionStatement();
}

string KPExpressionStatement::FirstToken() const
{
    return string("");
}

void KPExpressionStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    fExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);

    KPToken Token = Tokenizer->Next();
    if (Token.IsNot(";")) {
	throw KPException() << fExpression->Position() << "syntax error near '" << Token.AsString() << "'";
    }
}

KPStatement::TExecResult KPExpressionStatement::Execute(KPSymbolTable* SymbolTable) 
{
    return TExecResult(fExpression->Evaluate(SymbolTable));
}



KPEmptyStatement::KPEmptyStatement()
{
}

KPEmptyStatement::~KPEmptyStatement()
{
}

KPStatement* KPEmptyStatement::Clone()
{
    return new KPEmptyStatement();
}

string KPEmptyStatement::FirstToken() const
{
    return string(";");
}

void KPEmptyStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe(";");
}

KPStatement::TExecResult KPEmptyStatement::Execute(KPSymbolTable* SymbolTable) 
{
    return TExecResult();
}



KPComplexStatement::KPComplexStatement()
{
}

KPComplexStatement::~KPComplexStatement()
{
    vector<KPStatement*>::iterator Statement;
    for (
	 Statement = fStatementList.begin();
	 Statement != fStatementList.end();
	 Statement++
    ){
       delete *Statement;
    } 
}

KPStatement* KPComplexStatement::Clone()
{
    return new KPComplexStatement();
}

string KPComplexStatement::FirstToken() const
{
    return string("{");
}

void KPComplexStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("{");
    
    KPToken Token;
    KPStatement* Statement = nullptr;
    while ((Token = Tokenizer->Next()).IsNot("}")) {
        if (Token.IsEmpty()) {
	    Token.ThrowUnexpected("}");
	}
	else {
	    Tokenizer->Unget(Token);
	}

        Statement = StatementParser->Parse(Tokenizer, SymbolTable);
	fStatementList.push_back(Statement);
    }
}

KPStatement::TExecResult KPComplexStatement::Execute(KPSymbolTable* SymbolTable) 
{
    SymbolTable->EnterBlock();

    TExecResult Result;
    try {
	vector<KPStatement*>::const_iterator Statement;
	for (
	    Statement = fStatementList.begin();
	    Statement != fStatementList.end();
	    Statement++
	){
	    Result = (*Statement)->Execute(SymbolTable);
	    if (Result.ExecStatus != KPStatement::esNormal) {
		break;
	    }
	}
    }
    catch (KPException &e) {
	SymbolTable->ExitBlock();
	throw;
    }

    if (Result.ExecStatus != KPStatement::esReturn) {
	Result.ReturnValue = KPValue();
    }
    if (Result.ReturnValue.IsObject()) {
	SymbolTable->ExitBlock();
	throw("object cannot be passed to return");
    }

    SymbolTable->ExitBlock();

    return Result;
}



KPIfStatement::KPIfStatement()
{
    fConditionExpression = nullptr;
    fTrueStatement = nullptr;
    fFalseStatement = nullptr;
}

KPIfStatement::~KPIfStatement()
{
    delete fConditionExpression;
    delete fTrueStatement;
    delete fFalseStatement;
}

KPStatement* KPIfStatement::Clone()
{
    return new KPIfStatement();
}

string KPIfStatement::FirstToken() const
{
    return string("if");
}

void KPIfStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    Tokenizer->Next().MustBe("if");

    Tokenizer->Next().MustBe("(");
    fConditionExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    Tokenizer->Next().MustBe(")");

    fTrueStatement = StatementParser->Parse(Tokenizer, SymbolTable);

    KPToken Token = Tokenizer->Next();
    if (Token.Is("else")) {
        fFalseStatement = StatementParser->Parse(Tokenizer, SymbolTable);
    }
    else {
        Tokenizer->Unget(Token);
    }
}

KPStatement::TExecResult KPIfStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;

    if (fConditionExpression->Evaluate(SymbolTable).AsBool()) {
        Result = fTrueStatement->Execute(SymbolTable);
    }
    else {
        if (fFalseStatement != nullptr) {
	    Result = fFalseStatement->Execute(SymbolTable);
	}
    }

    return Result;
}



KPWhileStatement::KPWhileStatement()
{
    fConditionExpression = nullptr;
    fStatement = nullptr;
}

KPWhileStatement::~KPWhileStatement()
{
    delete fConditionExpression;
    delete fStatement;
}

KPStatement* KPWhileStatement::Clone()
{
    return new KPWhileStatement();
}

string KPWhileStatement::FirstToken() const
{
    return string("while");
}

void KPWhileStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    Tokenizer->Next().MustBe("while");

    Tokenizer->Next().MustBe("(");
    fConditionExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    Tokenizer->Next().MustBe(")");

    fStatement = StatementParser->Parse(Tokenizer, SymbolTable);
}

KPStatement::TExecResult KPWhileStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;

    while (fConditionExpression->Evaluate(SymbolTable).AsBool()) {
        Result = fStatement->Execute(SymbolTable);

	if (Result.ExecStatus == KPStatement::esBreak) {
	    Result.ExecStatus = esNormal;
	    break;
	}
	if (Result.ExecStatus == KPStatement::esContinue) {
	    Result.ExecStatus = esNormal;
	    continue;
	}
	if (Result.ExecStatus == KPStatement::esReturn) {
	    break;
	}
    }

    return Result;
}



KPForStatement::KPForStatement()
{
    fInitializeStatement = nullptr;
    fConditionExpression = nullptr;
    fIncrementExpression = nullptr;
    fStatement = nullptr;
}

KPForStatement::~KPForStatement()
{
    delete fInitializeStatement;
    delete fConditionExpression;
    delete fIncrementExpression;
    delete fStatement;
}

KPStatement* KPForStatement::Clone()
{
    return new KPForStatement();
}

string KPForStatement::FirstToken() const
{
    return string("for");
}

void KPForStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    Tokenizer->Next().MustBe("for");

    Tokenizer->Next().MustBe("(");
    fInitializeStatement = StatementParser->Parse(Tokenizer, SymbolTable);
    fConditionExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    Tokenizer->Next().MustBe(";");
    fIncrementExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    Tokenizer->Next().MustBe(")");

    fStatement = StatementParser->Parse(Tokenizer, SymbolTable);
}

KPStatement::TExecResult KPForStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;

    SymbolTable->EnterBlock();
    
    try {
	fInitializeStatement->Execute(SymbolTable);
	while (fConditionExpression->Evaluate(SymbolTable).AsBool()) {
	    Result = fStatement->Execute(SymbolTable);
	    
	    if (Result.ExecStatus == KPStatement::esBreak) {
		Result.ExecStatus = esNormal;
		break;
	    }
	    if (Result.ExecStatus == KPStatement::esReturn) {
		break;
	    }
	    if (Result.ExecStatus == KPStatement::esContinue) {
		Result.ExecStatus = esNormal;
	    }
	    
	    fIncrementExpression->Evaluate(SymbolTable);
	}
    }
    catch (KPException &e) {
	SymbolTable->ExitBlock();
	throw;
    }

    if (Result.ExecStatus != KPStatement::esReturn) {
	Result.ReturnValue = KPValue();
    }
    if (Result.ReturnValue.IsObject()) {
	SymbolTable->ExitBlock();
	throw("object cannot be passed to return");
    }

    SymbolTable->ExitBlock();
    
    return Result;
}



KPForeachStatement::KPForeachStatement()
{
    fVariableExpression = nullptr;
    fKeyExpression = nullptr;
    fIndexExpression = nullptr;
    fListExpression = nullptr;
    fStatement = nullptr;
}

KPForeachStatement::~KPForeachStatement()
{
    delete fVariableExpression;
    delete fKeyExpression;
    delete fIndexExpression;
    delete fListExpression;
    delete fStatement;
}

KPStatement* KPForeachStatement::Clone()
{
    return new KPForeachStatement();
}

string KPForeachStatement::FirstToken() const
{
    return string("foreach");
}

void KPForeachStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    Tokenizer->Next().MustBe("foreach");

    Tokenizer->Next().MustBe("(");
    if (Tokenizer->LookAhead().IsNot(";")) {
	fVariableExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    }
    if (Tokenizer->LookAhead().IsNot(";")) {
	Tokenizer->Next().MustBe(",");
	fKeyExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    }
    if (Tokenizer->LookAhead().IsNot(";")) {
	Tokenizer->Next().MustBe(",");
	fIndexExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    }
    Tokenizer->Next().MustBe(";");
    fListExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
    Tokenizer->Next().MustBe(")");

    fStatement = StatementParser->Parse(Tokenizer, SymbolTable);
}

KPStatement::TExecResult KPForeachStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;

    SymbolTable->EnterBlock();
    
    try {
	KPValue& ListExpressionValue = fListExpression->Evaluate(SymbolTable);
	if (! ListExpressionValue.IsList()) {
	    throw KPException() << fListExpression->Position() << "foreach: list value is expected";
	}
	KPListValue& ListValue = ListExpressionValue.AsList();

	KPValue* Variable = nullptr;
	if (fVariableExpression) {
	    Variable = &(fVariableExpression->Evaluate(SymbolTable));
	    if (! Variable->IsLeftValue()) {
		throw KPException() << fVariableExpression->Position() << "foreach: L-value is expected";
	    }
	}
	
	KPValue* Key = nullptr;
	if (fKeyExpression && ListValue.HasKeyIndex()) {
	    Key = &(fKeyExpression->Evaluate(SymbolTable));
	    if (! Key->IsLeftValue()) {
		throw KPException() << fIndexExpression->Position() << "foreach: L-value is expected";
	    }
	}
	
	KPValue* Index = nullptr;
	if (fIndexExpression) {
	    Index = &(fIndexExpression->Evaluate(SymbolTable));
	    if (! Index->IsLeftValue()) {
		throw KPException() << fIndexExpression->Position() << "foreach: L-value is expected";
	    }
	}
	
	for (unsigned i = 0; i < ListValue.ListSize(); i++) {
	    try {
		if (Variable) {
		    Variable->Assign(ListValue.ValueOf(i));
		}
		if (Key) {
		    Key->Assign(KPValue(ListValue.KeyOf(i)));
		}
		if (Index) {
		    Index->Assign(KPValue((long) i));
		}
	    }
	    catch (KPException &e) {
		throw KPException() << fListExpression->Position() + e.what();
	    }
	    
	    Result = fStatement->Execute(SymbolTable);
	    
	    if (Result.ExecStatus == KPStatement::esBreak) {
		Result.ExecStatus = esNormal;
		break;
	    }
	    if (Result.ExecStatus == KPStatement::esReturn) {
		break;
	    }
	    if (Result.ExecStatus == KPStatement::esContinue) {
		Result.ExecStatus = esNormal;
	    }
	}
    }
    catch (KPException &e) {
	SymbolTable->ExitBlock();
	throw;
    }

    if (Result.ExecStatus != KPStatement::esReturn) {
	Result.ReturnValue = KPValue();
    }
    if (Result.ReturnValue.IsObject()) {
	SymbolTable->ExitBlock();
	throw("object cannot be passed to return");
    }

    SymbolTable->ExitBlock();
    
    return Result;
}



KPBreakStatement::KPBreakStatement()
{
}

KPBreakStatement::~KPBreakStatement()
{
}

KPStatement* KPBreakStatement::Clone()
{
    return new KPBreakStatement();
}

string KPBreakStatement::FirstToken() const
{
    return string("break");
}

void KPBreakStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("break");
    Tokenizer->Next().MustBe(";");
}

KPStatement::TExecResult KPBreakStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;
    Result.ExecStatus = KPStatement::esBreak;

    return Result;
}



KPContinueStatement::KPContinueStatement()
{
}

KPContinueStatement::~KPContinueStatement()
{
}

KPStatement* KPContinueStatement::Clone()
{
    return new KPContinueStatement();
}

string KPContinueStatement::FirstToken() const
{
    return string("continue");
}

void KPContinueStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("continue");
    Tokenizer->Next().MustBe(";");
}

KPStatement::TExecResult KPContinueStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;
    Result.ExecStatus = KPStatement::esContinue;

    return Result;
}



KPReturnStatement::KPReturnStatement()
{
    fExpression = nullptr;
}

KPReturnStatement::~KPReturnStatement()
{
    delete fExpression;
}

KPStatement* KPReturnStatement::Clone()
{
    return new KPReturnStatement();
}

string KPReturnStatement::FirstToken() const
{
    return string("return");
}

void KPReturnStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    Tokenizer->Next().MustBe("return");

    KPToken Token = Tokenizer->Next();
    if (Token.IsNot(";")) {
        Tokenizer->Unget(Token);
        fExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
        Tokenizer->Next().MustBe(";");
    }
}

KPStatement::TExecResult KPReturnStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;

    if (fExpression != nullptr) {
        Result.ReturnValue = fExpression->Evaluate(SymbolTable);
    }

    Result.ExecStatus = KPStatement::esReturn;

    return Result;
}



KPThrowStatement::KPThrowStatement()
{
    fExceptionExpression = nullptr;
}

KPThrowStatement::~KPThrowStatement()
{
    delete fExceptionExpression;
}

KPStatement* KPThrowStatement::Clone()
{
    return new KPThrowStatement();
}

string KPThrowStatement::FirstToken() const
{
    return string("throw");
}

void KPThrowStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPExpressionParser* ExpressionParser;
    ExpressionParser = StatementParser->ExpressionParser();

    Tokenizer->Next().MustBe("throw");

    KPToken Token = Tokenizer->Next();
    if (Token.IsNot(";")) {
        Tokenizer->Unget(Token);
        fExceptionExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
        Tokenizer->Next().MustBe(";");
    }
}

KPStatement::TExecResult KPThrowStatement::Execute(KPSymbolTable* SymbolTable) 
{
    if (fExceptionExpression != nullptr) {        
	throw KPException() << fExceptionExpression->Evaluate(SymbolTable).AsString();
    }

    return TExecResult();
}



KPTryCatchStatement::KPTryCatchStatement()
{
    fTryStatement = nullptr;
    fArgumentDeclaration = nullptr;
    fCatchStatement = nullptr;
}

KPTryCatchStatement::~KPTryCatchStatement()
{
    delete fTryStatement;
    delete fArgumentDeclaration;
    delete fCatchStatement;
}

KPStatement* KPTryCatchStatement::Clone()
{
    return new KPTryCatchStatement();
}

string KPTryCatchStatement::FirstToken() const
{
    return string("try");
}

void KPTryCatchStatement::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("try");
    fTryStatement = StatementParser->Parse(Tokenizer, SymbolTable);

    if (Tokenizer->LookAhead().Is("catch")) {
	Tokenizer->Next();
	if (Tokenizer->LookAhead().Is("(")) {
	    Tokenizer->Next().MustBe("(");
            fArgumentDeclaration = new KPVariableDeclaration();
	    try {
		fArgumentDeclaration->Parse(Tokenizer, StatementParser, SymbolTable);
		Tokenizer->Next().MustBe(")");
	    }
	    catch (KPException &e) {
		delete fArgumentDeclaration;
		fArgumentDeclaration = nullptr;
		throw;
	    }
	}

	fCatchStatement = StatementParser->Parse(Tokenizer, SymbolTable);
    }
}

KPStatement::TExecResult KPTryCatchStatement::Execute(KPSymbolTable* SymbolTable) 
{
    TExecResult Result;

    try {
        Result = fTryStatement->Execute(SymbolTable);
    }
    catch (KPException &e) {
	SymbolTable->EnterBlock();

	try {
	    if (fArgumentDeclaration) {
		KPValue Argument(e.what());
		fArgumentDeclaration->Execute(SymbolTable);
		long VariableId = fArgumentDeclaration->VariableId();
		SymbolTable->GetVariable(VariableId)->Assign(Argument);
	    }
	    if (fCatchStatement) {
		fCatchStatement->Execute(SymbolTable);
	    }
	}
	catch (KPException &e) {
	    SymbolTable->ExitBlock();
	    throw;
	}

	SymbolTable->ExitBlock();
    }
    
    return Result;
}
