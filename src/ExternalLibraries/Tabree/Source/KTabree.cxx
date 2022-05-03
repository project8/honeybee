// KTabree.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include "KTabree.h"

using namespace std;
using namespace tabree;


KTabree::KTabree()
{
    fLastRowIndex = 0;
    fLastSegmentIndex = 0;
}

KTabree::KTabree(unsigned NumberOfColumns)
: KTable(NumberOfColumns)
{
    fLastRowIndex = 0;
    fLastSegmentIndex = 0;
}

KTabree::KTabree(const KTabree& Tabree)
: KTable(Tabree), fTree(Tabree.fTree)
{
    fColumnTypeList = Tabree.fColumnTypeList;
    fSegmentBoundaryList = Tabree.fSegmentBoundaryList;
    fLastRowIndex = Tabree.fLastRowIndex;
    fLastSegmentIndex = Tabree.fLastSegmentIndex;
}

KTabree::~KTabree()
{
}

KTabree& KTabree::operator=(const KTabree& Tabree)
{
    if (this == &Tabree) {
        return *this;
    }

    KTable::operator=(Tabree);
    fTree = Tabree.fTree;

    fColumnTypeList = Tabree.fColumnTypeList;
    fSegmentBoundaryList = Tabree.fSegmentBoundaryList;
    fLastRowIndex = Tabree.fLastRowIndex;
    fLastSegmentIndex = Tabree.fLastSegmentIndex;

    return *this;
}


void KTabree::SetColumnProperties(unsigned ColumnIndex, const std::string& ColumnName, const std::string& ColumnType)
{
    KTable::AddColumn(ColumnName, ColumnIndex);

    if (! ColumnType.empty()) {
        while (fColumnTypeList.size() < ColumnIndex) {
            fColumnTypeList.push_back("");
        }
        fColumnTypeList.push_back(ColumnType);
    }
}

const std::vector<std::string>& KTabree::ColumnTypeList() const
{
    return fColumnTypeList;
}

void KTabree::BreakSegment()
{
    fSegmentBoundaryList.push_back(NumberOfRows());
}

unsigned KTabree::NumberOfSegments() const
{
    return fSegmentBoundaryList.size();
}

unsigned KTabree::SegmentIndexOf(unsigned RowIndex) const
{
    unsigned SegmentIndex = (
	(RowIndex >= fLastRowIndex) ? fLastSegmentIndex : 0
    );

    while (SegmentIndex < fSegmentBoundaryList.size()) {
	if (RowIndex < fSegmentBoundaryList[SegmentIndex]) {
	    break;
	}
	SegmentIndex++;
    }

    fLastRowIndex = RowIndex;
    fLastSegmentIndex = SegmentIndex;

    return SegmentIndex;
}

void KTabree::Dump(std::ostream& os)
{
    os << "### TREE CONTENT ###" << endl;
    fTree.Dump(os);
    os << endl;

    os << "### TABLE CONTENT ###" << endl;

    if (! fColumnTypeList.empty()) {
        os << "# ColumnTypes: ";
        for (unsigned i = 0; i < fColumnTypeList.size(); i++) {
            if (i != 0) {
                os << "  ";
            }
            os << fColumnTypeList[i];
        }
        os << endl;
    }

    KTable::Dump(os);
}
