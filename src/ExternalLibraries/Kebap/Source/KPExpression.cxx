// KPExpression.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "KPObject.h"
#include "KPValue.h"
#include "KPOperator.h"
#include "KPSymbolTable.h"
#include "KPExpression.h"
#include "KPFunction.h"
#include "KPBuiltinFunction.h"

using namespace std;
using namespace kebap;


KPExpressionParser::KPExpressionParser(KPOperatorTable* OperatorTable)
{
    fOperatorTable = OperatorTable;
}

KPExpressionParser::~KPExpressionParser()
{
}

KPExpression* KPExpressionParser::Parse(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable, int PriorityRank) 
{
    KPExpression* ThisNode = nullptr;

    if (PriorityRank < 0) {
        /* default priority: start from the lowest priority (highest level) */
        PriorityRank = fOperatorTable->LowestPriorityRank();
    }
    if (PriorityRank < fOperatorTable->HighestPriorityRank()) {
        return ParsePrepositional(Tokenizer, SymbolTable);
    }
    
    ThisNode = Parse(Tokenizer, SymbolTable, PriorityRank - 1);

    while (true) {
	string OperatorSymbol = Tokenizer->LookAhead().AsString();
	if (fOperatorTable->PriorityRankOf(OperatorSymbol) != PriorityRank) {
	    break;
	}

	KPExpression* LeftNode = ThisNode;
	KPExpression* RightNode = nullptr;
	KPOperator* Operator = fOperatorTable->CreateOperator(OperatorSymbol);

	long LineNumber = Tokenizer->LineNumber();
	try {
	    Operator->Parse(Tokenizer, this, SymbolTable);
	}
	catch (KPException &e) {
	    delete Operator;
	    throw;
	}
    
	int RightNodePriorityRank = PriorityRank - 1;
	if (! Operator->IsLeftAssociative()) {
	    RightNodePriorityRank = PriorityRank;
	}

	try {
	    RightNode = Parse(Tokenizer, SymbolTable, RightNodePriorityRank);
	}
	catch (KPException &e) {
	    delete Operator;
	    delete LeftNode;
	    throw;
	}
	ThisNode = new KPOperatorNode(Operator, LeftNode, RightNode);
	ThisNode->SetLineNumber(LineNumber);

	if (! Operator->IsLeftAssociative()) {
	    break;
	}
    }
    
    return ThisNode;
}

KPExpression* KPExpressionParser::ParsePrepositional(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    KPExpression* ThisNode = nullptr;
    long LineNumber = Tokenizer->LineNumber();

    KPToken Token = Tokenizer->LookAhead();
    string OperatorSymbol = Token.AsString();
    KPOperator* PrepositionalOperator = fOperatorTable->CreatePrepositionalOperator(OperatorSymbol);
    
    try {
	// normal prepositional operator //
	if (PrepositionalOperator != nullptr) {
	    PrepositionalOperator->Parse(Tokenizer, this, SymbolTable);
	    KPExpression* RightNode = ParsePrepositional(Tokenizer, SymbolTable);
	    ThisNode = new KPOperatorNode(PrepositionalOperator, nullptr, RightNode);
	    ThisNode->SetLineNumber(LineNumber);
	}
	
	// typecast //
	// (int), (Scanner), (int*), (int***) etc.
	// the third condition is to make distinction from (Scanner(line).get())
	else if (
	    Tokenizer->LookAhead(1).Is("(") &&
	    SymbolTable->IsTypeName(Tokenizer->LookAhead(2).AsString()) &&
	    Tokenizer->LookAhead(3).IsNot("(")
	){
	    ThisNode = ParseTypeCast(Tokenizer, SymbolTable);
	}

	// go lower: element //
	else {
	    ThisNode = ParseElement(Tokenizer, SymbolTable);
	}
    }
    catch (KPException &e) {
	delete PrepositionalOperator;
	throw;
    }

    return ThisNode;
}

KPExpression* KPExpressionParser::ParseElement(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    KPExpression* ThisNode = nullptr;
    KPToken Token = Tokenizer->Next();
    long LineNumber = Tokenizer->LineNumber();
    
    string OperatorSymbol = Token.AsString();
    KPOperator* ElementaryOperator = fOperatorTable->CreateElementaryOperator(OperatorSymbol);

    if (ElementaryOperator != nullptr) {
	Tokenizer->Unget(Token);
	ElementaryOperator->Parse(Tokenizer, this, SymbolTable);
	ThisNode = new KPOperatorNode(ElementaryOperator, nullptr, nullptr);
	ThisNode->SetLineNumber(LineNumber);
    }

    else if (Token.Is("(")) {
	ThisNode = Parse(Tokenizer, SymbolTable);
	Tokenizer->Next().MustBe(")");
    }

    // primitive literals //
    else if (Token.IsBool()) {
        ThisNode = new KPLiteralNode(KPValue(Token.AsBool()));
	ThisNode->SetLineNumber(LineNumber);
    }
    else if (Token.IsInteger()) {
        ThisNode = new KPLiteralNode(KPValue(Token.AsLong()));
	ThisNode->SetLineNumber(LineNumber);
    }
    else if (Token.IsFloating()) {
        ThisNode = new KPLiteralNode(KPValue(Token.AsDouble()));
	ThisNode->SetLineNumber(LineNumber);
    }
    else if (Token.IsComplex()) {
        ThisNode = new KPLiteralNode(KPValue(Token.AsComplex()));
	ThisNode->SetLineNumber(LineNumber);
    }
    else if (Token.IsQuote()) {
        Token.RemoveQuotation();
        ThisNode = new KPLiteralNode(KPValue(Token.AsString()));
	ThisNode->SetLineNumber(LineNumber);
    }

    // temporary object creation //
    else if (SymbolTable->IsTypeName(Token.AsString())) {
	Tokenizer->Unget(Token);
	ThisNode = ParseTemporaryObjectCreation(Tokenizer, SymbolTable);
    }

    // identifier: function call or variable //
    else if (Token.IsIdentifier()) {
        if (Tokenizer->LookAhead().Is("(")) {
            Tokenizer->Unget(Token);
            ThisNode = ParseFunctionCall(Tokenizer, SymbolTable);
        }
        else {
            long VariableId = SymbolTable->NameToId(Token.AsString());
            ThisNode = new KPVariableNode(VariableId);
	    ThisNode->SetLineNumber(LineNumber);
        }
    }

    // list literal //
    else if (Token.Is("{")) {
	Tokenizer->Unget(Token);
        ThisNode = ParseListExpression(Tokenizer, SymbolTable);
	ThisNode->SetLineNumber(LineNumber);
    }
    
    else {
	Token.ThrowUnexpected();
    }

    return ParsePostpositional(ThisNode, Tokenizer, SymbolTable);
}

KPExpression* KPExpressionParser::ParseTypeCast(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    long LineNumber = Tokenizer->LineNumber();

    Tokenizer->Next().MustBe("(");
    string TypeName = Tokenizer->Next().AsString();
    while (Tokenizer->LookAhead().Is("*")) {
	TypeName = "pointer";
	Tokenizer->Next();
    }
    Tokenizer->Next().MustBe(")");

    KPExpression* RightNode = ParsePrepositional(Tokenizer, SymbolTable);

    KPExpression* ThisNode = new KPTypeCastNode(TypeName, RightNode);
    ThisNode->SetLineNumber(LineNumber);

    return ThisNode;
}

KPExpression* KPExpressionParser::ParseTemporaryObjectCreation(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    long LineNumber = Tokenizer->LineNumber();

    KPToken Token = Tokenizer->Next();
    string TypeName = Token.AsString();
    
    if (! SymbolTable->IsTypeName(Token.AsString())) {
	Token.ThrowUnexpected("type name");
    }
        
    vector<KPExpression*> ArgumentExpressionList;
    ArgumentExpressionList = ParseExpressionList(
	Tokenizer, SymbolTable, "(", ")", ","
    );

    KPExpression* ThisNode = new KPTemporaryObjectCreationNode(
	TypeName, ArgumentExpressionList
    );
    ThisNode->SetLineNumber(LineNumber);

    return ThisNode;
}

KPFunctionCallNode* KPExpressionParser::ParseFunctionCall(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    long LineNumber = Tokenizer->LineNumber();

    KPToken Token = Tokenizer->Next();
    if (! Token.IsIdentifier()) {
	Token.ThrowUnexpected("function name");
    }
    long FunctionId = SymbolTable->NameToId(Token.AsString());
        
    vector<KPExpression*> ArgumentExpressionList;
    ArgumentExpressionList = ParseExpressionList(
	Tokenizer, SymbolTable, "(", ")", ","
    );

    auto* ThisNode = new KPFunctionCallNode(
	FunctionId, ArgumentExpressionList
    );
    ThisNode->SetLineNumber(LineNumber);

    return ThisNode;
}

KPExpression* KPExpressionParser::ParseListExpression(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    vector<KPExpression*> KeyExpressionList;
    vector<KPExpression*> ValueExpressionList;

    KPToken Token;
    Tokenizer->Next().MustBe("{");
    
    if ((Token = Tokenizer->Next()).IsNot("}")) {
        Tokenizer->Unget(Token);

	KPExpression* Expression;
        while (true) {
	    Expression = Parse(Tokenizer, SymbolTable);

            Token = Tokenizer->Next();
	    if (Token.Is("=>")) {
		KeyExpressionList.push_back(Expression);
		Expression = Parse(Tokenizer, SymbolTable);
		Token = Tokenizer->Next();
	    }
	    else {
		KeyExpressionList.push_back((KPExpression*) nullptr);
	    }
	    
	    ValueExpressionList.push_back(Expression);

            if (Token.Is("}")) {
                break;
            }
            else {
                Token.MustBe(",");
		if (Tokenizer->LookAhead().Is("}")) {
		    Tokenizer->Next();
		    break;
		}
            }
        }
    }

    return new KPListNode(KeyExpressionList, ValueExpressionList);
}

KPExpression* KPExpressionParser::ParsePostpositional(KPExpression* Expression, KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    KPExpression* ThisNode = Expression;
    KPOperator* Operator;

    KPToken Token = Tokenizer->LookAhead();
    Operator = fOperatorTable->CreatePostpositionalOperator(Token.AsString());

    if (Operator != nullptr) {
	Operator->Parse(Tokenizer, this, SymbolTable);
	ThisNode = new KPOperatorNode(Operator, ThisNode, nullptr);
	ThisNode->SetLineNumber(Tokenizer->LineNumber());
    }
    else if (Token.Is(".") || Token.Is("->")) {
        ThisNode = ParseMethodInvocation(ThisNode, Tokenizer, SymbolTable);
    }
    else {
	return ThisNode;
    }

    return ParsePostpositional(ThisNode, Tokenizer, SymbolTable);
}

KPExpression* KPExpressionParser::ParseMethodInvocation(KPExpression* ObjectNode, KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable) 
{
    long LineNumber = Tokenizer->LineNumber();

    if (Tokenizer->Next().Is("->")) {
        KPOperator* Operator = new KPOperatorPointerReference();
        ObjectNode = new KPOperatorNode(Operator, nullptr, ObjectNode);
	ObjectNode->SetLineNumber(LineNumber);
    }    

    KPToken Token = Tokenizer->Next();
    if (! Token.IsIdentifier()) {
	Token.ThrowUnexpected("method or property name");
    }
    string MethodName = Token.AsString();
    long MethodId = SymbolTable->NameToId(MethodName);

    KPExpression* ThisNode = nullptr;
    if (Tokenizer->LookAhead().IsNot("(")) {
	ThisNode = new KPPropertyAccessNode(ObjectNode, MethodName);
    }
    else {
	vector<KPExpression*> ArgumentExpressionList;
	ArgumentExpressionList = ParseExpressionList(
	    Tokenizer, SymbolTable, "(", ")", ","
	);
	
	ThisNode = new KPMethodInvocationNode(
	    ObjectNode, MethodId, ArgumentExpressionList
	);
    }

    ThisNode->SetLineNumber(LineNumber);
    
    return ThisNode;
}

vector<KPExpression*> KPExpressionParser::ParseExpressionList(KPTokenizer* Tokenizer, KPSymbolTable* SymbolTable, const string& StartToken, const string& StopToken, const string& SeparatorToken) 
{
    vector<KPExpression*> ArgumentExpressionList;

    KPToken Token;
    Tokenizer->Next().MustBe(StartToken);
    
    if ((Token = Tokenizer->Next()).IsNot(StopToken)) {
        Tokenizer->Unget(Token);

        while (true) {
            KPExpression* Argument = Parse(Tokenizer, SymbolTable);
            ArgumentExpressionList.push_back(Argument);

            Token = Tokenizer->Next();
            if (Token.Is(StopToken)) {
                break;
            }
            else {
                Token.MustBe(SeparatorToken);
            }
        }
    }

    return ArgumentExpressionList;
}



KPExpression::KPExpression()
{
    fLeftNode = nullptr;
    fRightNode = nullptr;
}
    
KPExpression::~KPExpression()
{
    delete fLeftNode;
    delete fRightNode;
}
    
void KPExpression::SetLineNumber(long LineNumber)
{
    fLineNumber = LineNumber;
}

string KPExpression::Position() const
{
    if (fLineNumber == 0) {
	return string("");
    }

    ostringstream Stream;
    Stream << "line " << fLineNumber << ": ";

    return Stream.str();
}

void KPExpression::Dump(ostream &os, int IndentLevel) const
{
    if (fLeftNode != nullptr) {
        fLeftNode->Dump(os, IndentLevel + 1);
    }

    for (int i = 0; i < IndentLevel; i++) {
        os << "  ";
    }
    DumpThis(os);
    os << endl;

    if (fRightNode != nullptr) {
        fRightNode->Dump(os, IndentLevel + 1);
    }
}



KPOperatorNode::KPOperatorNode(KPOperator* Operator, KPExpression* LeftNode, KPExpression* RightNode)
{
    fOperator = Operator;
    fLeftNode = LeftNode ? LeftNode : new KPLiteralNode(KPValue());
    fRightNode = RightNode ? RightNode : new KPLiteralNode(KPValue());
}   

KPOperatorNode::~KPOperatorNode()
{
    delete fOperator;
}

KPValue& KPOperatorNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    KPValue& LeftValue = fLeftNode->Evaluate(SymbolTable);
    KPValue& RightValue = fRightNode->Evaluate(SymbolTable);
    
    try {
	//...
	if (LeftValue.IsVariant() && (fOperator->Name() == "Assign")) {
	    return fOperator->Evaluate(
		LeftValue, RightValue, SymbolTable, fValue
	    );
	}

	if (LeftValue.IsObject()) {
	    return LeftValue.AsObject()->EvaluateOperator(
		fOperator, LeftValue, RightValue, SymbolTable, fValue
	    );
	}
	else if (RightValue.IsObject()) {
	    return RightValue.AsObject()->EvaluateOperator(
		fOperator, LeftValue, RightValue, SymbolTable, fValue
	    );
	}
	else {
	    return fOperator->Evaluate(
		LeftValue, RightValue, SymbolTable, fValue
	    );
	}
    }
    catch (KPException &e) {
	throw KPException() << Position() << e.what();
    }
}

void KPOperatorNode::DumpThis(ostream &os) const
{
    os << fOperator->Symbol();
}



KPTypeCastNode::KPTypeCastNode(const string& TypeName, KPExpression* RightNode)
{
    fTypeName = TypeName;

    fLeftNode = nullptr;
    fRightNode = RightNode;
}   

KPTypeCastNode::~KPTypeCastNode()
{
}

KPValue& KPTypeCastNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    KPValue& Right = fRightNode->Evaluate(SymbolTable);

    if (Right.IsList() && (fTypeName != "list")) {
	KPListValue& RightList = Right.AsList();
	int Size = RightList.ListSize();
	KPListValue ResultList(Size);

	for (int i = 0; i < Size; i++) {
	    Convert(RightList[i], ResultList[i]);
	}
	fValue = KPValue(ResultList);
    }
    else {
	Convert(Right, fValue);
    }
    
    return fValue;
}

void KPTypeCastNode::Convert(const KPValue& From, KPValue& To) 
{
    try {
	if (fTypeName == "void") {
	    To = KPValue();
	}
	else if (fTypeName == "bool") {
	    To = KPValue(From.AsBool());
	}
	else if ((fTypeName == "int") || (fTypeName == "long")) {
	    To = KPValue(From.AsLong());
	}
	else if ((fTypeName == "float") || (fTypeName == "double")) {
	    To = KPValue(From.AsDouble());
	}
	else if (fTypeName == "complex") {
	    To = KPValue(From.AsComplex());
	}
	else if (fTypeName == "string") {
	    To = KPValue(From.AsString());
	}
	else if ((fTypeName == "pointer") || (fTypeName == "*")) {
	    To = KPValue(From.AsPointer());
	}
	else if (fTypeName == "list") {
	    if (! From.IsList()) {
		To = KPValue(KPListValue());
		To.AsValueList().push_back(From);
	    }
	    else {
		To = From;
	    }
	}
	else{ 
	    throw KPException() << "invalid type name";
	}
    }
    catch (KPException &e) {
	throw KPException() <<
	    "invalid type cast: " << fTypeName + ": " << e.what();
    }
}

void KPTypeCastNode::DumpThis(ostream &os) const
{
    os << "(" << fTypeName << ")";
}



KPLiteralNode::KPLiteralNode(const KPValue& Value)
: fValue(Value)
{
}

KPLiteralNode::~KPLiteralNode()
{
}

KPValue& KPLiteralNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    return fValue;
}

void KPLiteralNode::DumpThis(ostream &os) const
{
    os << fValue.AsString();
}



KPVariableNode::KPVariableNode(long VariableId)
{
    fVariableId = VariableId;
}

KPVariableNode::~KPVariableNode()
{
}

KPValue& KPVariableNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    KPValue* Variable = SymbolTable->GetVariable(fVariableId);
    if (Variable == nullptr) {
        string Name = SymbolTable->IdToName(fVariableId);
        throw KPException() << Position() << "undefined variable: " << Name;
    }

    return *Variable;
}

void KPVariableNode::DumpThis(ostream &os) const
{
    os << KPNameTable::GetInstance()->IdToName(fVariableId);
    os << "{" << fVariableId << "}";
}



KPListNode::KPListNode(const vector<KPExpression*>& KeyExpressionList, const vector<KPExpression*>& ValueExpressionList)
{
    fKeyExpressionList = KeyExpressionList;
    fValueExpressionList = ValueExpressionList;
}

KPListNode::~KPListNode()
{
    for (unsigned i = 0; i < fKeyExpressionList.size(); i++) {
	delete fKeyExpressionList[i];
	delete fValueExpressionList[i];
    }
}

KPValue& KPListNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    fValue = KPValue(KPListValue());
    KPListValue& ListValue = fValue.AsList();
    vector<KPValue>& ValueList = fValue.AsValueList();

    for (unsigned i = 0; i < fValueExpressionList.size(); i++) {
	KPValue& Value = fValueExpressionList[i]->Evaluate(SymbolTable);
	if (fKeyExpressionList[i] != nullptr) {
	    KPValue& Key = fKeyExpressionList[i]->Evaluate(SymbolTable);
	    ListValue.ValueOf(Key.AsString()) = Value;
	}
	else {
	    ValueList.push_back(Value);
	}
    }
    
    return fValue;
}

void KPListNode::DumpThis(ostream &os) const
{
    os << "list[" << fValueExpressionList.size() << "]";
}



KPFunctionCallNode::KPFunctionCallNode(long FunctionId, vector<KPExpression*>& ArgumentExpressionList)
{
    fFunctionId = FunctionId;
    fArgumentExpressionList = ArgumentExpressionList;
    
    fBuiltinFunctionTable = nullptr;
}

KPFunctionCallNode::~KPFunctionCallNode()
{
    for (unsigned i = 0; i < fArgumentExpressionList.size(); i++) {
	delete fArgumentExpressionList[i];
    }
}

KPValue& KPFunctionCallNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    KPValue* Variable = SymbolTable->GetVariable(fFunctionId);
    if ((Variable != nullptr) && Variable->IsObject()) {
	return EvaluateObjectFunction(Variable, SymbolTable);
    }

    // EvaluateArgument() and EvaluateFunction() are separated
    // for delayed execution

    EvaluateArguments(SymbolTable);
    return EvaluateFunction(SymbolTable);
}

void KPFunctionCallNode::EvaluateArguments(KPSymbolTable* SymbolTable) 
{
    fArgumentList.erase(
        fArgumentList.begin(), fArgumentList.end()
    );
    
    vector<KPExpression*>::iterator Expression;
    for (
        Expression = fArgumentExpressionList.begin();
        Expression != fArgumentExpressionList.end();
        Expression++
    ){
        KPValue& Value = (*Expression)->Evaluate(SymbolTable);
        fArgumentList.push_back(&Value);
    }
}

KPValue& KPFunctionCallNode::EvaluateFunction(KPSymbolTable* SymbolTable) 
{
    KPFunction* Function = SymbolTable->GetFunction(fFunctionId);
    if (Function != nullptr) {
	long ImportDepth;
	KPSymbolTable* GlobalSymbolTable;
	GlobalSymbolTable = new KPSymbolTable(SymbolTable, ImportDepth = 1);
	try {
	    fValue = Function->Execute(fArgumentList, GlobalSymbolTable);
	}
	catch (KPException &e) {
	    delete GlobalSymbolTable;    
	    throw;
	}
	delete GlobalSymbolTable;    

	return fValue;
    }

    if (fBuiltinFunctionTable == nullptr) {
	fBuiltinFunctionTable = SymbolTable->BuiltinFunctionTable();
	if (fBuiltinFunctionTable != nullptr) {
	    fBuiltinFunctionTable->RegisterFunctionId(
		SymbolTable->IdToName(fFunctionId), fFunctionId
	    );
	}
    }

    int Result = 0;
    if (fBuiltinFunctionTable != nullptr) {
	try {
	    Result = fBuiltinFunctionTable->Execute(
		fFunctionId, fArgumentList, fValue
	    );
	}
	catch (KPException &e) {
	    throw KPException() << Position() << e.what();
	}
    }
    
    if (Result == 0) {
	string FunctionName = SymbolTable->IdToName(fFunctionId);
	throw KPException() <<
	    Position() << "unknown function: " << FunctionName << "()";
    }

    return fValue;
}

KPValue& KPFunctionCallNode::EvaluateObjectFunction(KPValue* Variable, KPSymbolTable* SymbolTable) 
{
    KPOperatorFunctionCall FunctionCallOperator;    
    KPValue LeftValue, RightValue;

    FunctionCallOperator.SetArgumentExpressionList(&fArgumentExpressionList);

    return Variable->AsObject()->EvaluateOperator(
	&FunctionCallOperator, LeftValue, RightValue, SymbolTable, fValue
    );
}

void KPFunctionCallNode::DumpThis(ostream &os) const
{
    string FunctionName = KPNameTable::GetInstance()->IdToName(fFunctionId);
    os << FunctionName << "(...)";
}



KPMethodInvocationNode::KPMethodInvocationNode(KPExpression* ObjectExpression, long FunctionId, vector<KPExpression*>& ArgumentExpressionList)
: KPFunctionCallNode(FunctionId, ArgumentExpressionList)
{
    fObjectExpression = ObjectExpression;

    fObject = nullptr;
    fMethodId = -1;
}

KPMethodInvocationNode::~KPMethodInvocationNode()
{
    delete fObjectExpression;
}

KPValue& KPMethodInvocationNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    KPValue& ObjectValue = fObjectExpression->Evaluate(SymbolTable);
    KPObjectPrototype* Object;
    try {
	Object = ObjectValue.AsObject();
    }
    catch (KPException &e) {
	throw KPException() << Position() << e.what();
    }

    if (Object != fObject) {
	fObject = Object;
	fMethodName = SymbolTable->IdToName(fFunctionId);
	fMethodId = Object->MethodIdOf(fMethodName);
    }
    
    int Result = 0;
    try {
	EvaluateArguments(SymbolTable);
	if (fMethodId > 0) {
	    Result = Object->InvokeMethod(
		fMethodId, fArgumentList, fValue
	    );
	}
	else {
	    Result = Object->InvokeMethodByName(
		fMethodName, fArgumentList, fValue
	    );
	}
    }
    catch (KPException &e) {
	throw KPException() << Position() << e.what();
    }
    if (Result == 0) {
	throw KPException() << "unknown method: " << fMethodName;
    }

    return fValue;
}



KPPropertyAccessNode::KPPropertyAccessNode(KPExpression* ObjectExpression, const string& PropertyName)
{
    fPropertyName = PropertyName;
    fObjectExpression = ObjectExpression;

    fPropertyId = -1;
    fObject = nullptr;
}

KPPropertyAccessNode::~KPPropertyAccessNode()
{
    delete fObjectExpression;
}

KPValue& KPPropertyAccessNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    KPValue& ObjectValue = fObjectExpression->Evaluate(SymbolTable);
    KPObjectPrototype* Object;
    try {
	Object = ObjectValue.AsObject();
    }
    catch (KPException &e) {
	throw KPException() << Position() << e.what();
    }

    if (Object != fObject) {
	fObject = Object;
	fPropertyId = Object->PropertyIdOf(fPropertyName);
    }

    int Result;
    try {
	if (fPropertyId > 0) {
	    Result = Object->GetProperty(fPropertyId, fValue);
	}
	else {
	    Result = Object->GetPropertyByName(fPropertyName, fValue);
	}
    }
    catch (KPException &e) {
	throw KPException() << Position() << e.what();
    }
    if (Result == 0) {
	throw KPException() << "unknown property: " << fPropertyName;
    }

    return fValue;
}

void KPPropertyAccessNode::DumpThis(ostream &os) const
{
    os << "." << fPropertyName;
}



KPTemporaryObjectCreationNode::KPTemporaryObjectCreationNode(const string& TypeName, vector<KPExpression*>& ArgumentExpressionList)
: KPFunctionCallNode(0, ArgumentExpressionList)
{
    fTypeName = TypeName;
}

KPTemporaryObjectCreationNode::~KPTemporaryObjectCreationNode()
{
}

KPValue& KPTemporaryObjectCreationNode::Evaluate(KPSymbolTable* SymbolTable) 
{
    EvaluateArguments(SymbolTable);
    
    KPValue* Variable = SymbolTable->CreateObject(fTypeName);
    if (Variable == nullptr) {
        throw KPException() <<
	    Position() << "unknown variable type: " << fTypeName;
    }
    long VariableId = -1;  // unused value for temporary objects
    SymbolTable->RegisterVariable(VariableId, Variable);

    if (Variable->IsObject()) {
	Variable->AsObject()->Construct(fTypeName, fArgumentList);
    }
    else {
        if (fArgumentList.size() > 1) {
	    throw KPException() << "too many initial-value arguments";
	}
	else if (! fArgumentList.empty()) {
	    Variable->Assign(*fArgumentList[0]);
	}
    }

    return *Variable;
}
