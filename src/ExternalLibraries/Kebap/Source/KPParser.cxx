// KPParser.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include "KPTokenizer.h"
#include "KPOperator.h"
#include "KPExpression.h"
#include "KPStatement.h"
#include "KPModule.h"
#include "KPStandardLibrary.h"
#include "KPMathLibrary.h"
#include "KPParser.h"

using namespace std;
using namespace kebap;


KPParser::KPParser()
{
    fObjectPrototypeTable = nullptr;
    fBuiltinFunctionTable = nullptr;
    fTokenTable = nullptr;
    fOperatorTable = nullptr;
    fStatementTable = nullptr;
    fModule = nullptr;

    fSymbolTable = nullptr;
    fExpressionParser = nullptr;
    fStatementParser = nullptr;

    fLineNumberOffset = 0;

    fIsConstructed = false;
}

KPParser::~KPParser()
{
    delete fStatementParser;
    delete fExpressionParser;

    delete fModule;
    delete fStatementTable;
    delete fOperatorTable;
    delete fTokenTable;
    delete fBuiltinFunctionTable;
    delete fObjectPrototypeTable;

    // Master symbol table has to be deteted at the last
    // otherwise exported symbols will be invalid

    delete fSymbolTable;
}

void KPParser::Merge(KPParser* Source)
{
    if (! fIsConstructed) {
	Construct();
    }
    Source->Construct();

    fObjectPrototypeTable->Merge(Source->fObjectPrototypeTable);
    fBuiltinFunctionTable->Merge(Source->fBuiltinFunctionTable);
    fTokenTable->Merge(Source->fTokenTable);
    fOperatorTable->Merge(Source->fOperatorTable);
    fStatementTable->Merge(Source->fStatementTable);
    fModule->Merge(Source->fModule);
}

void KPParser::Parse(istream& SourceStream) 
{
    if (! fIsConstructed) {
	Construct();
    }

    KPTokenizer Tokenizer(SourceStream, fTokenTable);
    Tokenizer.SetLineNumber(fLineNumberOffset);

    fModule->Parse(&Tokenizer, fStatementParser, fSymbolTable);
}

KPValue KPParser::Execute(const string& EntryName) 
{
    return fModule->Execute(EntryName, fSymbolTable);
}

KPValue KPParser::Execute(const std::string& EntryName, const std::vector<KPValue*>& ArgumentList) 
{
    return fModule->Execute(EntryName, ArgumentList, fSymbolTable);
}

bool KPParser::HasEntryOf(const string& EntryName) const
{
    return (fModule->GetEntry(EntryName) != nullptr);
}

KPTokenTable* KPParser::GetTokenTable()
{
    if (! fIsConstructed) {
	Construct();
    }

    return fTokenTable;
}

KPSymbolTable* KPParser::GetSymbolTable()
{
    if (! fIsConstructed) {
	Construct();
    }

    return fSymbolTable;
}

KPExpressionParser* KPParser::GetExpressionParser()
{
    if (! fIsConstructed) {
	Construct();
    }

    return fExpressionParser;
}

KPStatementParser* KPParser::GetStatementParser()
{
    if (! fIsConstructed) {
	Construct();
    }

    return fStatementParser;
}

KPModule* KPParser::GetModule()
{
    if (! fIsConstructed) {
	Construct();
    }

    return fModule;
}

void KPParser::Construct()
{
    if (fIsConstructed) {
	return;
    }

    fObjectPrototypeTable = CreateObjectPrototypeTable();
    fBuiltinFunctionTable = CreateBuiltinFunctionTable();
    fTokenTable = CreateTokenTable();
    fOperatorTable = CreateOperatorTable();
    fStatementTable = CreateStatementTable();
    fModule = CreateModule();

    fSymbolTable = new KPSymbolTable(fObjectPrototypeTable, fBuiltinFunctionTable);
    fExpressionParser = new KPExpressionParser(fOperatorTable);
    fStatementParser = new KPStatementParser(fStatementTable, fExpressionParser);

    KPObjectPrototype* Cin = new KPInputFileObject(&cin);
    KPObjectPrototype* Cout = new KPOutputFileObject(&cout);
    KPObjectPrototype* Cerr = new KPOutputFileObject(&cerr);

    fSymbolTable->RegisterVariable("cin", new KPValue(Cin));
    fSymbolTable->RegisterVariable("cout", new KPValue(Cout));
    fSymbolTable->RegisterVariable("cerr", new KPValue(Cerr));

    fIsConstructed = true;
    OnConstruct();
}

void KPParser::SetLineNumberOffset(long LineNumberOffset)
{
    fLineNumberOffset = LineNumberOffset;
}

KPObjectPrototypeTable* KPParser::CreateObjectPrototypeTable()
{
    return new KPObjectPrototypeTable();
}

KPBuiltinFunctionTable* KPParser::CreateBuiltinFunctionTable()
{
    return new KPBuiltinFunctionTable();
}

KPTokenTable* KPParser::CreateTokenTable()
{
    return new KPTokenTable();
}

KPOperatorTable* KPParser::CreateOperatorTable()
{
    return new KPOperatorTable();
}

KPStatementTable* KPParser::CreateStatementTable()
{
    return new KPStatementTable();
}

KPModule* KPParser::CreateModule()
{
    return new KPModule();
}



KPStandardParser::KPStandardParser()
{
    fArgc = 0;
    fArgv = nullptr;
}

KPStandardParser::KPStandardParser(int argc, char** argv)
{
    fArgc = argc;
    fArgv = argv;
}

KPStandardParser::~KPStandardParser()
{
}

KPTokenTable* KPStandardParser::CreateTokenTable()
{
    return new KPCxxTokenTable();
}

KPOperatorTable* KPStandardParser::CreateOperatorTable()
{
    return new KPCxxOperatorTable();
}

KPStatementTable* KPStandardParser::CreateStatementTable()
{
    return new KPCxxStatementTable();
}

KPModule* KPStandardParser::CreateModule()
{
    return new KPCxxModule();
}

KPObjectPrototypeTable* KPStandardParser::CreateObjectPrototypeTable()
{
    KPObjectPrototypeTable* ObjectPrototypeTable;
    ObjectPrototypeTable = KPParser::CreateObjectPrototypeTable();

    ObjectPrototypeTable->RegisterClass("InputFile", new KPInputFileObject);
    ObjectPrototypeTable->RegisterClass("OutputFile", new KPOutputFileObject);
    ObjectPrototypeTable->RegisterClass("InputPipe", new KPInputPipeObject);
    ObjectPrototypeTable->RegisterClass("OutputPipe", new KPOutputPipeObject);

    ObjectPrototypeTable->RegisterClass("Formatter", new KPFormatterObject);
    ObjectPrototypeTable->RegisterClass("Scanner", new KPScannerObject);

    return ObjectPrototypeTable;
}

KPBuiltinFunctionTable* KPStandardParser::CreateBuiltinFunctionTable()
{
    KPBuiltinFunctionTable* BuiltinFunctionTable;
    BuiltinFunctionTable = KPParser::CreateBuiltinFunctionTable();

    BuiltinFunctionTable->RegisterStaticObject(new KPConsoleObject);
    BuiltinFunctionTable->RegisterStaticObject(new KPMathObject);
    BuiltinFunctionTable->RegisterStaticObject(new KPListMathObject);
    BuiltinFunctionTable->RegisterStaticObject(new KPArgumentObject(fArgc, fArgv));
    BuiltinFunctionTable->RegisterStaticObject(new KPStringObject());
    BuiltinFunctionTable->RegisterStaticObject(new KPSystemObject());
    BuiltinFunctionTable->RegisterStaticObject(new KPParserObject(this));

    return BuiltinFunctionTable;
}
