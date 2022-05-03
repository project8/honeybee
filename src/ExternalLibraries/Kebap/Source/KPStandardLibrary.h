// KPStandardLibrary.h //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#ifndef __KPStandardLibrary_h__
#define __KPStandardLibrary_h__

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "KPObject.h"


namespace kebap {

class KPConsoleObject: public KPObjectPrototype {
  public:
    KPConsoleObject();
    ~KPConsoleObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_Print = KPObjectPrototype::fNumberOfMethods,
	MethodId_PrintLine,
	MethodId_PutByte,
	MethodId_GetLine,
	MethodId_GetByte,
	fNumberOfMethods
    };
  protected:
    int Print(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int PrintLine(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int PutByte(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int GetLine(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int GetByte(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
};


class KPInputFileObject: public KPObjectPrototype {
  public:
    KPInputFileObject(std::istream* DefaultInputStream = nullptr);
    ~KPInputFileObject() override;
    KPObjectPrototype* Clone() override;
    void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList) override ;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_GetLine = KPObjectPrototype::fNumberOfMethods,
	MethodId_GetByte,
	MethodId_GetInteger,
	fNumberOfMethods
    };
  protected:
    virtual int GetLine(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    virtual int GetByte(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    virtual int GetInteger(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
  private:
    std::istream* fFileStream;
    std::ifstream* fMyFileStream;
};


class KPOutputFileObject: public KPObjectPrototype {
  public:
    KPOutputFileObject(std::ostream* DefaultOutputStream = nullptr);
    ~KPOutputFileObject() override;
    KPObjectPrototype* Clone() override;
    void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList) override ;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_Print = KPObjectPrototype::fNumberOfMethods,
	MethodId_PrintLine,
	MethodId_PutByte,
	fNumberOfMethods
    };
  protected:
    virtual int Print(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    virtual int PrintLine(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    virtual int PutByte(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
  private:
    std::ostream* fFileStream;
    std::ofstream* fMyFileStream;
};


class KPInputPipeObject: public KPInputFileObject {
  public:
    KPInputPipeObject();
    ~KPInputPipeObject() override;
    KPObjectPrototype* Clone() override;
    void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList) override ;
  protected:
    int GetLine(std::vector<KPValue*>& ArgumentList, KPValue& Result) override ;
    int GetByte(std::vector<KPValue*>& ArgumentList, KPValue& Result) override ;
  private:
    std::FILE* fPipe;
};


class KPOutputPipeObject: public KPOutputFileObject {
  public:
    KPOutputPipeObject();
    ~KPOutputPipeObject() override;
    KPObjectPrototype* Clone() override;
    void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList) override ;
  protected:
    int Print(std::vector<KPValue*>& ArgumentList, KPValue& Result) override ;
    int PrintLine(std::vector<KPValue*>& ArgumentList, KPValue& Result) override ;
  private:
    std::FILE* fPipe;
};


class KPFormatterObject: public KPObjectPrototype {
  public:
    KPFormatterObject();
    ~KPFormatterObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_Put = KPObjectPrototype::fNumberOfMethods,
	MethodId_Flush,
	MethodId_SetWidth,
	MethodId_SetPrecision,
	MethodId_SetFill,
	MethodId_SetBase,
	MethodId_Hex,
	MethodId_Dec,
	MethodId_Fixed,
	MethodId_Scientific,
	fNumberOfMethods
    };
  protected:
    int Put(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Flush(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int SetWidth(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int SetPrecision(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int SetFill(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int SetBase(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Hex(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Dec(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Fixed(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Scientific(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
  private:
    std::ostringstream* fFormatStream;
};


class KPScannerObject: public KPObjectPrototype {
  public:
    KPScannerObject();
    ~KPScannerObject() override;
    KPObjectPrototype* Clone() override;
    void Construct(const std::string& ClassName, std::vector<KPValue*>& ArgumentList) override ;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    enum {
	MethodId_Load = KPObjectPrototype::fNumberOfMethods,
	MethodId_Get,
	MethodId_GetLine,
	MethodId_SkipWhiteSpace,
	MethodId_SetBase,
	MethodId_IsGood,
	MethodId_LastGetCount,
	fNumberOfMethods
    };
  protected:
    int Load(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int Get(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int GetLine(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int SkipWhiteSpace(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int SetBase(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int IsGood(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
    int LastGetCount(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) ;
  private:
    std::istringstream* fSourceStream;
};


class KPArgumentObject: public KPObjectPrototype {
  public:
    KPArgumentObject(int argc, char** argv);
    ~KPArgumentObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    virtual void Parse();
  protected:
    enum {
	MethodId_NumberOfArguments = KPObjectPrototype::fNumberOfMethods,
	MethodId_GetArgumentOf,
	MethodId_NumberOfParameters,
	MethodId_GetParameterOf,
	MethodId_IsOptionSpecified,
	MethodId_GetOptionValueOf,
	MethodId_IsSwitchSpecified,
	fNumberOfMethods
    };
  protected:
    int NumberOfArguments(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int GetArgumentOf(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int NumberOfParameters(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int GetParameterOf(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int IsOptionSpecified(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int GetOptionValueOf(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int IsSwitchSpecified(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
  private:
    int fArgc;
    char** fArgv;
    bool fIsParsed;
    std::vector<std::string> fArgumentList;
    std::vector<std::string> fParameterList;
    std::map<std::string, std::string> fOptionTable;
    std::set<char> fSwitchSet;
};


class KPStringObject: public KPObjectPrototype {
  public:
    KPStringObject();
    ~KPStringObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    int Chop(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int Chomp(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int Substr(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int Index(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
  protected:
    enum {
	MethodId_Chop = KPObjectPrototype::fNumberOfMethods,
	MethodId_Chomp,
	MethodId_Substr,
	MethodId_Index,
	fNumberOfMethods
    };
};


class KPSystemObject: public KPObjectPrototype {
  public:
    KPSystemObject();
    ~KPSystemObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    int System(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
    int Shell(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
  protected:
    enum {
	MethodId_System = KPObjectPrototype::fNumberOfMethods,
	MethodId_Shell,
	fNumberOfMethods
    };
};



class KPParser;
class KPExpressionParser;
class KPSymbolTable;
class KPTokenTable;

class KPParserObject: public KPObjectPrototype {
  public:
    KPParserObject(KPParser* Parser);
    ~KPParserObject() override;
    KPObjectPrototype* Clone() override;
    int MethodIdOf(const std::string& MethodName) override;
    int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override ;
  protected:
    int Evaluate(std::vector<KPValue*>& ArgumentList, KPValue& Result) ;
  protected:
    enum {
	MethodId_Evaluate = KPObjectPrototype::fNumberOfMethods,
	fNumberOfMethods
    };
  protected:
    KPParser* fParser;
    KPExpressionParser* fExpressionParser;
    KPSymbolTable* fSymbolTable;
    KPTokenTable* fTokenTable;
};


}
#endif
