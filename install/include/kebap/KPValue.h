// KPValue.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPValue_h__
#define __KPValue_h__

#include <string>
#include <complex>
#include <vector>
#include <map>
#include "KPException.h"


namespace kebap {

class KPObjectPrototype;
class KPListValue;

class KPVariant {};

class KPValue {
  public:
    explicit KPValue();
    explicit KPValue(bool BoolValue);
    explicit KPValue(long LongValue);
    explicit KPValue(double DoubleValue);
    explicit KPValue(std::complex<double> ComplexValue);
    explicit KPValue(const std::string& StringValue);
    explicit KPValue(KPObjectPrototype* ObjectValue);
    explicit KPValue(KPValue* PointerValue);
    explicit KPValue(const KPVariant& VariantValue);
    explicit KPValue(const KPListValue& ListValue);
    KPValue(const KPValue& Value);
    KPValue& operator=(const KPValue& Value);
    virtual ~KPValue();
    virtual void Refer();
    virtual void Unrefer();
    virtual void Destroy();
    virtual void SetName(std::string Name);
    virtual void Assign(const KPValue& Value) ;
    virtual void AssignDouble(double Value) ;
    virtual bool IsVoid() const;
    virtual bool IsBool() const;
    virtual bool IsLong() const;
    virtual bool IsDouble() const;
    virtual bool IsComplex() const;
    virtual bool IsString() const;
    virtual bool IsObject() const;
    virtual bool IsPointer() const;
    virtual bool IsList() const;
    virtual bool IsNumeric() const;
    virtual bool IsReal() const;
    virtual bool IsObject(const std::string& InternalClassName) const;
    virtual std::string TypeName() const;
    virtual bool AsBool() const ;
    virtual long AsLong() const ;
    virtual double AsDouble() const ;
    virtual std::complex<double> AsComplex() const ;
    virtual std::string AsString() const ;
    virtual std::string& AsStringReference() const ;
    virtual KPObjectPrototype* AsObject() const ;
    virtual KPValue* AsPointer() const ;
    virtual KPListValue& AsList() ;
    virtual const KPListValue& AsConstList() const ;
    virtual std::vector<KPValue>& AsValueList() ;
    virtual void SetLeftValueFlag();
    virtual bool IsLeftValue() const;
    virtual void SetArrayLength(int ArrayLength);
    virtual int ArrayLength() const;
    virtual void SetVariantFlag();
    virtual bool IsVariant() const;
    virtual void SetArrayPointerFlag();
    virtual bool IsArrayPointer() const;
    virtual void Dump(std::ostream& os) const;
  protected:
    enum TValueType {
        ValueType_Void, 
        ValueType_Bool, 
        ValueType_Long, 
        ValueType_Double, 
        ValueType_Complex, 
        ValueType_String, 
	ValueType_Object, 
        ValueType_Pointer, 
        ValueType_List
    };
    union TPrimitiveValue {
	bool fBoolValue;
        long fLongValue;
        double fDoubleValue;
        std::complex<double>* fComplexValue;
	std::string* fStringValue;
        KPObjectPrototype* fObjectValue;
        KPValue* fPointerValue;
	KPListValue* fListValue;
    };
  protected:
    TPrimitiveValue fPrimitiveValue;
    TValueType fType;
    int fArrayLength;
    bool fIsLeftValue;
    bool fIsVariant;
    bool fIsArrayPointer;
};



class KPListValue {
  public:
    KPListValue();
    explicit KPListValue(unsigned InitialCapacity);
    KPListValue(const std::vector<KPValue>& ValueList);
    KPListValue(const KPListValue& ListValue);
    KPListValue& operator=(const KPListValue& ListValue);
    virtual ~KPListValue();
    virtual unsigned ListSize() const;
    virtual bool HasKeyIndex() const;
    virtual void AppendList(const KPListValue& ListValue);
    virtual unsigned AppendValue(const KPValue& Value);
    virtual std::vector<KPValue>& ValueList();
    virtual const std::vector<KPValue>& ConstValueList() const;
    virtual const std::vector<std::string>& KeyList() const;
    virtual void SetKey(unsigned Index, const std::string& Key);
    virtual std::string KeyOf(unsigned Index);
    virtual unsigned IndexOf(const std::string& Key);
    virtual KPValue& ValueOf(unsigned Index);
    virtual KPValue& ValueOf(const std::string& Key);
    virtual std::string AsString() const;
    inline KPValue& operator[] (unsigned Index);
    inline KPValue& operator[] (std::string& Key);
  protected:
    std::vector<KPValue> fValueList;
    mutable std::vector<std::string>* fKeyList;
    mutable std::map<std::string, unsigned>* fKeyIndexTable;
    mutable std::map<unsigned, std::string>* fIndexKeyTable;
};


inline KPValue& KPListValue::operator[] (unsigned Index)
{
    return ValueOf(Index);
}

inline KPValue& KPListValue::operator[] (std::string& Key)
{
    return ValueOf(Key);
}


}
#endif
