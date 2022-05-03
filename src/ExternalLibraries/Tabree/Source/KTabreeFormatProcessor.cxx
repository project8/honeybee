// KTabreeFormatProcessor.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdio>
#include <cctype>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "KTabree.h"
#include "KTreeBuilder.h"
#include "KTabreeFormat.h"
#include "KTabreeFormatProcessor.h"


using namespace std;
using namespace tabree;

static const char CR = '\x0d';
static const char LF = '\x0a';


KTabreeFormatLineExtractor::KTabreeFormatLineExtractor(std::istream& Input, char LineTerminator)
: fInput(Input)
{
    fLineTerminator = LineTerminator;

    fLastChar = '\0';
    fCRCount = 0;
}

KTabreeFormatLineExtractor::~KTabreeFormatLineExtractor()
{
}

bool KTabreeFormatLineExtractor::GetNext(std::string& Line)
{
    if (fLineTerminator != '\0') {
        if (! getline(fInput, Line, fLineTerminator)) {
            return false;
        }
        if ((! Line.empty()) && (*Line.rbegin() == CR)) {
            Line.erase(Line.end()-1);
        }
        return true;
    }

    bool Result = false;
    Line.clear();

    char Char = fLastChar;
    while (fLastChar = Char, fInput.get(Char)) {
        if ((fLastChar == CR) && (Char == LF)) {
            fLineTerminator = LF;   // DOS (CR+LF)
            continue;
        }
        Result = true;

        if (Char == CR) {
            fCRCount++;
            if ((fCRCount > 1) && (fLineTerminator == '\0')) {
                fLineTerminator = CR;   // Mac (CR)
            }                
            break;
        }
        if (Char == LF) {
            if (fLineTerminator == '\0') {
                fLineTerminator = LF;   // Unix (LF)
            }
            break;
        }

        Line += Char;
    }
    fLastChar = Char;

    return Result;
}



KTabreeFormatTokenizer::KTabreeFormatTokenizer()
{
    fDelimiter = '\0';
    fQuote = '"';
}

KTabreeFormatTokenizer::~KTabreeFormatTokenizer()
{
}

char KTabreeFormatTokenizer::SetDelimiter(char Delimiter)
{
    char OldDelimiter = fDelimiter;
    fDelimiter = Delimiter;

    return OldDelimiter;
}

char KTabreeFormatTokenizer::SetQuote(char Quote)
{
    char OldQuote = fQuote;
    fQuote = Quote;

    return OldQuote;
}

void KTabreeFormatTokenizer::Tokenize(const std::string& Line, std::vector<std::string>& ElementList) const 
{
    string Element, TrailingWhiteSpace;
    string::const_iterator Char;
    bool IsStarted = false, IsInQuote = false, IsEscaped = false;
    for (Char = Line.begin(); Char != Line.end(); Char++) {
	if ((! IsStarted) && isspace(*Char)) {
	    continue;
	}
	IsStarted = true;
	if (IsEscaped) {
	    IsEscaped = false;
	}
	else if (*Char == '\\') {
	    IsEscaped = true;
	    continue;
	}
	else if (*Char == fQuote) {
	    IsInQuote = ! IsInQuote;
	    continue;
	}
	else if (! IsInQuote) {
	    if (
		((fDelimiter != '\0') && (*Char == fDelimiter)) ||
		((fDelimiter == '\0') && isspace(*Char))
	    ){
		ElementList.push_back(Element);
		Element = "";
                TrailingWhiteSpace = "";
		IsStarted = false;
		continue;
	    }
	}
        if ((! IsInQuote) && isspace(*Char)) {
            TrailingWhiteSpace += *Char;
        }
        else {
            if (! TrailingWhiteSpace.empty()) {
                Element += TrailingWhiteSpace;
                TrailingWhiteSpace = "";
            }
            Element += *Char;
        }
    }
    if (IsInQuote) {
	throw KException() << "quote mismatch";
    }
    if (IsStarted) {
	ElementList.push_back(Element);
    }
}



KTabreeFormatColumnParser::KTabreeFormatColumnParser()
{
}

KTabreeFormatColumnParser::~KTabreeFormatColumnParser()
{
}

void KTabreeFormatColumnParser::SetColumnTypeList(const std::vector<std::string>& ColumnTypeList)
{
    for (unsigned i = 0; i < ColumnTypeList.size(); i++) {
	const string& Type = ColumnTypeList[i];
	if ((Type == "int") || (Type == "long")) {
	    fColumnTypeList.push_back(ColumnType_Int);
	}
	else if ((Type == "float") || (Type == "double")) {
	    fColumnTypeList.push_back(ColumnType_Float);
	}
	else {
	    fColumnTypeList.push_back(ColumnType_String);
	}
    }
}

unsigned KTabreeFormatColumnParser::ProcessLine(KTable::TRow Row, std::string& Line, unsigned ColumnOffset) 
{
    vector<string> ElementList;
    Tokenize(Line, ElementList);

    for (unsigned i = 0; i < ElementList.size(); i++) {
        if (i < fColumnTypeList.size()) {
            if (fColumnTypeList[i] == ColumnType_Int) {
                Row[ColumnOffset + i] = (long) KVariant(ElementList[i]);
            }
            else if (fColumnTypeList[i] == ColumnType_Float) {
                Row[ColumnOffset + i] = (double) KVariant(ElementList[i]);
            }
            else {
                Row[ColumnOffset + i] = ElementList[i];
            }
        }
        else if (ElementList[i].empty()) {
            Row[ColumnOffset + i] = KVariant();
        }
        else {
	    try {
                Row[ColumnOffset + i] = (double) KVariant(ElementList[i]);
	    }
	    catch (KException &e) {
		Row[ColumnOffset + i] = ElementList[i];
	    }
        }
    }

    // fill the empty fields with 'void' //
    for (unsigned i = ElementList.size(); i < fColumnTypeList.size(); i++) {
        Row[ColumnOffset + i] = KVariant();
    }

    return ElementList.size();
}    



KTabreeFormatHeaderProcessor::KTabreeFormatHeaderProcessor()
{
    fTreeBuilder = nullptr;

    fIsCsvHeaderEnabled = false;

    fQuote = '"';
    fCommentHeader = '#';
    fTreeIndent = '\0';
}

KTabreeFormatHeaderProcessor::~KTabreeFormatHeaderProcessor()
{
    delete fTreeBuilder;
}

void KTabreeFormatHeaderProcessor::EnableCsvHeader()
{
    fIsCsvHeaderEnabled = true;
}

void KTabreeFormatHeaderProcessor::SetCommentHeader(char CommentHeader)
{
    fCommentHeader = CommentHeader;
}

void KTabreeFormatHeaderProcessor::SetTreeIndent(char TreeIndent)
{
    if (TreeIndent == ' ') {
        fTreeIndent = '\0';
    }
    else {
        fTreeIndent = TreeIndent;
    }
}

void KTabreeFormatHeaderProcessor::SetStorage(KTree* Tree)
{
    delete fTreeBuilder;
    fTreeBuilder = new KTreeBuilder(Tree);
}

bool KTabreeFormatHeaderProcessor::ProcessFirstLine(const std::string& Line)
{
    if (fIsCsvHeaderEnabled) {
        return ProcessCsvHeader(Line);
    }
    else {
        return ProcessRootTreeDescriptor(Line);
    }
}

bool KTabreeFormatHeaderProcessor::ProcessLine(const string& Line)
{
#if 0
// syntax //
Line:
    Header Name Separater VALUE
    Header Depth Name Separater VALUE
    Header Depth Separator VALUE
Header:
    COMMENT_HEADER
    Header Header
Depth:
    TREE_INDENT
    DepthQualifier TREE_INDENT
    DepthQualifier ' '
Name:
    NAME
    Name ' '
Separater:
    ':'
    Separater ' '
#endif

    int Depth = 0;
    string Name;
    string Value;
    
    enum TState { 
	State_Initial, 
	State_Header, 
	State_Depth, 
	State_Name, 
	State_PreSeparatorSpace, State_Separator, State_PostSeparatorSpace,
	State_Value
    };
    enum TContext {
        Context_Single,
        Context_Array,
        Context_ContinuedLine
    };

    TState State = State_Initial;
    TContext Context = Context_Single;
    bool IsInQuote = false, IsEscaped = false;
    char CurrentQuoteChar = 0;

    for (unsigned i = 0; i < Line.size(); i++) {
	char Char = Line[i];

	if (State == State_Initial) {
            if (fCommentHeader == '\0') {
                State = State_Depth;
            }
	    else if (Char == fCommentHeader) {
                State = State_Header;
            }
            else {
		return false;
	    }
        }
	if (State == State_Header) {
	    if (Char == fCommentHeader) {
		continue;
	    }
	    else {
		State = State_Depth;
	    }
	}
	if (State == State_Depth) {
	    if ((fTreeIndent == '\0') && isspace(Char)) {
                Depth++;
                continue;
            }
	    else if (Char == fTreeIndent) {
		Depth++;
		continue;
	    }
            else if (isspace(Char)) {
                continue;
            }
	    else if (Char == '-') {
                // YAML-Style Array //
                Depth++;
                Context = Context_Array;
                State = State_Separator;
            }
	    else if (Char == ':') {
                // continued line //
                Context = Context_ContinuedLine;
                State = State_Separator;
	    }
	    else {
		State = State_Name;
	    }
	}
	if (State == State_Name) {
            if (IsEscaped) {
		Name += Char;
                IsEscaped = false;
		continue;
            }
            else if (Char == '\\') {
                IsEscaped = true;
                continue;
            }
            if (IsInQuote) {
                if (Char == CurrentQuoteChar) {
                    IsInQuote = false;
                    continue;
                }
                Name += Char;
                continue;
            }
            else if ((Char == '"') || (Char == '\'')) {
                IsInQuote = true;
                CurrentQuoteChar = Char;
                continue;
            }
                
	    if (! isspace(Char) && (Char != ':')) {
		Name += Char;
		continue;	    
	    }

            State = State_PreSeparatorSpace;
	}
	if (State == State_PreSeparatorSpace) {
	    if (isspace(Char)) {
		continue;	    
	    }
	    else {
		State = State_Separator;
	    }
	}
	if (State == State_Separator) {
	    if (Char == ':') {
		State = State_PostSeparatorSpace;
		continue;
	    }
	    else if (Context != Context_Single) {
		State = State_PostSeparatorSpace;
		continue;	    
            }
	    else {
		return false;
	    }
	}
	if (State == State_PostSeparatorSpace) {
	    if (isspace(Char)) {
                continue;
	    }
	    else {
		State = State_Value;
	    }
	}
	if (State == State_Value) {
	    Value += Char;
	    continue;	    
	}
    }

    if ((State != State_Value) && (State != State_PostSeparatorSpace)) {
	return false;
    }

    string::size_type End = Value.find_last_not_of(" \t\r\n\x0a\x0d");
    if (End != string::npos) {
        Value = Value.substr(0, End+1);
    }

    if (fTreeBuilder) {
        if (Context == Context_ContinuedLine) {
            fTreeBuilder->AppendLine(Value);
        }
        else {
            fTreeBuilder->AddNode(Depth, Name, Value);
        }

#if 1
        //... for backwards compatibility ...//
        if (
            ((Name == "Fields") || (Name == "FieldTypes") || (Name == "Units"))
        ){
            Name = Name.substr(0, Name.size()-1);

            vector<string> ElementList;
            char OldDelimiter = this->SetDelimiter('\0');
            this->Tokenize(Value, ElementList);        
            this->SetDelimiter(OldDelimiter);
            
            Value = "[";
            for (unsigned i = 0; i < ElementList.size(); i++) {
                if (i != 0) {
                    Value += ", ";
                }
                Value += "\"" + ElementList[i] + "\"";
            }
            Value += "]";

            fTreeBuilder->AddNode(Depth, Name, Value);
        }
#endif
    }

    if ((Name == "Delimiter") && ! Value.empty()) {
	SetDelimiter(Value[0]);  // for header parsing
    }
    else if ((Name == "Quote") && ! Value.empty()) {
	SetQuote(Value[0]);  // for header parsing
    }
    
    return true;
}

bool KTabreeFormatHeaderProcessor::ProcessCsvHeader(const std::string& Line)
{    
    string Field, FieldList;
    bool IsQuoted = false, IsEscaped = false;
    for (unsigned i = 0; i < Line.size(); i++) {
        char ch = Line[i];
        if (IsEscaped) {
            Field += ch;
            IsEscaped = false;
        }
        else if (ch == '\\') {
            IsEscaped = true;
            continue;
        }
        else if (IsQuoted) {
            if (ch != fQuote) {
                Field += ch;
            }
            else {
                IsQuoted = false;
            }
        }
        else if (ch == fQuote) {
            IsQuoted = true;
        }
        else if (ch == ',') {
            FieldList += "  \"" + Field + "\"";
            Field.clear();
        }
        else {
            Field += ch;
        }
    }
    if (! Field.empty()) {
        FieldList += "  \"" + Field + "\"";
    }

    ProcessLine("# Fields:" + FieldList);
    
    return true;
}

bool KTabreeFormatHeaderProcessor::ProcessRootTreeDescriptor(const std::string& Line)
{    
    enum TState { 
	State_LeafStart, 
	State_Name, 
	State_LengthStart, 
	State_Length, 
	State_LengthComplete,
	State_TypeStart,
	State_TypeComplete,
	State_SizeComplete,
	State_Illigal
    };

    TState State = State_LeafStart;
    string Name, Type = "float";

    vector<pair<string, string> > LeafList;
    bool HasValidTypeSyntax = false, IsSpaceUsed = false;

    for (unsigned i = 0; i < Line.size(); i++) {
	char Char = Line[i];

	if (State == State_LeafStart) {
            if (! Name.empty()) {
                LeafList.push_back(make_pair(Name, Type));
                Name.clear();
            }

            // using space and starting with a digit are both allowed //
            // although ROOT's TTree::ReadFile() does not accept white spaces //
            if (isalnum(Char) || isspace(Char) || Char == '_') {
                Name = Char;
                State = State_Name;
                if (isspace(Char)) {
                    IsSpaceUsed = true;
                }
            }
            else {
                State = State_Illigal;
                break;
            }
	}
	else if (State == State_Name) {
            if (isalnum(Char) || isspace(Char) || Char == '_') {
                Name += Char;
                if (isspace(Char)) {
                    IsSpaceUsed = true;
                }
            }
            else {
                if (Char == '[') {
                    State = State_LengthStart;
                }
                else if (Char == '/') {
                    State = State_TypeStart;
                }
                else if (Char == ':') {
                    State = State_LeafStart;
                }
                else {
                    State = State_Illigal;
                    break;
                }
            }
        }
	else if (State == State_LengthStart) {
            if (isdigit(Char)) {
                State = State_Length;
            }
            else {
                State = State_Illigal;
                break;
            }
        }
	else if (State == State_Length) {
            if (isdigit(Char)) {
                ;
            }
            else if (Char == ']') {
                State = State_LengthComplete;
            }
            else {
                State = State_Illigal;
                break;
            }
        }
	else if (State == State_LengthComplete) {
            if (Char == '/') {
                State = State_TypeStart;
            }
            else if (Char == ':') {
                State = State_LeafStart;
            }
            else {
                State = State_Illigal;
                break;
            }
        }
	else if (State == State_TypeStart) {
            switch (Char) {
              case 'C':
                Type = "string";
                break;
              case 'B':
              case 'b':
              case 'S':
              case 's':
              case 'I':
              case 'i':
                Type = "int";
                break;
              case 'L':
              case 'l':
                Type = "long";
                break;
              case 'F':
                Type = "float";
                break;
              case 'D':
                Type = "double";
                break;
              default:
                State = State_Illigal;
                break;
            }

            if (State != State_Illigal) {
                State = State_TypeComplete;
                HasValidTypeSyntax = true;
            }
            else {
                break;
            }
	}
	else if (State == State_TypeComplete) {
            if (isdigit(Char)) {
                State = State_SizeComplete;
            }
            else if (Char == ':') {
                State = State_LeafStart;
            }
            else {
                State = State_Illigal;
                break;
            }
        }
	else if (State == State_SizeComplete) {
            if (Char == ':') {
                State = State_LeafStart;
            }
            else {
                State = State_Illigal;
                break;
            }
        }
    }
    if (! Name.empty()) {
        LeafList.push_back(make_pair(Name, Type));
    }

    if (
        (State != State_SizeComplete) && 
        (State != State_TypeComplete) && 
        (State != State_LengthComplete) && 
        (State != State_Name)
    ){
        return false;
    }
    
    // To be recognized as a ROOT tree descriptor, the line must
    // *) contain at least one valid type specifier, or
    // *) consist of multiple leaves and no space used in names
    if (! HasValidTypeSyntax && ((LeafList.size() < 2) || IsSpaceUsed)) {
        return false;
    }
    
    string ColumnList, ColumnTypeList;
    string Quote = IsSpaceUsed ? "\"" : "";
    for (unsigned i = 0; i < LeafList.size(); i++) {
        ColumnList += Quote + LeafList[i].first + Quote + " ";
        ColumnTypeList += LeafList[i].second + " ";
    }
    ProcessLine("# Fields: " + ColumnList);
    ProcessLine("# FieldTypes: " + ColumnTypeList);
    
    return true;
}
