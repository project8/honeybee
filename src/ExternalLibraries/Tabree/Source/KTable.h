// KTable.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#ifndef KTable_h__
#define KTable_h__


#include <vector>
#include <map>
#include "KVariant.h"


namespace tabree {


/**
 * \brief Table data structure
 */
class KTable {
  public:
    class TRow {
      public:
	inline TRow(KTable& Table, unsigned RowIndex);
	inline KVariant& operator[](unsigned ColumnIndex);
	inline KVariant& operator[](const std::string& ColumnName);
	inline const KVariant& operator[](unsigned ColumnIndex) const ;
	inline const KVariant& operator[](const std::string& ColumnName) const ;
      private:
	KTable& fTable;
	unsigned fRowIndex;
    };
  public:
    KTable();
    explicit KTable(unsigned NumberOfColumns);
    KTable(const KTable& Table);
    virtual ~KTable() {}
    virtual KTable& operator=(const KTable& Table);
    virtual unsigned AddColumn(const std::string& Name = "", int ColumnIndex = -1);
    virtual unsigned AddRow(int RowIndex = -1);
    virtual void Clear();
    virtual void Dump(std::ostream& os = std::cout);
  public:
    inline unsigned NumberOfRows() const;
    inline TRow operator[](unsigned RowIndex);
    inline const TRow operator[](unsigned RowIndex) const ;
  public:
    inline unsigned NumberOfColumns() const;
    inline const std::vector<std::string>& ColumnNameList() const;
    inline unsigned ColumnIndexOf(const std::string& ColumnName) const ;
    inline std::vector<KVariant>& GetColumn(unsigned ColumnIndex);
    inline const std::vector<KVariant>& GetColumn(unsigned ColumnIndex) const ;
    inline std::vector<KVariant>& GetColumn(const std::string& ColumnName);
    inline const std::vector<KVariant>& GetColumn(const std::string& ColumnName) const ;
  private:
    unsigned fNumberOfRows;
    std::vector<std::vector<KVariant> > fTable;
    std::vector<std::string> fColumnNameList;
    std::map<std::string, unsigned> fColumnNameIndexTable;
};



inline unsigned KTable::NumberOfRows() const
{
    return fNumberOfRows;
}

inline KTable::TRow KTable::operator[](unsigned RowIndex)
{
    if (RowIndex >= fNumberOfRows) {
        AddRow(RowIndex);
    }

    return TRow(*this, RowIndex);
}

inline const KTable::TRow KTable::operator[](unsigned RowIndex) const 
{
    if (RowIndex >= fNumberOfRows) {
	throw KException() << "table row index out of range";
    }

    return TRow(const_cast<KTable&>(*this), RowIndex);
}

inline unsigned KTable::NumberOfColumns() const
{
    return fTable.size();
}

inline const std::vector<std::string>& KTable::ColumnNameList() const
{
    return fColumnNameList;
}

inline std::vector<KVariant>& KTable::GetColumn(unsigned ColumnIndex)
{
    if (ColumnIndex >= fTable.size()) {
        AddColumn("", ColumnIndex);
    }

    return fTable[ColumnIndex];
}

inline const std::vector<KVariant>& KTable::GetColumn(unsigned ColumnIndex) const 
{
    if (ColumnIndex >= fTable.size()) {
	throw KException() << "table column index out of range";
    }

    return fTable[ColumnIndex];
}

inline std::vector<KVariant>& KTable::GetColumn(const std::string& ColumnName)
{
    std::map<std::string, unsigned>::const_iterator Iterator = (
        fColumnNameIndexTable.find(ColumnName)
    );
    if (Iterator == fColumnNameIndexTable.end()) {
        return fTable[AddColumn(ColumnName)];
    }

    return fTable[Iterator->second];
}

inline const std::vector<KVariant>& KTable::GetColumn(const std::string& ColumnName) const 
{
    auto Iterator = (
        fColumnNameIndexTable.find(ColumnName)
    );
    if (Iterator == fColumnNameIndexTable.end()) {
	throw KException() << "unknown table column name: " << ColumnName;
    }

    return fTable[Iterator->second];
}

inline unsigned KTable::ColumnIndexOf(const std::string& ColumnName) const 
{
    auto Iterator = (
        fColumnNameIndexTable.find(ColumnName)
    );
    if (Iterator == fColumnNameIndexTable.end()) {
	throw KException() << "unknown table column name: " << ColumnName;
    }

    return Iterator->second;
}


inline KTable::TRow::TRow(KTable& Table, unsigned RowIndex)
: fTable(Table), fRowIndex(RowIndex)
{
}

inline KVariant& KTable::TRow::operator[](unsigned ColumnIndex)
{
    return fTable.GetColumn(ColumnIndex)[fRowIndex];
}

inline KVariant& KTable::TRow::operator[](const std::string& ColumnName)
{
    return fTable.GetColumn(ColumnName)[fRowIndex];
}

inline const KVariant& KTable::TRow::operator[](unsigned ColumnIndex) const 
{
    return fTable.GetColumn(ColumnIndex)[fRowIndex];
}

inline const KVariant& KTable::TRow::operator[](const std::string& ColumnName) const 
{
    return fTable.GetColumn(ColumnName)[fRowIndex];
}


}
#endif
