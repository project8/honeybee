// KTabreeFormat.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>
#include <cstdio>
#include <cctype>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "KTabree.h"
#include "KTabreeFormat.h"
#include "KTreeSerializer.h"


using namespace std;
using namespace tabree;


KTabreeFormat::KTabreeFormat()
{
    fDefaultHeaderProcessor = nullptr;
    fHeaderProcessor = nullptr;

    fDefaultDelimiter = '\0';
    fDefaultQuote = '\0';
    fLineTerminator = '\0';  // auto-detect

    fCommentHeader = '#';
    fTreeIndent = '\0';

    fIsCsvHeaderEnabled = false;
}

KTabreeFormat::~KTabreeFormat()
{
    delete fDefaultHeaderProcessor;
}

void KTabreeFormat::EnableCsvHeader()
{
    fIsCsvHeaderEnabled = true;
    fDefaultDelimiter = ',';
}

void KTabreeFormat::SetDelimiter(char Delimiter)
{
    fDefaultDelimiter = Delimiter;
}

void KTabreeFormat::SetQuote(char Quote)
{
    fDefaultQuote = Quote;
}

void KTabreeFormat::SetLineTerminator(char LineTerminator)
{
    fLineTerminator = LineTerminator;
}

void KTabreeFormat::SetCommentHeader(char CommentHeader)
{
    fCommentHeader = CommentHeader;
}

void KTabreeFormat::SetTreeIndent(char TreeIndent)
{
    fTreeIndent = TreeIndent;
}

void KTabreeFormat::SetHeaderProcessor(KTabreeFormatHeaderProcessor* HeaderProcessor)
{
    fHeaderProcessor = HeaderProcessor;
}

void KTabreeFormat::Read(KTabree& Tabree, istream& Input) 
{
    string Line;
    KTabreeFormatColumnParser ColumnParser;
    KTabreeFormatLineExtractor LineExtractor(Input, fLineTerminator);

    unsigned ColumnOffset = Tabree.NumberOfColumns();
    KTree& HeaderTree = Tabree.Tree();

    if (fHeaderProcessor == nullptr) {
        fDefaultHeaderProcessor = new KTabreeFormatHeaderProcessor();
        fHeaderProcessor = fDefaultHeaderProcessor;
    }
    fHeaderProcessor->SetStorage(&HeaderTree);
    if (fIsCsvHeaderEnabled) {
        fHeaderProcessor->EnableCsvHeader();
    }
    if (fDefaultDelimiter != '\0') {
        fHeaderProcessor->SetDelimiter(fDefaultDelimiter);
    }
    if (fDefaultQuote != '\0') {
        fHeaderProcessor->SetQuote(fDefaultQuote);
    }
    fHeaderProcessor->SetCommentHeader(fCommentHeader);
    fHeaderProcessor->SetTreeIndent(fTreeIndent);

    int RowIndex = 0, LineNumber = 0;
    bool IsParserInitialized = false;
    while (LineExtractor.GetNext(Line)) {
        LineNumber++;

	// blank line //
	if (Line.find_first_not_of(" \t\r\n\x0a\x0d") == string::npos) {
	    if (RowIndex > 0) {
		Tabree.BreakSegment();
	    }
	    continue;
	}

	// header/comment line //
        if (LineNumber == 1) {
            if (fHeaderProcessor->ProcessFirstLine(Line)) {
                continue;
            }
        }
        if ((fCommentHeader == '\0') || (Line[0] == fCommentHeader)) {
            fHeaderProcessor->ProcessLine(Line);
            continue;
	}

	// table data line //
	if (! IsParserInitialized) {
            vector<string> ColumnTypeList;
            for (unsigned i = 0; i < HeaderTree["FieldType"].Length(); i++) {
                ColumnTypeList.push_back(
                    HeaderTree["FieldType"][i].As<string>()
                );
            }
            ColumnParser.SetColumnTypeList(ColumnTypeList);
            if (! HeaderTree["Delimiter"].IsVoid()) {
                ColumnParser.SetDelimiter(
                    HeaderTree["Delimiter"].As<string>()[0]
                );
            }
            else if (fDefaultDelimiter != '\0') {
                ColumnParser.SetDelimiter(fDefaultDelimiter);
            }
            if (! HeaderTree["Quote"].IsVoid()) {
		ColumnParser.SetQuote(
                    HeaderTree["Quote"].As<string>()[0]
                );
            }
            else if (fDefaultQuote != '\0') {
		ColumnParser.SetQuote(fDefaultQuote);
            }
	    IsParserInitialized = true;
	}
	try {
	    if (ColumnParser.ProcessLine(Tabree[RowIndex], Line, ColumnOffset)) {
		RowIndex++;
	    }
	}
	catch (KException &e) {
	    ostringstream os;
	    os << "line " << LineNumber << ": " << e.what();
	    throw KException() << os.str();
	}
    }

    for (unsigned i = 0; i < Tabree.NumberOfColumns(); i++) {
        string ColumnName = HeaderTree[string("Field")][i];
        string ColumnType = HeaderTree[string("FieldType")][i];
        if (ColumnName.empty()) {
            ostringstream os;
            os << "Column" << setfill('0') << setw(2) << i;
            ColumnName = os.str();
        }
        Tabree.SetColumnProperties(ColumnOffset + i, ColumnName, ColumnType);
    }
}

void KTabreeFormat::Write(const KTabree& Tabree, ostream& Output) 
{
    KTree HeaderTree = Tabree.Tree();

    string Delimiter, Quote;
    if (fIsCsvHeaderEnabled) {
        Delimiter = ",";
    }
    else if (! HeaderTree["Delimiter"].Value().IsVoid()) {
        Delimiter = HeaderTree["Delimiter"].As<string>();
    }
    else if (fDefaultDelimiter != '\0') {
        Delimiter = fDefaultDelimiter;
    }
    if (! HeaderTree["Quote"].Value().IsVoid()) {
        Quote = HeaderTree["Quote"].As<string>();
    }
    else if (fDefaultQuote != '\0') {
        Quote = fDefaultQuote;
    }
    
    string HeaderQuote = Quote;
    if (Delimiter.empty() && Quote.empty()) {
	unsigned NumberOfColumns = Tabree.NumberOfColumns();
	for (unsigned Column = 0; Column < NumberOfColumns; Column++) {
            string ColumnName = Tabree.ColumnNameList()[Column];
	    if (ColumnName.find_first_of(" \t") != string::npos) {
		HeaderQuote = "\"";
                break;
	    }
	}
    }

    if (! fIsCsvHeaderEnabled) {
        Delimiter += " ";
    }

    string ColumnList;
    unsigned NumberOfColumns = Tabree.NumberOfColumns();
    for (unsigned Column = 0; Column < NumberOfColumns; Column++) {
        string ColumnName = Tabree.ColumnNameList()[Column];
	if (Column > 0) {
	    ColumnList += Delimiter;
	}
	ColumnList += HeaderQuote + ColumnName + HeaderQuote;
    }
    if (! ColumnList.empty()) {
        //... for backwards compatibility ...//
        HeaderTree["Fields"] = ColumnList;
    }

    const vector<string>& ColumnTypeList = Tabree.ColumnTypeList();
    if (! ColumnTypeList.empty()) {
	string ColumnTypeListString;
	for (unsigned Column = 0; Column < ColumnTypeList.size(); Column++) {
	    if (Column > 0) {
		ColumnTypeListString += Delimiter;
	    }
	    ColumnTypeListString += HeaderQuote + ColumnTypeList[Column] + HeaderQuote;
	}
        //... for backwards compatibility ...//
	HeaderTree["FieldTypes"] = ColumnTypeListString;
    }
    
    if (fIsCsvHeaderEnabled) {
        Output << ColumnList << endl;
    }
    else {
        if (! Delimiter.empty()) {
            Output << fCommentHeader << " Delimiter: " << Delimiter << endl;
        }
        if (! Quote.empty()) {
            Output << fCommentHeader << " Quote: " << Quote << endl;
        }
        KKtfTreeSerializer(Output).Serialize(HeaderTree);
        Output << endl;
    }

    unsigned CurrentSegmentIndex = 0;
    for (unsigned Row = 0; Row < Tabree.NumberOfRows(); Row++) {
	if (Tabree.SegmentIndexOf(Row) != CurrentSegmentIndex) {
	    CurrentSegmentIndex = Tabree.SegmentIndexOf(Row);
	    Output << endl;
	}
	for (unsigned Column = 0; Column < NumberOfColumns; Column++) {
	    bool IsString = Tabree[Row][Column].IsString();
	    if ((Column == 0) && Quote.empty() && IsString) {
		string Value = Tabree[Row][Column];
		if ((Value.size() > 0) && (Value[0] == fCommentHeader)) {
		    Output << '\\';
		}
	    }
	    if (Column != 0) {
		Output << Delimiter;
	    }
		
	    if (IsString) {
		Output << Quote << Tabree[Row][Column] << Quote;
	    }
	    else if (Tabree[Row][Column].IsInteger()) {
		Output << Tabree[Row][Column].AsLong();
            }
	    else if (Tabree[Row][Column].IsNumeric()) {
		Output << setprecision(9) << Tabree[Row][Column].AsDouble();
            }
	    else {
		Output << Tabree[Row][Column];
	    }
	}
	Output << endl;
    }
}
