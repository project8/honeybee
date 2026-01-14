// KTabree.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef KTabree_h__
#define KTabree_h__


#include <string>
#include <vector>
#include "KTree.h"
#include "KTable.h"


namespace tabree {


/**
 * \brief Tabree (= Table + Tree) data structure
 */
class KTabree: public KTable {
  public:
    KTabree();
    explicit KTabree(unsigned NumberOfColumns);
    KTabree(const KTabree& Tabree);
    ~KTabree() override;
    virtual KTabree& operator=(const KTabree& Tabree);
  public:
    inline KTable::TRow operator[](unsigned RowIndex);
    inline KTree& operator[](const std::string& Key) ;
    inline KTree& Tree();
    inline KTable& Table();
  public:
    inline const KTable::TRow operator[](unsigned RowIndex) const ;
    inline const KTree& operator[](const std::string& Key) const ;
    inline const KTree& Tree() const;
  public:
    virtual void SetColumnProperties(unsigned Index, const std::string& Name = "", const std::string& Type = "");
    virtual const std::vector<std::string>& ColumnTypeList() const;
    virtual void BreakSegment();
    virtual unsigned NumberOfSegments() const;
    virtual unsigned SegmentIndexOf(unsigned RowIndex) const;
    void Dump(std::ostream& os = std::cout) override;
  private:
    mutable KTree fTree;
  private:
    std::vector<std::string> fColumnTypeList;
    std::vector<unsigned> fSegmentBoundaryList;
    mutable unsigned fLastRowIndex, fLastSegmentIndex;
};



inline KTable::TRow KTabree::operator[](unsigned RowIndex)
{
    return KTable::operator[](RowIndex);
}

inline KTree& KTabree::operator[](const std::string& Key) 
{
    return fTree[Key];
}

inline KTree& KTabree::Tree() 
{ 
    return fTree; 
}

inline KTable& KTabree::Table() 
{ 
    return *this; 
}

inline const KTable::TRow KTabree::operator[](unsigned RowIndex) const 
{
    return KTable::operator[](RowIndex);
}

inline const KTree& KTabree::operator[](const std::string& Key) const 
{
    return fTree[Key];
}

inline const KTree& KTabree::Tree() const
{ 
    return fTree; 
}


}

// suger for convenient includes //
#include "KTabreeFormat.h"


#endif
