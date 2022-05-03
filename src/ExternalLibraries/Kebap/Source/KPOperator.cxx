// KPOperator.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.

#include <string>
#include <cstdlib>
#include <cmath>
#include "KPObject.h"
#include "KPValue.h"
#include "KPTokenizer.h"
#include "KPExpression.h"
#include "KPOperator.h"

using namespace std;
using namespace kebap;


KPOperatorPriority::KPOperatorPriority(int PriorityRank)
{
    fPriorityRank = PriorityRank;
    fBaseOperatorSymbol = "";
    fPriorityOffset = 0;
}

KPOperatorPriority::KPOperatorPriority(const string& BaseOperatorSymbol, int PriorityOffset, int TemporaryPriorityRank)
{
    fPriorityRank = TemporaryPriorityRank;
    fBaseOperatorSymbol = BaseOperatorSymbol;
    fPriorityOffset = PriorityOffset;
}

KPOperatorPriority::KPOperatorPriority(const KPOperatorPriority& Priority)
{
    fPriorityRank = Priority.fPriorityRank;
    fBaseOperatorSymbol = Priority.fBaseOperatorSymbol;
    fPriorityOffset = Priority.fPriorityOffset;
}

KPOperatorPriority::~KPOperatorPriority()
{
}

KPOperatorPriority& KPOperatorPriority::operator=(const KPOperatorPriority& Priority)
{
    fPriorityRank = Priority.fPriorityRank;
    fBaseOperatorSymbol = Priority.fBaseOperatorSymbol;
    fPriorityOffset = Priority.fPriorityOffset;

    return *this;
}

void KPOperatorPriority::SetPriorityRank(int PriorityRank)
{
    fPriorityRank = PriorityRank;
}

int KPOperatorPriority::PriorityRank() const
{
    return fPriorityRank;
}

const string& KPOperatorPriority::BaseOperatorSymbol() const
{
    return fBaseOperatorSymbol;
}

int KPOperatorPriority::PriorityOffset() const
{
    return fPriorityOffset;
}



KPOperatorTable::KPOperatorTable()
{
    fHighestPriorityRank = 1;
    fLowestPriorityRank = 0;
}

KPOperatorTable::~KPOperatorTable()
{
    map<string, KPOperator*>::iterator OperatorEntry;
    for (
	OperatorEntry = fOperatorTable.begin();
	OperatorEntry != fOperatorTable.end();
	OperatorEntry++
    ){
	delete (*OperatorEntry).second;
    }

    for (
	OperatorEntry = fPrepositionalOperatorTable.begin();
	OperatorEntry != fPrepositionalOperatorTable.end();
	OperatorEntry++
    ){
	delete (*OperatorEntry).second;
    }

    for (
	OperatorEntry = fPostpositionalOperatorTable.begin();
	OperatorEntry != fPostpositionalOperatorTable.end();
	OperatorEntry++
    ){
	delete (*OperatorEntry).second;
    }

    for (
	OperatorEntry = fElementaryOperatorTable.begin();
	OperatorEntry != fElementaryOperatorTable.end();
	OperatorEntry++
    ){
	delete (*OperatorEntry).second;
    }
}

void KPOperatorTable::Merge(KPOperatorTable* Source)
{
    map<string, KPOperator*>::iterator OperatorTableIterator;
    for (
	OperatorTableIterator = Source->fOperatorTable.begin();
	OperatorTableIterator != Source->fOperatorTable.end();
	OperatorTableIterator++
    ){
	string Symbol = (*OperatorTableIterator).first;
	const KPOperatorPriority& Priority = Source->PriorityOf(Symbol);
	KPOperator* Operator = (*OperatorTableIterator).second;
	AddOperator(Operator->Clone(), Priority);
    }

    for (
	OperatorTableIterator = Source->fPrepositionalOperatorTable.begin();
	OperatorTableIterator != Source->fPrepositionalOperatorTable.end();
	OperatorTableIterator++
    ){
	string Symbol = (*OperatorTableIterator).first;
	KPOperator* Operator = (*OperatorTableIterator).second;
	AddPrepositionalOperator(Operator->Clone());
    }

    for (
	OperatorTableIterator = Source->fPostpositionalOperatorTable.begin();
	OperatorTableIterator != Source->fPostpositionalOperatorTable.end();
	OperatorTableIterator++
    ){
	string Symbol = (*OperatorTableIterator).first;
	KPOperator* Operator = (*OperatorTableIterator).second;
	AddPostpositionalOperator(Operator->Clone());
    }

    for (
	OperatorTableIterator = Source->fElementaryOperatorTable.begin();
	OperatorTableIterator != Source->fElementaryOperatorTable.end();
	OperatorTableIterator++
    ){
	string Symbol = (*OperatorTableIterator).first;
	KPOperator* Operator = (*OperatorTableIterator).second;
	AddElementaryOperator(Operator->Clone());
    }
}

void KPOperatorTable::AddOperator(KPOperator* Operator, int PriorityRank)
{
    AddOperator(Operator, KPOperatorPriority(PriorityRank));
}

void KPOperatorTable::AddOperator(KPOperator* Operator, const KPOperatorPriority& Priority)
{
    string Symbol = Operator->Symbol();

    // if the same operator symbol has already been registered,
    // it will be overwritten.
    if (fOperatorTable.count(Symbol) > 0) {
	delete fOperatorTable[Symbol];
    }
    
    int PriorityRank = Priority.PriorityRank();
    fPriorityTable[Symbol] = Priority;
    fOperatorTable[Symbol] = Operator;

    const string& BaseOperatorSymbol = Priority.BaseOperatorSymbol();
    if (! BaseOperatorSymbol.empty()) {
	if (fPriorityRankTable.count(BaseOperatorSymbol) > 0) {
	    int BasePriorityRank = fPriorityRankTable[BaseOperatorSymbol];
	    PriorityRank = BasePriorityRank + Priority.PriorityOffset();
	    fPriorityTable[Symbol].SetPriorityRank(PriorityRank);
	}
    }

    if (PriorityRank < 0) {
	PriorityRank = fLowestPriorityRank;
    }

    fPriorityRankTable[Symbol] = PriorityRank;
    if (fHighestPriorityRank > PriorityRank) {
        fHighestPriorityRank = PriorityRank;
    }
    if (fLowestPriorityRank < PriorityRank) {
        fLowestPriorityRank = PriorityRank;
    }
}

void KPOperatorTable::AddPrepositionalOperator(KPOperator* Operator)
{
    string Symbol = Operator->Symbol();

    // if the same operator symbol has already been registered,
    // it will be overwritten.
    if (fPrepositionalOperatorTable.count(Symbol) > 0) {
	delete fPrepositionalOperatorTable[Symbol];
    }

    fPrepositionalOperatorTable[Symbol] = Operator;
}

void KPOperatorTable::AddPostpositionalOperator(KPOperator* Operator)
{
    string Symbol = Operator->Symbol();

    // if the same operator symbol has already been registered,
    // it will be overwritten.
    if (fPostpositionalOperatorTable.count(Symbol) > 0) {
	delete fPostpositionalOperatorTable[Symbol];
    }

    fPostpositionalOperatorTable[Symbol] = Operator;
}

void KPOperatorTable::AddElementaryOperator(KPOperator* Operator)
{
    string Symbol = Operator->Symbol();

    // if the same operator symbol has already been registered,
    // it will be overwritten.
    if (fElementaryOperatorTable.count(Symbol) > 0) {
	delete fElementaryOperatorTable[Symbol];
    }

    fElementaryOperatorTable[Symbol] = Operator;
}

KPOperator* KPOperatorTable::CreateOperator(const string& Symbol)
{
    if (fOperatorTable.count(Symbol) > 0) {
        return fOperatorTable[Symbol]->Clone();
    }
    else {
        return nullptr;
    }
}

KPOperator* KPOperatorTable::CreatePrepositionalOperator(const string& Symbol)
{
    if (fPrepositionalOperatorTable.count(Symbol) > 0) {
        return fPrepositionalOperatorTable[Symbol]->Clone();
    }
    else {
        return nullptr;
    }
}

KPOperator* KPOperatorTable::CreatePostpositionalOperator(const string& Symbol)
{
    if (fPostpositionalOperatorTable.count(Symbol) > 0) {
        return fPostpositionalOperatorTable[Symbol]->Clone();
    }
    else {
        return nullptr;
    }
}

KPOperator* KPOperatorTable::CreateElementaryOperator(const string& Symbol)
{
    if (fElementaryOperatorTable.count(Symbol) > 0) {
        return fElementaryOperatorTable[Symbol]->Clone();
    }
    else {
        return nullptr;
    }
}

const KPOperatorPriority& KPOperatorTable::PriorityOf(const string& Symbol)
{
    if (fOperatorTable.count(Symbol) == 0) {
        throw KPException() << "KPOperatorTable::PriorityOf(): inconsistent operator table (internal error)";
    }

    return fPriorityTable[Symbol];
}

int KPOperatorTable::PriorityRankOf(const string& Symbol)
{
    if (fOperatorTable.count(Symbol) > 0) {
        return fPriorityRankTable[Symbol];
    }
    else {
        return 0;
    }
}

int KPOperatorTable::HighestPriorityRank() const
{
    return fHighestPriorityRank;
}

int KPOperatorTable::LowestPriorityRank() const
{
    return fLowestPriorityRank;
}



KPCxxOperatorTable::KPCxxOperatorTable()
{
    AddElementaryOperator(new KPOperatorNew());
    AddElementaryOperator(new KPOperatorListGenerate());
    AddElementaryOperator(new KPOperatorVariableAccess());

    AddPrepositionalOperator(new KPOperatorDelete());
    AddPrepositionalOperator(new KPOperatorSizeOf());
    AddPrepositionalOperator(new KPOperatorTypeOf());
    AddPrepositionalOperator(new KPOperatorKeys());
    AddPrepositionalOperator(new KPOperatorPointerReference());
    AddPrepositionalOperator(new KPOperatorAddress());
    AddPrepositionalOperator(new KPOperatorIncrement());
    AddPrepositionalOperator(new KPOperatorDecrement());
    AddPrepositionalOperator(new KPOperatorSignPlus());
    AddPrepositionalOperator(new KPOperatorSignMinus());
    AddPrepositionalOperator(new KPOperatorNot());
    AddPrepositionalOperator(new KPOperatorBitReverse());

    AddPostpositionalOperator(new KPOperatorPostpositionalIncrement());
    AddPostpositionalOperator(new KPOperatorPostpositionalDecrement());
    AddPostpositionalOperator(new KPOperatorFactorial());
    AddPostpositionalOperator(new KPOperatorPower());
    AddPostpositionalOperator(new KPOperatorFunctionCall());
    AddPostpositionalOperator(new KPOperatorArrayReference());
    AddPostpositionalOperator(new KPOperatorTableReference());
    
    int PriorityRank;
    //AddOperator(new KPOperatorPower(), PriorityRank = 2);
    AddOperator(new KPOperatorMultiple(), PriorityRank = 4);
    AddOperator(new KPOperatorDivide(), PriorityRank = 4);
    AddOperator(new KPOperatorModulo(), PriorityRank = 4);
    AddOperator(new KPOperatorAdd(), PriorityRank = 6);
    AddOperator(new KPOperatorSubtract(), PriorityRank = 6);
    AddOperator(new KPOperatorConcatenate(), PriorityRank = 6);
    AddOperator(new KPOperatorLeftShift(), PriorityRank = 8);
    AddOperator(new KPOperatorRightShift(), PriorityRank = 8);
    AddOperator(new KPOperatorGreaterThan(), PriorityRank = 10);
    AddOperator(new KPOperatorLessThan(), PriorityRank = 10);
    AddOperator(new KPOperatorGreaterEqual(), PriorityRank = 10);
    AddOperator(new KPOperatorLessEqual(), PriorityRank = 10);
    AddOperator(new KPOperatorEqual(), PriorityRank = 12);
    AddOperator(new KPOperatorNotEqual(), PriorityRank = 12);
    AddOperator(new KPOperatorBitAnd(), PriorityRank = 14);
    AddOperator(new KPOperatorBitXor(), PriorityRank = 16);
    AddOperator(new KPOperatorBitOr(), PriorityRank = 18);
    AddOperator(new KPOperatorListAnd(), PriorityRank = 14);
    AddOperator(new KPOperatorAnd(), PriorityRank = 20);
    AddOperator(new KPOperatorOr(), PriorityRank = 22);
    AddOperator(new KPOperatorAssign(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignSum(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignDifference(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignProduct(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignQuotient(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignRemainder(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignBitAnd(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignBitOr(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignBitXor(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignRightShift(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignLeftShift(), PriorityRank = 24);
    AddOperator(new KPOperatorAssignConcatenation(), PriorityRank = 24);
}

KPCxxOperatorTable::~KPCxxOperatorTable()
{
}



KPOperator::KPOperator()
{
}

KPOperator::~KPOperator()
{
}

bool KPOperator::IsLeftAssociative() const
{
    return true;
}

KPExpression* KPOperator::InternalExpression(int Index)
{
    return nullptr;
}

void KPOperator::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next();
}

KPValue& KPOperator::EvaluateList(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    if (Left.IsList() && Right.IsList()) {
	if (Left.AsValueList().size() != Right.AsValueList().size()) {
	    throw KPException() << "inconsistent list length";
	}
    }

    int Length = Left.IsList() ? Left.AsValueList().size() : Right.AsValueList().size();

    Result = KPValue(KPListValue(Length));
    vector<KPValue>& ListResult = Result.AsValueList();
    ListResult.reserve(Length);

    for (int i = 0; i < Length; i++) {
	KPValue& ThisLeft = Left.IsList() ? Left.AsValueList()[i] : Left;
	KPValue& ThisRight = Right.IsList() ? Right.AsValueList()[i] : Right;
	KPValue ElementResult;
	Evaluate(ThisLeft, ThisRight, SymbolTable, ElementResult);

	ListResult.push_back(ElementResult);
    }

    KPListValue* InputListValue = nullptr;
    if (Left.IsList() && Left.AsConstList().HasKeyIndex()) {
	InputListValue = &Left.AsList();
    }
    else if (Right.IsList() && Right.AsConstList().HasKeyIndex()) {
	InputListValue = &Right.AsList();
    }
    if (InputListValue) {
	KPListValue& ResultListValue = Result.AsList();
	for (int Index = 0; Index < Length; Index++) {
	    ResultListValue.SetKey(Index, InputListValue->KeyOf(Index));
	}
    }

    return Result;
}



KPOperatorNew::KPOperatorNew()
{
    fLengthExpression = nullptr;
}

KPOperatorNew::~KPOperatorNew()
{
    delete fLengthExpression;

    for (unsigned i = 0; i < fArgumentList.size(); i++) {
	delete fArgumentList[i];
    }
}

KPOperator* KPOperatorNew::Clone() const
{
    return new KPOperatorNew();
}

string KPOperatorNew::Symbol() const
{
    return string("new");
}

string KPOperatorNew::Name() const
{
    return string("New");
}

void KPOperatorNew::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next();
    fTypeName = Tokenizer->Next().AsString();

    while (Tokenizer->LookAhead().Is("*")) {
	fTypeName = "pointer";
    }

    if (Tokenizer->LookAhead().Is("[")) {
	Tokenizer->Next().MustBe("[");
	fLengthExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
	Tokenizer->Next().MustBe("]");
    }

    if (Tokenizer->LookAhead().Is("(")) {
	fArgumentList = ExpressionParser->ParseExpressionList(
	    Tokenizer, SymbolTable, "(", ")", ","
	);
    }
}

KPValue& KPOperatorNew::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    int Length = 0;
    if (fLengthExpression != nullptr) {
	Length = fLengthExpression->Evaluate(SymbolTable).AsLong();
	if (Length < 1) {
	    throw KPException() << "new: invalid array length";
	}
    }

    vector<KPValue*> ArgumentValueList;
    if (fArgumentList.size() > 0) {
 	for (unsigned i = 0; i < fArgumentList.size(); i++) {
	    KPValue& ArgumentValue = fArgumentList[i]->Evaluate(SymbolTable);
	    ArgumentValueList.push_back(&ArgumentValue);
	}
    }

    KPValue* Instance = SymbolTable->CreateObject(fTypeName, Length);
    if (Instance == nullptr) {
        throw KPException() << "new: unknown variable type: " << fTypeName;
    }
    Instance->Refer();

    try {
	for (int i = 0; i < ((Length > 0) ? Length : 1); i++) {
	    if (Instance[i].IsObject()) {
		Instance[i].SetName(Instance[i].AsString());
		Instance[i].AsObject()->Construct(fTypeName, ArgumentValueList);
	    }
	    else {
		if (ArgumentValueList.size() > 1) {
		    throw KPException() << "new: too many initial values";
		}
		else if (ArgumentValueList.size() > 0) {
		    Instance[i].Assign(*ArgumentValueList[0]);
		}
	    }
	    Instance[i].SetLeftValueFlag();
	}
    }
    catch (KPException &e) {
	Instance->Destroy();
	if (Length > 1) {
	    delete[] Instance;
	}
	else {
	    delete Instance;
	}
	throw;
    }

    return Result = KPValue(Instance);
}



KPOperatorDelete::KPOperatorDelete()
{
    fIsForArray = false;
}

KPOperatorDelete::~KPOperatorDelete()
{
}

void KPOperatorDelete::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next();
    
    if (Tokenizer->LookAhead().Is("[")) {
	Tokenizer->Next().MustBe("[");
	Tokenizer->Next().MustBe("]");
	fIsForArray = true;
    }
}

KPOperator* KPOperatorDelete::Clone() const
{
    return new KPOperatorDelete();
}

string KPOperatorDelete::Symbol() const
{
    return string("delete");
}

string KPOperatorDelete::Name() const
{
    return string("Delete");
}

KPValue& KPOperatorDelete::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Right.AsPointer() != nullptr) {
	KPValue* Instance = Right.AsPointer();

        // array elements are destroy()ed in destroy()
	Instance->Destroy(); 

	if (fIsForArray) {
	    delete[] Instance;
	}
	else {
	    delete Instance;
	}
    }

    Right.Assign(KPValue((KPValue*) nullptr));
    
    return Result = KPValue((KPValue*) nullptr);
}



KPSymbolTable KPOperatorVariableAccess::fLocalSymbolTable;

KPOperatorVariableAccess::KPOperatorVariableAccess()
{
    fVariableNameExpression = nullptr;
}

KPOperatorVariableAccess::~KPOperatorVariableAccess()
{
    delete fVariableNameExpression;
}

KPSymbolTable& KPOperatorVariableAccess::LocalSymbolTable()
{
    return fLocalSymbolTable;
}

KPOperator* KPOperatorVariableAccess::Clone() const
{
    return new KPOperatorVariableAccess();
}

string KPOperatorVariableAccess::Symbol() const
{
    return string("$");
}

string KPOperatorVariableAccess::Name() const
{
    return string("VariableAccess");
}

void KPOperatorVariableAccess::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next();
    if (Tokenizer->LookAhead().Is("{")) {
	Tokenizer->Next().MustBe("{");
	fVariableNameExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
	Tokenizer->Next().MustBe("}");
    }
    else {
	fVariableName = Tokenizer->Next().AsString();
	if (fVariableName == "$") {
	    if (Tokenizer->LookAhead().IsIdentifier()) {
		fVariableName += Tokenizer->Next().AsString();
	    }
	}
	    
	fVariableId = fLocalSymbolTable.NameToId(fVariableName);
    }
}

KPValue& KPOperatorVariableAccess::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    if (fVariableNameExpression != nullptr) {
	fVariableName = fVariableNameExpression->Evaluate(SymbolTable).AsString();
	fVariableId = fLocalSymbolTable.NameToId(fVariableName);
    }

    KPValue* Variable = fLocalSymbolTable.GetVariable(fVariableId);

    if (Variable == nullptr) {
	Variable = fLocalSymbolTable.CreateObject("variant");
	Variable->SetName(fVariableName);
	fLocalSymbolTable.RegisterVariable(fVariableId, Variable);
    }

    return *Variable;
}



KPOperatorListGenerate::KPOperatorListGenerate()
{
    fStartValueExpression = nullptr;
    fEndValueExpression = nullptr;
    fStepValueExpression = nullptr;
}

KPOperatorListGenerate::~KPOperatorListGenerate()
{
    delete fStartValueExpression;
    delete fEndValueExpression;
    delete fStepValueExpression;
}

KPOperator* KPOperatorListGenerate::Clone() const
{
    return new KPOperatorListGenerate();
}

string KPOperatorListGenerate::Symbol() const
{
    return string("[");
}

string KPOperatorListGenerate::Name() const
{
    return string("ListGenerate");
}

void KPOperatorListGenerate::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("[");

    fStartValueExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);

    KPToken SeparatorToken = Tokenizer->Next();
    fSeparator = SeparatorToken.AsString();
    if ((fSeparator != ",") && (fSeparator != ":")) {
	SeparatorToken.ThrowUnexpected();
    }

    fEndValueExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);

    if (Tokenizer->LookAhead().IsNot("]")) {
	Tokenizer->Next().MustBe(fSeparator);
	fStepValueExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);
	if (fSeparator == ":") {
	    swap(fStepValueExpression, fEndValueExpression);
	}
    }

    Tokenizer->Next().MustBe("]");
}

KPValue& KPOperatorListGenerate::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    KPValue StartValue = fStartValueExpression->Evaluate(SymbolTable);
    KPValue EndValue = fEndValueExpression->Evaluate(SymbolTable);

    KPValue StepValue;
    if (fStepValueExpression != nullptr) {
	StepValue = fStepValueExpression->Evaluate(SymbolTable);
    }
    else {
	if (StartValue.AsDouble() < EndValue.AsDouble()) {
	    StepValue = KPValue((long) 1);
	}
	else {
	    StepValue = KPValue((long) -1);
	}
    }

    bool IsIntegerList = (
	StartValue.IsLong() && EndValue.IsLong() && StepValue.IsLong()
    );
    
    long LStart = 0, LEnd = 0, LStep = 0;
    double DStart = 0., DEnd = 0., DStep = 0.;
    int NumberOfElements;
    if (IsIntegerList) {
	LStart = StartValue.AsLong();
	LEnd = EndValue.AsLong();
	LStep = StepValue.AsLong();
	NumberOfElements = ((LEnd - LStart) / LStep) + 1;
    }    
    else {
	DStart = StartValue.AsDouble();
	DEnd = EndValue.AsDouble();
	DStep = StepValue.AsDouble();
	NumberOfElements = (int) ((DEnd - DStart) / DStep + 1.0e-6) + 1;
    }

    if (NumberOfElements < 0) {
	NumberOfElements = 0;
    }

    if (fSeparator == ",") {
	if (IsIntegerList) {
	    if (LStart + (NumberOfElements-1) * LStep == LEnd) {
		NumberOfElements -= 1;
	    }
	}
	else {
	    double Last = DStart + (NumberOfElements-1) * DStep;
	    if (fabs((Last - DEnd) / DStep) < 1.0e-6) {
		NumberOfElements -= 1;
	    }
	}
    }

    Result = KPValue(KPListValue(NumberOfElements));
    vector<KPValue>& ValueList = Result.AsValueList();
    
    for (int i = 0; i < NumberOfElements; i++) {
	if (IsIntegerList) {
	    ValueList.push_back(KPValue(LStart + i * LStep));
	}
	else {
	    ValueList.push_back(KPValue(DStart + i * DStep));
	}
    }

    return Result;
}



KPOperator* KPOperatorSizeOf::Clone() const
{
    return new KPOperatorSizeOf();
}

string KPOperatorSizeOf::Symbol() const
{
    return string("sizeof");
}

string KPOperatorSizeOf::Name() const
{
    return string("SizeOf");
}

KPValue& KPOperatorSizeOf::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Right.IsPointer()) {
	long Length = Right.AsPointer()->ArrayLength();
	Result = KPValue((Length > 0) ? Length : (long) 1);
    }
    else if (Right.IsVoid()) {
	Result = KPValue((long) 0);
    }
    else if (Right.IsList()) {
	Result = KPValue((long) Right.AsValueList().size());
    }
    else if (Right.IsString()) {
	Result = KPValue((long) Right.AsString().size());
    }
    else {
	throw KPException() << "sizeof: array, list or string is expected";
    }

    return Result;
}



KPOperator* KPOperatorTypeOf::Clone() const
{
    return new KPOperatorTypeOf();
}

string KPOperatorTypeOf::Symbol() const
{
    return string("typeof");
}

string KPOperatorTypeOf::Name() const
{
    return string("TypeOf");
}

KPValue& KPOperatorTypeOf::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    return Result = KPValue(Right.TypeName());
}



KPOperator* KPOperatorKeys::Clone() const
{
    return new KPOperatorKeys();
}

string KPOperatorKeys::Symbol() const
{
    return string("keys");
}

string KPOperatorKeys::Name() const
{
    return string("Keys");
}

KPValue& KPOperatorKeys::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Right.IsList()) {
	throw KPException() << "keys: list value is expected";
    }
    KPListValue& ListValue = Right.AsList();

    Result = KPValue(KPListValue(ListValue.ListSize()));
    vector<KPValue>& ResultList = Result.AsValueList();

    for (unsigned Index = 0; Index < ListValue.ListSize(); Index++) {
	ResultList.push_back(KPValue(ListValue.KeyOf(Index)));
    }
    
    return Result;
}



KPOperator* KPOperatorPointerReference::Clone() const
{
    return new KPOperatorPointerReference();
}

string KPOperatorPointerReference::Symbol() const
{
    return string("*");
}

string KPOperatorPointerReference::Name() const
{
    return string("PointerReference");
}

KPValue& KPOperatorPointerReference::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Right.IsPointer() /* && ! Right.IsLong() */) {
        throw KPException() << "pointer value is expected";
    }
    if (Right.AsLong() == 0) {
        throw KPException() << "null pointer reference";
    }
    
    return *(Right.AsPointer());
} 



KPOperator* KPOperatorAddress::Clone() const
{
    return new KPOperatorAddress();
}

string KPOperatorAddress::Symbol() const
{
    return string("&");
}

string KPOperatorAddress::Name() const
{
    return string("Address");
}

KPValue& KPOperatorAddress::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Right.IsLeftValue()) {
        throw KPException() << "l-value is expected";
    }

    return Result = KPValue(&Right);
} 



KPOperator* KPOperatorIncrement::Clone() const
{
    return new KPOperatorIncrement();
}

string KPOperatorIncrement::Symbol() const
{
    return string("++");
}

string KPOperatorIncrement::Name() const
{
    return string("Increment");
}

KPValue& KPOperatorIncrement::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    KPValue Step((long) 1);
    KPOperatorAdd().Evaluate(Right, Step, SymbolTable, Result);
    Right.Assign(Result);

    return Result;
}



KPOperator* KPOperatorDecrement::Clone() const
{
    return new KPOperatorDecrement();
}

string KPOperatorDecrement::Symbol() const
{
    return string("--");
}

string KPOperatorDecrement::Name() const
{
    return string("Decrement");
}

KPValue& KPOperatorDecrement::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    KPValue Step((long) 1);
    KPOperatorSubtract().Evaluate(Right, Step, SymbolTable, Result);
    Right.Assign(Result);

    return Result;
}



KPOperator* KPOperatorPostpositionalIncrement::Clone() const
{
    return new KPOperatorPostpositionalIncrement();
}

string KPOperatorPostpositionalIncrement::Symbol() const
{
    return string("++");
}

string KPOperatorPostpositionalIncrement::Name() const
{
    return string("PostpositionalIncrement");
}

KPValue& KPOperatorPostpositionalIncrement::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    Result = Left;

    KPValue Step((long) 1);
    KPValue NewValue;
    KPOperatorAdd().Evaluate(Left, Step, SymbolTable, NewValue);
    Left.Assign(NewValue);

    return Result;
}



KPOperator* KPOperatorPostpositionalDecrement::Clone() const
{
    return new KPOperatorPostpositionalDecrement();
}

string KPOperatorPostpositionalDecrement::Symbol() const
{
    return string("--");
}

string KPOperatorPostpositionalDecrement::Name() const
{
    return string("PostpositionalDecrement");
}

KPValue& KPOperatorPostpositionalDecrement::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    Result = Left;

    KPValue Step((long) 1);
    KPValue NewValue;
    KPOperatorSubtract().Evaluate(Left, Step, SymbolTable, NewValue);
    Left.Assign(NewValue);

    return Result;
}



KPOperator* KPOperatorSignPlus::Clone() const
{
    return new KPOperatorSignPlus();
}

string KPOperatorSignPlus::Symbol() const
{
    return string("+");
}

string KPOperatorSignPlus::Name() const
{
    return string("SignPlus");
}

KPValue& KPOperatorSignPlus::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    Result = Right;
    return Result;
} 



KPOperator* KPOperatorSignMinus::Clone() const
{
    return new KPOperatorSignMinus();
}

string KPOperatorSignMinus::Symbol() const
{
    return string("-");
}

string KPOperatorSignMinus::Name() const
{
    return string("SignMinus");
}

KPValue& KPOperatorSignMinus::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    KPValue ZeroValue((long) 0);
    KPOperatorSubtract().Evaluate(ZeroValue, Right, SymbolTable, Result);

    return Result;
}



KPOperator* KPOperatorNot::Clone() const
{
    return new KPOperatorNot();
}

string KPOperatorNot::Symbol() const
{
    return string("!");
}

string KPOperatorNot::Name() const
{
    return string("Not");
}

KPValue& KPOperatorNot::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(! Right.AsBool());
} 



KPOperator* KPOperatorBitReverse::Clone() const
{
    return new KPOperatorBitReverse();
}

string KPOperatorBitReverse::Symbol() const
{
    return string("~");
}

string KPOperatorBitReverse::Name() const
{
    return string("BitReverse");
}

KPValue& KPOperatorBitReverse::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue((long) ~(Right.AsLong()));
} 



KPOperator* KPOperatorMultiple::Clone() const
{
    return new KPOperatorMultiple();
}

string KPOperatorMultiple::Symbol() const
{
    return string("*");
}

string KPOperatorMultiple::Name() const
{
    return string("Multiple");
}

KPValue& KPOperatorMultiple::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    // priority handling for intensive numerical calculation //
    if (
        (Left.IsReal() && Right.IsDouble()) ||
        (Left.IsDouble() && Right.IsReal())
    ){
        return Result = KPValue(Left.AsDouble() * Right.AsDouble());
    }

    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    if (! Left.IsNumeric() || ! Right.IsNumeric()) {
        throw KPException() << "numeric value is expected";
    }

    else if (Left.IsComplex() || Right.IsComplex()) {
        Result = KPValue(Left.AsComplex() * Right.AsComplex());
    }
    else if (Left.IsDouble() || Right.IsDouble()) {
        Result = KPValue(Left.AsDouble() * Right.AsDouble());
    }
    else {
        Result = KPValue(Left.AsLong() * Right.AsLong());
    }

    return Result;
} 



KPOperator* KPOperatorDivide::Clone() const
{
    return new KPOperatorDivide();
}

string KPOperatorDivide::Symbol() const
{
    return string("/");
}

string KPOperatorDivide::Name() const
{
    return string("Divide");
}

KPValue& KPOperatorDivide::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    // priority handling for intensive numerical calculation //
    if (
        (Left.IsReal() && Right.IsDouble()) ||
        (Left.IsDouble() && Right.IsReal())
    ){
	if (Right.AsDouble() == 0) {
	    throw KPException() << "divide by zero";
	}
        return Result = KPValue(Left.AsDouble() / Right.AsDouble());
    }

    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    if (! Left.IsNumeric() || ! Right.IsNumeric()) {
        throw KPException() << "numeric value is expected";
    }

    else if (Left.IsComplex() || Right.IsComplex()) {
	if (abs(Right.AsComplex()) == 0) {
	    throw KPException() << "divide by zero";
	}
        Result = KPValue(Left.AsComplex() / Right.AsComplex());
    }
    else if (Left.IsDouble() || Right.IsDouble()) {
	if (Right.AsDouble() == 0) {
	    throw KPException() << "divide by zero";
	}
        Result = KPValue(Left.AsDouble() / Right.AsDouble());
    }
    else {
	if (Right.AsLong() == 0) {
	    throw KPException() << "divide by zero";
	}
        Result = KPValue(Left.AsLong() / Right.AsLong());
    }

    return Result;
} 



KPOperator* KPOperatorModulo::Clone() const
{
    return new KPOperatorModulo();
}

string KPOperatorModulo::Symbol() const
{
    return string("%");
}

string KPOperatorModulo::Name() const
{
    return string("Modulo");
}

KPValue& KPOperatorModulo::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    if ((! Left.IsLong()) || (! Right.IsLong())) {
        throw KPException() << "integer value is expected";
    }

    else if (Right.AsLong() == 0) {
        throw KPException() << "divide by zero";
    }

    else {
	Result = KPValue((long) Left.AsLong() % Right.AsLong());
    }

    return Result;
} 



KPOperator* KPOperatorAdd::Clone() const
{
    return new KPOperatorAdd();
}

string KPOperatorAdd::Symbol() const
{
    return string("+");
}

string KPOperatorAdd::Name() const
{
    return string("Add");
}

KPValue& KPOperatorAdd::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    // priority handling for intensive numerical calculation //
    if (
        (Left.IsReal() && Right.IsDouble()) ||
        (Left.IsDouble() && Right.IsReal())
    ){
        return Result = KPValue(Left.AsDouble() + Right.AsDouble());
    }

    if (Left.IsVoid()) {
        return Result = KPValue(Right);
    }
    else if (Right.IsVoid()) {
        return Result = KPValue(Left);
    }

    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    if (Left.IsPointer() && Right.IsLong()) {
        Result = KPValue(Left.AsPointer() + Right.AsLong());
    }
    else if (Right.IsPointer() && Left.IsLong()) {
        Result = KPValue(Right.AsPointer() + Left.AsLong());
    }

    else if (Left.IsString() || Right.IsString()) {
        Result = KPValue(Left.AsString() + Right.AsString());
    }

    else if (! Left.IsNumeric() || ! Right.IsNumeric()) {
        throw KPException() << "numeric value is expected";
    }

    else if (Left.IsComplex() || Right.IsComplex()) {
        Result = KPValue(Left.AsComplex() + Right.AsComplex());
    }
    else if (Left.IsDouble() || Right.IsDouble()) {
        Result = KPValue(Left.AsDouble() + Right.AsDouble());
    }
    else {
        Result = KPValue(Left.AsLong() + Right.AsLong());
    }

    return Result;
} 



KPOperator* KPOperatorSubtract::Clone() const
{
    return new KPOperatorSubtract();
}

string KPOperatorSubtract::Symbol() const
{
    return string("-");
}

string KPOperatorSubtract::Name() const
{
    return string("Subtract");
}

KPValue& KPOperatorSubtract::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    // priority handling for intensive numerical calculation //
    if (
        (Left.IsReal() && Right.IsDouble()) ||
        (Left.IsDouble() && Right.IsReal())
    ){
        return Result = KPValue(Left.AsDouble() - Right.AsDouble());
    }

    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    if (Left.IsPointer() && Right.IsLong()) {
        Result = KPValue(Left.AsPointer() - Right.AsLong());
    }

    else if (Left.IsPointer() && Right.IsPointer()) {
        Result = KPValue((long) (Left.AsPointer() - Right.AsPointer()));
    }

    else if (! Left.IsNumeric() || ! Right.IsNumeric()) {
        throw KPException() << "numeric value is expected";
    }

    else if (Left.IsComplex() || Right.IsComplex()) {
        Result = KPValue(Left.AsComplex() - Right.AsComplex());
    }
    else if (Left.IsDouble() || Right.IsDouble()) {
        Result = KPValue(Left.AsDouble() - Right.AsDouble());
    }
    else {
        Result = KPValue(Left.AsLong() - Right.AsLong());
    }

    return Result;
} 



KPOperator* KPOperatorConcatenate::Clone() const
{
    return new KPOperatorConcatenate();
}

string KPOperatorConcatenate::Symbol() const
{
    return string("<+>");
}

string KPOperatorConcatenate::Name() const
{
    return string("Concatenate");
}

KPValue& KPOperatorConcatenate::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    Result = KPValue(KPListValue());
    KPListValue& ListValue = Result.AsList();

    if (Left.IsList()) {
	ListValue.AppendList(Left.AsList());
    }
    else {
	ListValue.AppendValue(Left);
    }
    
    if (Right.IsList()) {
	ListValue.AppendList(Right.AsList());
    }
    else {
	ListValue.AppendValue(Right);
    }

    return Result;
} 



KPOperator* KPOperatorLeftShift::Clone() const
{
    return new KPOperatorLeftShift();
}

string KPOperatorLeftShift::Symbol() const
{
    return string("<<");
}

string KPOperatorLeftShift::Name() const
{
    return string("LeftShift");
}

KPValue& KPOperatorLeftShift::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue((long) (Left.AsLong() << Right.AsLong()));
} 



KPOperator* KPOperatorRightShift::Clone() const
{
    return new KPOperatorRightShift();
}

string KPOperatorRightShift::Symbol() const
{
    return string(">>");
}

string KPOperatorRightShift::Name() const
{
    return string("RightShift");
}

KPValue& KPOperatorRightShift::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue((long) (Left.AsLong() >> Right.AsLong()));
} 



KPOperator* KPOperatorGreaterThan::Clone() const
{
    return new KPOperatorGreaterThan();
}

string KPOperatorGreaterThan::Symbol() const
{
    return string(">");
}

string KPOperatorGreaterThan::Name() const
{
    return string("GreaterThan");
}

KPValue& KPOperatorGreaterThan::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(Left.AsDouble() > Right.AsDouble());
} 



KPOperator* KPOperatorLessThan::Clone() const
{
    return new KPOperatorLessThan();
}

string KPOperatorLessThan::Symbol() const
{
    return string("<");
}

string KPOperatorLessThan::Name() const
{
    return string("LessThan");
}

KPValue& KPOperatorLessThan::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(Left.AsDouble() < Right.AsDouble());
} 



KPOperator* KPOperatorGreaterEqual::Clone() const
{
    return new KPOperatorGreaterEqual();
}

string KPOperatorGreaterEqual::Symbol() const
{
    return string(">=");
}

string KPOperatorGreaterEqual::Name() const
{
    return string("GreaterEqual");
}

KPValue& KPOperatorGreaterEqual::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(Left.AsDouble() >= Right.AsDouble());
} 



KPOperator* KPOperatorLessEqual::Clone() const
{
    return new KPOperatorLessEqual();
}

string KPOperatorLessEqual::Symbol() const
{
    return string("<=");
}

string KPOperatorLessEqual::Name() const
{
    return string("LessEqual");
}

KPValue& KPOperatorLessEqual::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(Left.AsDouble() <= Right.AsDouble());
} 



KPOperator* KPOperatorEqual::Clone() const
{
    return new KPOperatorEqual();
}

string KPOperatorEqual::Symbol() const
{
    return string("==");
}

string KPOperatorEqual::Name() const
{
    return string("Equal");
}

KPValue& KPOperatorEqual::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    bool Value;
    if (Left.IsPointer() || Right.IsPointer()) {
        Value = (Left.AsPointer() == Right.AsPointer());
    }

    else if (Left.IsObject() || Right.IsObject()) {
        Value = (((void*) Left.AsObject()) == ((void*) Right.AsObject()));
    }

    else if (Left.IsString() || Right.IsString()) {
        Value = (Left.AsString() == Right.AsString());
    }

    else if (Left.IsComplex() || Right.IsComplex()) {
        Value = (Left.AsComplex() == Right.AsComplex());
    }

    else if (Left.IsDouble() || Right.IsDouble()) {
        Value = (Left.AsDouble() == Right.AsDouble());
    }

    else {
        Value = (Left.AsLong() == Right.AsLong());
    }

    return Result = KPValue(Value);
}



KPOperator* KPOperatorNotEqual::Clone() const
{
    return new KPOperatorNotEqual();
}

string KPOperatorNotEqual::Symbol() const
{
    return string("!=");
}

string KPOperatorNotEqual::Name() const
{
    return string("NotEqual");
}

KPValue& KPOperatorNotEqual::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    KPOperatorEqual().Evaluate(Left, Right, SymbolTable, Result);

    return Result = KPValue(! Result.AsBool());
} 



KPOperator* KPOperatorBitAnd::Clone() const
{
    return new KPOperatorBitAnd();
}

string KPOperatorBitAnd::Symbol() const
{
    return string("&");
}

string KPOperatorBitAnd::Name() const
{
    return string("BitAnd");
}

KPValue& KPOperatorBitAnd::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue((long) (Left.AsLong() & Right.AsLong()));
} 



KPOperator* KPOperatorBitXor::Clone() const
{
    return new KPOperatorBitXor();
}

string KPOperatorBitXor::Symbol() const
{
    return string("^");
}

string KPOperatorBitXor::Name() const
{
    return string("BitXor");
}

KPValue& KPOperatorBitXor::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue((long) (Left.AsLong() ^ Right.AsLong()));
} 



KPOperator* KPOperatorBitOr::Clone() const
{
    return new KPOperatorBitOr();
}

string KPOperatorBitOr::Symbol() const
{
    return string("|");
}

string KPOperatorBitOr::Name() const
{
    return string("BitOr");
}

KPValue& KPOperatorBitOr::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue((long) (Left.AsLong() | Right.AsLong()));
} 



KPOperator* KPOperatorListAnd::Clone() const
{
    return new KPOperatorListAnd();
}

string KPOperatorListAnd::Symbol() const
{
    return string("<&>");
}

string KPOperatorListAnd::Name() const
{
    return string("ListAnd");
}

KPValue& KPOperatorListAnd::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    int LeftLength = (Left.IsList()) ? Left.AsValueList().size() : 1;
    int RightLength = (Right.IsList()) ? Right.AsValueList().size() : 1;

    Result = KPValue(KPListValue());
    vector<KPValue>& ResultList = Result.AsValueList();

    KPOperatorEqual OperatorEqual;
    KPValue CompareResult;

    for (int i = 0; i < LeftLength; i++) {
	KPValue& ThisLeft = (Left.IsList()) ? Left.AsValueList()[i] : Left;
	for (int j = 0; j < RightLength; j++) {
	    KPValue& ThisRight = (Right.IsList()) ? Right.AsValueList()[j] : Right;
	    OperatorEqual.Evaluate(ThisLeft, ThisRight, SymbolTable, CompareResult);
	    if (CompareResult.AsBool()) {
		ResultList.push_back(ThisLeft);
	    }
	}
    }

    return Result;
} 



KPOperator* KPOperatorAnd::Clone() const
{
    return new KPOperatorAnd();
}

string KPOperatorAnd::Symbol() const
{
    return string("&&");
}

string KPOperatorAnd::Name() const
{
    return string("And");
}

KPValue& KPOperatorAnd::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(Left.AsBool() && Right.AsBool());
} 



KPOperator* KPOperatorOr::Clone() const
{
    return new KPOperatorOr();
}

string KPOperatorOr::Symbol() const
{
    return string("||");
}

string KPOperatorOr::Name() const
{
    return string("Or");
}

KPValue& KPOperatorOr::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    return Result = KPValue(Left.AsBool() || Right.AsBool());
} 



KPOperator* KPOperatorAssign::Clone() const
{
    return new KPOperatorAssign();
}

string KPOperatorAssign::Symbol() const
{
    return string("=");
}

string KPOperatorAssign::Name() const
{
    return string("Assign");
}

KPValue& KPOperatorAssign::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    Left.Assign(Right);

    return Left;
}

bool KPOperatorAssign::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignSum::Clone() const
{
    return new KPOperatorAssignSum();
}

string KPOperatorAssignSum::Symbol() const
{
    return string("+=");
}

string KPOperatorAssignSum::Name() const
{
    return string("AssignSum");
}

KPValue& KPOperatorAssignSum::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorAdd().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignSum::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignDifference::Clone() const
{
    return new KPOperatorAssignDifference();
}

string KPOperatorAssignDifference::Symbol() const
{
    return string("-=");
}

string KPOperatorAssignDifference::Name() const
{
    return string("AssignDifference");
}

KPValue& KPOperatorAssignDifference::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorSubtract().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignDifference::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignProduct::Clone() const
{
    return new KPOperatorAssignProduct();
}

string KPOperatorAssignProduct::Symbol() const
{
    return string("*=");
}

string KPOperatorAssignProduct::Name() const
{
    return string("AssignProduct");
}

KPValue& KPOperatorAssignProduct::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorMultiple().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignProduct::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignQuotient::Clone() const
{
    return new KPOperatorAssignQuotient();
}

string KPOperatorAssignQuotient::Symbol() const
{
    return string("/=");
}

string KPOperatorAssignQuotient::Name() const
{
    return string("AssignQuotient");
}

KPValue& KPOperatorAssignQuotient::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorDivide().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignQuotient::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignRemainder::Clone() const
{
    return new KPOperatorAssignRemainder();
}

string KPOperatorAssignRemainder::Symbol() const
{
    return string("%=");
}

string KPOperatorAssignRemainder::Name() const
{
    return string("AssignRemainder");
}

KPValue& KPOperatorAssignRemainder::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorModulo().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignRemainder::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignBitAnd::Clone() const
{
    return new KPOperatorAssignBitAnd();
}

string KPOperatorAssignBitAnd::Symbol() const
{
    return string("&=");
}

string KPOperatorAssignBitAnd::Name() const
{
    return string("AssignBitAnd");
}

KPValue& KPOperatorAssignBitAnd::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorBitAnd().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignBitAnd::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignBitOr::Clone() const
{
    return new KPOperatorAssignBitOr();
}

string KPOperatorAssignBitOr::Symbol() const
{
    return string("|=");
}

string KPOperatorAssignBitOr::Name() const
{
    return string("AssignBitOr");
}

KPValue& KPOperatorAssignBitOr::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorBitOr().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignBitOr::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignBitXor::Clone() const
{
    return new KPOperatorAssignBitXor();
}

string KPOperatorAssignBitXor::Symbol() const
{
    return string("^=");
}

string KPOperatorAssignBitXor::Name() const
{
    return string("AssignBitXor");
}

KPValue& KPOperatorAssignBitXor::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorBitXor().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignBitXor::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignRightShift::Clone() const
{
    return new KPOperatorAssignRightShift();
}

string KPOperatorAssignRightShift::Symbol() const
{
    return string(">>=");
}

string KPOperatorAssignRightShift::Name() const
{
    return string("AssignRightShift");
}

KPValue& KPOperatorAssignRightShift::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorRightShift().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignRightShift::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignLeftShift::Clone() const
{
    return new KPOperatorAssignLeftShift();
}

string KPOperatorAssignLeftShift::Symbol() const
{
    return string("<<=");
}

string KPOperatorAssignLeftShift::Name() const
{
    return string("AssignLeftShift");
}

KPValue& KPOperatorAssignLeftShift::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    KPOperatorLeftShift().Evaluate(Left, Right, SymbolTable, Result);
    Left.Assign(Result);

    return Left;
} 

bool KPOperatorAssignLeftShift::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorAssignConcatenation::Clone() const
{
    return new KPOperatorAssignConcatenation();
}

string KPOperatorAssignConcatenation::Symbol() const
{
    return string("<+>=");
}

string KPOperatorAssignConcatenation::Name() const
{
    return string("AssignConcatenation");
}

KPValue& KPOperatorAssignConcatenation::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (! Left.IsLeftValue()) {
        throw KPException() << "l-value expected";
    }

    if (Left.IsVoid() && Left.IsVariant()) {
	Left.Assign(KPValue(KPListValue()));
    }
    else if (! Left.IsList()) {
	throw KPException() << "list value is expected";
    }

    if (Right.IsList()) {
	Left.AsList().AppendList(Right.AsList());
    }
    else {
	Left.AsList().AppendValue(Right);
    }

    return Left;
} 

bool KPOperatorAssignConcatenation::IsLeftAssociative() const
{
    return false;
}



KPOperator* KPOperatorFactorial::Clone() const
{
    return new KPOperatorFactorial();
}

string KPOperatorFactorial::Symbol() const
{
    return string("!");
}

string KPOperatorFactorial::Name() const
{
    return string("Factorial");
}

KPValue& KPOperatorFactorial::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
    if (Left.IsList() || Right.IsList()) {
	EvaluateList(Left, Right, SymbolTable, Result);
	return Result;
    }

    long Value = 1;
    for (long n = Left.AsLong(); n > 0; n--) {
	Value *= n;
    }

    return Result = KPValue(Value);
} 




KPOperatorPower::KPOperatorPower()
{
    fPowerExpression = nullptr;
}

KPOperatorPower::~KPOperatorPower()
{
    delete fPowerExpression;
}

KPOperator* KPOperatorPower::Clone() const
{
    return new KPOperatorPower();
}

string KPOperatorPower::Symbol() const
{
    return string("**");
}

string KPOperatorPower::Name() const
{
    return string("Power");
}

void KPOperatorPower::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe(Symbol());

#if 1
    // use the power operator as a postpositional unary operator,
    // to make -1**2 == -(1**2) = -1.
    fPowerExpression = ExpressionParser->ParseElement(Tokenizer, SymbolTable);
#else
    // use the power operator as a normal binary operator:
    // -1**2 becomes (-1)**2 = 1.
    fPowerExpression = 0;
#endif
}

KPValue& KPOperatorPower::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result)  
{
#if 1
    // use the power operator as a postpositional unary operator.
    KPValue* Power;
    if (Right.IsVoid()) {
	// postpositional operator node sets the right operand to VOID
	Power = &fPowerExpression->Evaluate(SymbolTable);
    }
    else {
	// EvaluateList() calls this method recursively with RightValue.
	Power = &Right;
    }
#else
    // use the power operator as a normal binary operator.
    KPValue* Power = &Right;
#endif

    if (Power->IsList() || Left.IsList()) {
	EvaluateList(Left, *Power, SymbolTable, Result);
	return Result;
    }

    if (! Left.IsNumeric() || ! Power->IsNumeric()) {
        throw KPException() << "numeric value is expected";
    }

    if (Left.IsComplex() || Right.IsComplex()) {
	Result = KPValue(pow(Left.AsComplex(), Power->AsComplex()));
    }
    else {
	Result = KPValue(pow(Left.AsDouble(), Power->AsDouble()));
    }

    return Result;
} 



KPOperatorFunctionCall::KPOperatorFunctionCall()
{
}

KPOperatorFunctionCall::~KPOperatorFunctionCall()
{
    for (unsigned i = 0; i < fMyArgumentExpressionList.size(); i++) {
	delete fMyArgumentExpressionList[i];
    }
}

KPOperator* KPOperatorFunctionCall::Clone() const
{
    return new KPOperatorFunctionCall();
}

std::string KPOperatorFunctionCall::Symbol() const
{
    return string("(");
}

std::string KPOperatorFunctionCall::Name() const
{
    return string("FunctionCall");
}

void KPOperatorFunctionCall::SetArgumentExpressionList(std::vector<KPExpression*>* ArgumentExpressionList)
{
    fArgumentExpressionList = ArgumentExpressionList;
}

void KPOperatorFunctionCall::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    fMyArgumentExpressionList = ExpressionParser->ParseExpressionList(
	Tokenizer, SymbolTable, "(", ")", ","
    );

    fArgumentExpressionList = &fMyArgumentExpressionList;
}

KPValue& KPOperatorFunctionCall::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    // this operator is supposed to be overridden by Objects //

    throw KPException() << "KPOperatorFunctionCall::Evaluate(): invalid function called";
}

KPExpression* KPOperatorFunctionCall::InternalExpression(int Index)
{
    if ((unsigned) Index < fArgumentExpressionList->size()) {
	return (*fArgumentExpressionList)[Index];
    }
    else {
	return nullptr;
    }
}



KPOperatorArrayReference::KPOperatorArrayReference()
{
    fIndexExpression = nullptr;
}


KPOperatorArrayReference::~KPOperatorArrayReference()
{
    delete fIndexExpression;
}

KPOperator* KPOperatorArrayReference::Clone() const
{
    return new KPOperatorArrayReference();
}

std::string KPOperatorArrayReference::Symbol() const
{
    return string("[");
}

std::string KPOperatorArrayReference::Name() const
{
    return string("ArrayReference");
}

void KPOperatorArrayReference::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("[");
    fIndexExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);

    try {
	Tokenizer->Next().MustBe("]");
    }
    catch (KPException &e) {
	delete fIndexExpression;
	fIndexExpression = nullptr;
	throw;
    }
}

KPValue& KPOperatorArrayReference::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    KPValue& ObjectValue = Left;
    KPValue& IndexValue = fIndexExpression->Evaluate(SymbolTable);

    try {
	if (ObjectValue.IsList() && IndexValue.IsList()) {
	    return PartialListOf(ObjectValue, IndexValue, Result);
	}
    }
    catch (KPException &e) {
	throw KPException() << "operator[]: " << e.what();
    }

    long Index;
    try {
	Index = IndexValue.AsLong();
    }
    catch (KPException &e) {
	throw KPException() << "operator[]: " << e.what();
    }

    if (ObjectValue.IsVoid() && ObjectValue.IsVariant() && ObjectValue.IsLeftValue()) {
	ObjectValue.Assign(KPValue(KPListValue()));
    }

    if (ObjectValue.IsList()) {
	return ListItemOf(ObjectValue, Index, Result);
    }

    if (ObjectValue.IsString()) {
	return StringItemOf(ObjectValue, Index, Result);
    }

    if (ObjectValue.IsPointer()) {
	if ((Index < 0) || (Index >= ObjectValue.AsPointer()->ArrayLength())) {
	    throw KPException() << "operator[]: index out of range";
	}
	return ObjectValue.AsPointer()[Index];
    }

throw KPException() << "operator[]: list, array or string is expected";
}

KPValue& KPOperatorArrayReference::ListItemOf(KPValue& ListValue, int Index, KPValue& Result) 
{
    vector<KPValue>& List = ListValue.AsValueList();
    int ListSize = List.size();

    if (Index < 0) {
	Index = ListSize - ((abs(Index) - 1) % ListSize + 1);
    }

    if (Index >= ListSize) {
	KPVariant Variant;
	List.resize(Index + 1, KPValue(Variant));
	if (ListValue.IsLeftValue()) {
	    ListValue.SetLeftValueFlag();
	}
    }
    
    return List[Index];
}

KPValue& KPOperatorArrayReference::PartialListOf(KPValue& ObjectListValue, KPValue& IndexListValue, KPValue& Result) 
{
    Result = KPValue(KPListValue());

    vector<KPValue>& ObjectList = ObjectListValue.AsValueList();
    vector<KPValue>& IndexList = IndexListValue.AsValueList();
    vector<KPValue>& ResultList = Result.AsValueList();
    
    for (unsigned i = 0; i < IndexList.size(); i++) {
	long Index = IndexList[i].AsLong();
	if ((Index >= 0) && (Index < (long) ObjectList.size())) {
	    ResultList.push_back(ObjectList[Index]);
	}
    }

    return Result;
}    

KPValue& KPOperatorArrayReference::StringItemOf(KPValue& StringValue, int Index, KPValue& Result) 
{
    string& String = StringValue.AsStringReference();
    auto StringSize = (int) String.size();

    if (Index < 0) {
	Index = StringSize - ((abs(Index) - 1) % StringSize + 1);
    }

    if (Index >= StringSize) {
	Result = KPValue(string());
    }
    else {
	Result = KPValue(String.substr(Index, 1));
    }
    
    return Result;
}

KPExpression* KPOperatorArrayReference::InternalExpression(int Index)
{
    return fIndexExpression;
}


KPOperatorTableReference::KPOperatorTableReference()
{
    fIndexExpression = nullptr;
}

KPOperatorTableReference::~KPOperatorTableReference()
{
    delete fIndexExpression;
}

KPOperator* KPOperatorTableReference::Clone() const
{
    return new KPOperatorTableReference();
}

std::string KPOperatorTableReference::Symbol() const
{
    return string("{");
}

std::string KPOperatorTableReference::Name() const
{
    return string("TableReference");
}

void KPOperatorTableReference::Parse(KPTokenizer* Tokenizer, KPExpressionParser* ExpressionParser, KPSymbolTable* SymbolTable) 
{
    Tokenizer->Next().MustBe("{");
    fIndexExpression = ExpressionParser->Parse(Tokenizer, SymbolTable);

    try {
	Tokenizer->Next().MustBe("}");
    }
    catch (KPException &e) {
	delete fIndexExpression;
	fIndexExpression = nullptr;
	throw;
    }
}

KPValue& KPOperatorTableReference::Evaluate(KPValue& Left, KPValue& Right, KPSymbolTable* SymbolTable, KPValue& Result) 
{
    KPValue& ObjectValue = Left;
    KPValue& IndexValue = fIndexExpression->Evaluate(SymbolTable);

    if (ObjectValue.IsVoid() && ObjectValue.IsVariant() && ObjectValue.IsLeftValue()) {
	ObjectValue.Assign(KPValue(KPListValue()));
    }

    if (! ObjectValue.IsList()) {
	throw KPException() << "operator{}: list value is expected";
    }

    try {
	if (IndexValue.IsList()) {
	    return PartialListOf(ObjectValue, IndexValue, Result);
	}
	else {
	    string Key = fIndexExpression->Evaluate(SymbolTable).AsString();
	    return ObjectValue.AsList().ValueOf(Key);
	}
    }
    catch (KPException &e) {
	throw KPException() << "operator{}: " << e.what();
    }
}

KPValue& KPOperatorTableReference::PartialListOf(KPValue& ObjectListValue, KPValue& IndexListValue, KPValue& Result) 
{
    Result = KPValue(KPListValue());

    vector<KPValue>& IndexList = IndexListValue.AsValueList();
    KPListValue& ObjectList = ObjectListValue.AsList();
    KPListValue& ResultList = Result.AsList();
    
    for (unsigned i = 0; i < IndexList.size(); i++) {
	string Key = IndexList[i].AsString();
	int Index = ResultList.AppendValue(ObjectList.ValueOf(Key));
	ResultList.SetKey(Index, Key);
    }

    return Result;
}    

KPExpression* KPOperatorTableReference::InternalExpression(int Index)
{
    return fIndexExpression;
}
