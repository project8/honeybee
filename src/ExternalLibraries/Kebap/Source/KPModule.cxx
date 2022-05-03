// KPModule.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include <cstdlib>
#include "KPTokenizer.h"
#include "KPSymbolTable.h"
#include "KPStatement.h"
#include "KPFunction.h"
#include "KPModule.h"

using namespace std;
using namespace kebap;


KPModule::KPModule()
{
    fNumberOfProcessedBareStatements = 0;
}

KPModule::~KPModule()
{
    for (unsigned i = 0; i < fEntryList.size(); i++) {
	delete fEntryList[i];
    }

    for (unsigned j = 0; j < fEntryPrototypeList.size(); j++) {
	delete fEntryPrototypeList[j];
    }

    for (unsigned k = 0; k < fBareStatementList.size(); k++) {
	delete fBareStatementList[k];
    }
}

void KPModule::Merge(KPModule* Source)
{
    for (unsigned i = 0; i < Source->fEntryList.size(); i++) {
	AddEntry(Source->fEntryList[i]->Clone());
    }
}

void KPModule::AddEntry(KPModuleEntry* EntryPrototype)
{
    fEntryPrototypeList.push_back(EntryPrototype);
}

KPModuleEntry* KPModule::CreateEntry(KPTokenizer* Tokenizer)
{
    KPModuleEntry* Entry = nullptr;

    vector<KPModuleEntry*>::reverse_iterator EntryPrototype;
    for (
	EntryPrototype = fEntryPrototypeList.rbegin();
	EntryPrototype != fEntryPrototypeList.rend();
	EntryPrototype++
    ){
	if ((*EntryPrototype)->HasEntryWordsOf(Tokenizer)) {
	    Entry = (*EntryPrototype)->Clone();
	    break;
	}
    }

    return Entry;
}

void KPModule::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    while (! Tokenizer->LookAhead().IsEmpty()) {
	KPModuleEntry* Entry = CreateEntry(Tokenizer);

	if (Entry != nullptr) {
	    try {
		Entry->Parse(Tokenizer, StatementParser, SymbolTable);
	    }
	    catch (KPException &e) {
		delete Entry;
		throw;
	    }
	    fEntryList.push_back(Entry);

	    string EntryName = Entry->EntryName();
	    if (EntryName.size() > 0) {
		fEntryTable[EntryName] = Entry;
		fEntryNameList.push_back(EntryName);
	    }
	}
	else {
	    KPStatement* Statement = StatementParser->Parse(
		Tokenizer, SymbolTable
	    );
	    fBareStatementList.push_back(Statement);
        }
    }
}

void KPModule::ExecuteBareStatements(KPSymbolTable* SymbolTable) 
{
    KPStatement::TExecResult Result;
    for (
	unsigned i = fNumberOfProcessedBareStatements;
	i < fBareStatementList.size();
	i++
    ){
	Result = fBareStatementList[i]->Execute(SymbolTable);
	if (Result.ExecStatus != KPStatement::esNormal) {
	    break;
	}
    }

    fNumberOfProcessedBareStatements = fBareStatementList.size();
}

KPValue KPModule::Execute(KPSymbolTable* SymbolTable) 
{
    string EntryName;
    vector<KPValue*> ArgumentList;
    return Execute(EntryName, ArgumentList, SymbolTable);
}

KPValue KPModule::Execute(const string& EntryName, KPSymbolTable* SymbolTable) 
{
    vector<KPValue*> ArgumentList;
    return Execute(EntryName, ArgumentList, SymbolTable);
}

KPValue KPModule::Execute(const string& EntryName, const vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) 
{
    if (fNumberOfProcessedBareStatements < fBareStatementList.size()) {
	ExecuteBareStatements(SymbolTable);
    }

    if (EntryName.empty()) {
	return KPValue((long) 0);
    }
    else if (fEntryTable.count(EntryName) > 0) {
	return fEntryTable[EntryName]->Execute(ArgumentList, SymbolTable);
    }
    else {
	throw KPException() << "unknown entry: " << EntryName;
    }
}

KPModuleEntry* KPModule::GetEntry(const string& EntryName)
{
    return (fEntryTable.count(EntryName) > 0) ? fEntryTable[EntryName] : nullptr;
}

const vector<KPModuleEntry*>& KPModule::EntryList() const
{
    return fEntryList;
}

const vector<string>& KPModule::EntryNameList() const
{
    return fEntryNameList;
}



KPCxxModule::KPCxxModule()
{
    AddEntry(new KPFunctionEntry());
    AddEntry(new KPIncludeEntry());
}

KPCxxModule::~KPCxxModule()
{
}



KPModuleEntry::KPModuleEntry(const string& EntryTypeName)
{
    fEntryTypeName = EntryTypeName;
    fEntryName = "";
}

KPModuleEntry::~KPModuleEntry()
{
}

const string& KPModuleEntry::EntryTypeName() const
{
    return fEntryTypeName;
}

const string& KPModuleEntry::EntryName() const
{
    return fEntryName;
}

void KPModuleEntry::SetEntryName(const string& EntryName)
{
    fEntryName = EntryName;
}

KPValue KPModuleEntry::Execute(const vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) 
{
    return KPValue((long) 0);
}



KPFunctionEntry::KPFunctionEntry()
: KPModuleEntry("Function")
{
    fFunction = nullptr;
}

KPFunctionEntry::~KPFunctionEntry()
{
    delete fFunction;
}

KPModuleEntry* KPFunctionEntry::Clone()
{
    return new KPFunctionEntry();
}

bool KPFunctionEntry::HasEntryWordsOf(KPTokenizer* Tokenizer)
{
    KPToken Token;
    int Index = 1;

    // return-value type declaration //
    Token = Tokenizer->LookAhead(Index++);
#if 0
    //... FIXME: SymbolTable is not accessible here
    if (! fSymbolTable->IsTypeName(Token.AsString())) {
	return false;
    }
#else
    if (! (Token.IsIdentifier() || Token.IsKeyword())) {
	return false;
    }
#endif
    while ((Token = Tokenizer->LookAhead(Index)).Is("*")) {
	Index++;
    }
        
    // function name declaration //
    Token = Tokenizer->LookAhead(Index++);
    if (! Token.IsIdentifier()) {
	return false;
    }
    
    // function parameter list declaration //
    Token = Tokenizer->LookAhead(Index++);
    if (Token.IsNot("(")) {
	return false;
    }
    
    Token = Tokenizer->LookAhead(Index++);
    if (Token.Is("void")) {
	return Tokenizer->LookAhead(Index++).Is(")");
    }
    if (Token.Is(")")) {
	return Tokenizer->LookAhead(Index++).IsNot(";");
    }
    if (! (Token.IsIdentifier() || (Token.IsKeyword()))) {
	// the token must be a type name
	return false;
    }
    
    while ((Token = Tokenizer->LookAhead(Index)).Is("*")) {
	Index++;
    }
    
    Token = Tokenizer->LookAhead(Index++);
    if (! Token.IsIdentifier()) {
	// the token must be a variable name
	return false;
    }

    return true;
}

void KPFunctionEntry::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    fFunction = new KPCxxFunction();
    try {
	fFunction->Parse(Tokenizer, StatementParser, SymbolTable);
    }
    catch (KPException &e) {
	delete fFunction;
	fFunction = nullptr;
	throw;
    }

    string FunctionName = fFunction->Name();
    long FunctionId = SymbolTable->NameToId(FunctionName);

    SymbolTable->RegisterFunction(FunctionId, fFunction);
    SetEntryName(FunctionName);
}

KPValue KPFunctionEntry::Execute(const vector<KPValue*>& ArgumentList, KPSymbolTable* SymbolTable) 
{
    return fFunction->Execute(ArgumentList, SymbolTable);
}



KPIncludeEntry::KPIncludeEntry()
: KPModuleEntry("Include")
{
    fInputFile = nullptr;
}

KPIncludeEntry::~KPIncludeEntry()
{
    delete fInputFile;
}

KPModuleEntry* KPIncludeEntry::Clone()
{
    return new KPIncludeEntry();
}

bool KPIncludeEntry::HasEntryWordsOf(KPTokenizer* Tokenizer)
{
    return Tokenizer->LookAhead().Is("include");
}

void KPIncludeEntry::Parse(KPTokenizer* Tokenizer, KPStatementParser* StatementParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("include");
    string FileName = Tokenizer->Next().RemoveQuotation('"').AsString();

    if (Tokenizer->LookAhead().Is("from")) {
	Tokenizer->Next();
	string PathListName = Tokenizer->Next().AsString();
	char* PathListValue = getenv(PathListName.c_str());
	if (PathListValue == nullptr) {
	    throw KPException() << "include: undefined environmental variable: " << PathListName;
	}
	string PathList = PathListValue;

	while (! PathList.empty()) {
	    if (PathList[0] == ':') {
		PathList.erase(PathList.begin());
		continue;
	    }

	    string::size_type Length = PathList.find_first_of(':');
	    string Path = PathList.substr(0, Length);

	    fInputFile = new ifstream((Path + "/" + FileName).c_str());
	    if (*fInputFile) {
		break;
	    }

	    PathList.erase(0, Length);
	}

	if (PathList.empty()) {
	    throw KPException() << "include: unable to find file '" << FileName << "' from path '" << string(getenv(PathListName.c_str())) + "'";
	}
    }
    else {
	fInputFile = new ifstream(FileName.c_str());
	if (! *fInputFile) {
	    delete fInputFile;
	    fInputFile = nullptr;
	    throw KPException() << "include: unable to find file: " << FileName;
	}
    }

    if (Tokenizer->LookAhead().IsNot(";")) {
	delete fInputFile;
	fInputFile = nullptr;
    }
    Tokenizer->Next().MustBe(";");

    Tokenizer->InputBuffer()->SetChildInput(*fInputFile);
}
