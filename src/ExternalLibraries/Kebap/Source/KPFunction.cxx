// KPFunction.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include "KPTokenizer.h"
#include "KPExpression.h"
#include "KPSymbolTable.h"
#include "KPStatement.h"
#include "KPFunction.h"

using namespace std;
using namespace kebap;


KPFunction::KPFunction()
{
    fStatement = new KPEmptyStatement();
    fReturnValue = new KPValue();
}

KPFunction::~KPFunction()
{
    for (unsigned i = 0; i < fArgumentDeclarationList.size(); i++) {
        delete fArgumentDeclarationList[i];
    }
    
    delete fReturnValue;
    delete fStatement;
}

KPValue KPFunction::Execute(const vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) 
{
    SymbolTable->EnterBlock();
    
    KPStatement::TExecResult Result;
    try {
	ProcessArguments(ArgumentList, SymbolTable);
	Result = fStatement->Execute(SymbolTable); 
    }
    catch (KPException &e) {
	SymbolTable->ExitBlock();
	throw;
    }

    SymbolTable->ExitBlock();

    try {
	fReturnValue->Assign(Result.ReturnValue);
    }
    catch (KPException &e) {
	throw KPException() << 
	    fName << "(): bad return-value type: " << e.what();
    }

    return *fReturnValue;
}

void KPFunction::ProcessArguments(const vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) 
{
    if (ArgumentList.size() > fArgumentDeclarationList.size()) {
        throw KPException() << fName << "(): too many aguments";
    }

    for (unsigned i = 0; i < fArgumentDeclarationList.size(); i++) {
        fArgumentDeclarationList[i]->Execute(SymbolTable);

	if (i < ArgumentList.size()) {
	    long VariableId = fArgumentDeclarationList[i]->VariableId();
	    SymbolTable->GetVariable(VariableId)->Assign(*ArgumentList[i]);
	}
    }
}

string KPFunction::Name()
{
    return fName;
}

void KPFunction::SetName(const string& Name)
{
    fName = Name;
}

void KPFunction::SetReturnValue(KPValue* ReturnValue)
{
    delete fReturnValue;
    fReturnValue = ReturnValue;
}

void KPFunction::AddArgumentDeclaration(KPVariableDeclaration* ArgumentDeclaration)
{
    fArgumentDeclarationList.push_back(ArgumentDeclaration);
}

void KPFunction::SetStatement(KPStatement* Statement)
{
    delete fStatement;
    fStatement = Statement;
}



KPCxxFunction::KPCxxFunction()
{
}

KPCxxFunction::~KPCxxFunction()
{
}

void KPCxxFunction::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    KPToken Token = Tokenizer->Next();    
    KPValue* ReturnValue = SymbolTable->CreateObject(Token.AsString());
    if (ReturnValue == nullptr) {
	Token.ThrowUnexpected("type name");
    }
    while ((Token = Tokenizer->Next()).Is("*")) {
	delete ReturnValue;
	ReturnValue = new KPValue((KPValue*) nullptr);
    }
    SetReturnValue(ReturnValue);

    if (! Token.IsIdentifier()) {
        Token.ThrowUnexpected("function name");
    }
    string FunctionName = Token.AsString();
    SetName(FunctionName);

    ParseArgumentDeclaration(Tokenizer, StatementParser, SymbolTable);
    
    KPStatement* Statement = StatementParser->Parse(Tokenizer, SymbolTable);
    SetStatement(Statement);
}

void KPCxxFunction::ParseArgumentDeclaration(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("(");

    if (Tokenizer->LookAhead().Is("void")) {
	Tokenizer->Next();
	Tokenizer->LookAhead().MustBe(")");
    }

    KPToken Token;
    if (Tokenizer->LookAhead().Is(")")) {
	Token = Tokenizer->Next();
    }
    else {
    KPVariableDeclaration* VariableDeclaration = nullptr;
	do {
	    try {
		VariableDeclaration = new KPVariableDeclaration();
		VariableDeclaration->Parse(Tokenizer, StatementParser, SymbolTable);
	    }
	    catch (KPException &e) {
		delete VariableDeclaration;
		throw;
	    }
            AddArgumentDeclaration(VariableDeclaration);
	} while ((Token = Tokenizer->Next()).Is(","));
    }

    Token.MustBe(")");
}
