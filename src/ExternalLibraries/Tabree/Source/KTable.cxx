// KTable.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include "KTable.h"


using namespace std;
using namespace tabree;


KTable::KTable()
{
    fNumberOfRows = 0;
}

KTable::KTable(unsigned NumberOfColumns)
: fTable(NumberOfColumns)
{
    fNumberOfRows = 0;
    for (unsigned Index = 0; Index < NumberOfColumns; Index++) {
        ostringstream os;
        os << setfill('0') << setw(3) << Index;
        fColumnNameList.push_back(os.str());
    }
}

KTable::KTable(const KTable& Table)
{
    if (this != &Table) {
	fNumberOfRows = Table.fNumberOfRows;
	fTable = Table.fTable;
	fColumnNameList = Table.fColumnNameList;
    }
}

KTable& KTable::operator=(const KTable& Table)
{
    if (this != &Table) {
        fNumberOfRows = Table.fNumberOfRows;
        fTable = Table.fTable;
        fColumnNameList = Table.fColumnNameList;
    }

    return *this;
}

unsigned KTable::AddColumn(const std::string& ColumnName, int ColumnIndex) 
{
    unsigned Index = (ColumnIndex < 0) ? fTable.size() : ColumnIndex;

    if (Index >= fTable.size()) {
	fTable.insert(
	    fTable.end(), Index - fTable.size() + 1, 
	    std::vector<KVariant>(fNumberOfRows)
	);
    }

    while (fColumnNameList.size() <= Index) {
        ostringstream os;
        os << setfill('0') << setw(3) << fColumnNameList.size();
        fColumnNameList.push_back(os.str());
    }
    if (! ColumnName.empty()) {
        fColumnNameList[Index] = ColumnName;
    }

    fColumnNameIndexTable[fColumnNameList[Index]] = Index;

    return Index;
}

unsigned KTable::AddRow(int RowIndex) 
{
    unsigned Index = (RowIndex < 0) ? fNumberOfRows : RowIndex;

    if (Index >= fNumberOfRows) {
	fNumberOfRows = Index + 1;
	for (unsigned i = 0; i < fTable.size(); i++) {
	    fTable[i].resize(fNumberOfRows);
	}
    }
    
    return Index;
}

void KTable::Clear()
{
    for (unsigned i = 0; i < fTable.size(); i++) {
	fTable[i].erase(fTable[i].begin(), fTable[i].end());
    }
    fNumberOfRows = 0;
}

void KTable::Dump(std::ostream& os)
{
    if (NumberOfColumns()) {
        os << "# ColumnNames: ";
        for (unsigned i = 0; i < NumberOfColumns(); i++) {
            if (i != 0) {
                os << "  ";
            }
            os << '"' << ColumnNameList()[i] << '"';
        }
    }
    os << endl << endl;

    for (unsigned i = 0; i < NumberOfRows(); i++) {
	for (unsigned j = 0; j < NumberOfColumns(); j++) {
            if (j != 0) {
                os << "  ";
            }
            KVariant& Value = this->operator[](i)[j];
            if (Value.IsVoid()) {
                os << "\"\"";
            }
            else if (Value.IsString()) {
                os << '"' << this->operator[](i)[j] << '"';
            }
            else {
                os << this->operator[](i)[j];
            }
	}
	os << endl;
    }
}
