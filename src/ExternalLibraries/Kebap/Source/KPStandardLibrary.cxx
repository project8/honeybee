// KPStandardLibrary.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <utility>
#include "KPObject.h"
#include "KPParser.h"
#include "KPStandardLibrary.h"

using namespace std;
using namespace kebap;


KPConsoleObject::KPConsoleObject()
: KPObjectPrototype("Console")
{
}

KPConsoleObject::~KPConsoleObject()
{
}

KPObjectPrototype* KPConsoleObject::Clone()
{
    return new KPConsoleObject();
}

int KPConsoleObject::MethodIdOf(const string& MethodName)
{
    if (MethodName == "print") {
        return MethodId_Print;
    }
    else if ((MethodName == "printLine") || (MethodName == "println")) {
        return MethodId_PrintLine;
    }
    else if ((MethodName == "putByte") || (MethodName == "putc")) {
        return MethodId_PutByte;
    }
    else if ((MethodName == "getLine") || (MethodName == "getln")) {
        return MethodId_GetLine;
    }
    else if ((MethodName == "getByte") || (MethodName == "getc")) {
        return MethodId_GetByte;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPConsoleObject::InvokeMethod(int MethodId, vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;

    switch (MethodId) {
      case MethodId_Print:
        Result = Print(ArgumentList, ReturnValue);
	break;
      case MethodId_PrintLine:
        Result = PrintLine(ArgumentList, ReturnValue);
	break;
      case MethodId_PutByte:
        Result = PutByte(ArgumentList, ReturnValue);
	break;
      case MethodId_GetLine:
        Result = GetLine(ArgumentList, ReturnValue);
	break;
      case MethodId_GetByte:
        Result = GetByte(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPConsoleObject::Print(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    for (unsigned i = 0; i < ArgumentList.size(); i++) {
        cout << ArgumentList[i]->AsString();
    }
    cout << flush;

    return 1;
}

int KPConsoleObject::PrintLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    Print(ArgumentList, ReturnValue);
    cout << endl;
    
    return 1;
}

int KPConsoleObject::PutByte(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << fInternalClassName << "::putByte(int byte): too few argument[s]";
    }
    if (! ArgumentList[0]->IsLong()) {
	throw KPException() << fInternalClassName << "::putByte(int byte): invalid argument[s]";
    }
    
    long Value = ArgumentList[0]->AsLong();
    cout.put((char) (Value & 0xff));

    return 1;
}

int KPConsoleObject::GetLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    char LineTerminator = '\n';
    if (ArgumentList.size() > 0) {
	LineTerminator = ArgumentList[0]->AsString()[0];
    }

    string Line;
    if (getline(cin, Line, LineTerminator)) {
	ReturnValue = KPValue(Line + LineTerminator);
    }
    else {
	ReturnValue = KPValue(string(""));
    }

    return 1;
}

int KPConsoleObject::GetByte(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    long Value = -1;

    char ch;
    if (cin.get(ch)) {
	Value = (long) ch;
    }

    ReturnValue = KPValue(Value);

    return 1;
}



KPInputFileObject::KPInputFileObject(istream* DefaultInputStream)
: KPObjectPrototype("InputFile")
{
    fFileStream = DefaultInputStream;
    fMyFileStream = nullptr;
}

KPInputFileObject::~KPInputFileObject()
{
    delete fMyFileStream;
}

KPObjectPrototype* KPInputFileObject::Clone()
{
    return new KPInputFileObject();
}

void KPInputFileObject::Construct(const string& ClassName, vector<KPValue*>& ArgumentList) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): too few argument[s]";
    }
    if (! ArgumentList[0]->IsString()) {
	throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): invalid argument[s]";
    }

    string FileName = ArgumentList[0]->AsString();
    fMyFileStream = new ifstream(FileName.c_str());
    if (! *fMyFileStream) {
	delete fMyFileStream;
	fMyFileStream = nullptr;
	throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): unable to open file: " << FileName;
    }

    fFileStream = fMyFileStream;
}

int KPInputFileObject::MethodIdOf(const string& MethodName)
{
    if ((MethodName == "getLine") || (MethodName == "getln")) {
        return MethodId_GetLine;
    }
    else if ((MethodName == "getByte") || (MethodName == "getc")) {
        return MethodId_GetByte;
    }
    else if ((MethodName == "getInteger") || (MethodName == "getint")) {
        return MethodId_GetInteger;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPInputFileObject::InvokeMethod(int MethodId, vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;
    
    switch (MethodId) {
      case MethodId_GetLine:
        Result = GetLine(ArgumentList, ReturnValue);
	break;
      case MethodId_GetByte:
        Result = GetByte(ArgumentList, ReturnValue);
	break;
      case MethodId_GetInteger:
	Result = GetInteger(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPInputFileObject::GetLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    char LineTerminator = '\n';
    if (ArgumentList.size() > 0) {
	LineTerminator = ArgumentList[0]->AsString()[0];
    }

    string Line;
    if (getline(*fFileStream, Line, LineTerminator)) {
	ReturnValue = KPValue(Line + LineTerminator);
    }
    else {
	ReturnValue = KPValue(string(""));
    }

    return 1;
}

int KPInputFileObject::GetByte(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    long Value = -1;

    char ch;
    if (fFileStream->get(ch)) {
	Value = (long) ch;
    }

    ReturnValue = KPValue(Value);

    return 1;
}

int KPInputFileObject::GetInteger(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    bool IsSuccess = true;
    long Value = 0;

    int WordLength = 4;
    if (ArgumentList.size() > 1) {
	WordLength = ArgumentList[1]->AsLong();
    }

    char Byte;
    for (int i = 0; i < WordLength; i++) {
	if (fFileStream->get(Byte)) {
#if 0
	    // big endian //
	    Value <<= 8;
	    Value |= 0x00ff & Byte;
#else
	    // little endian //
	    Value |= ((0x00ff & Byte) << (i*8));
#endif
	}
	else {
	    IsSuccess = false;
	    break;
	}
    }

    if (IsSuccess) {
	if (ArgumentList.size() > 0) {
	    ArgumentList[0]->Assign(KPValue(Value));
	}
    }

    ReturnValue = KPValue(IsSuccess);

    return 1;
}



KPOutputFileObject::KPOutputFileObject(ostream* DefaultOutputStream)
: KPObjectPrototype("OutputFile")
{
    fFileStream = DefaultOutputStream;
    fMyFileStream = nullptr;
}

KPOutputFileObject::~KPOutputFileObject()
{
    delete fMyFileStream;
}

KPObjectPrototype* KPOutputFileObject::Clone()
{
    return new KPOutputFileObject();
}

void KPOutputFileObject::Construct(const string& ClassName, vector<KPValue*>& ArgumentList) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): too few argument[s]";
    }
    if (! ArgumentList[0]->IsString()) {
	throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): invalid argument[s]";
    }

    string FileName = ArgumentList[0]->AsString();
    fMyFileStream = new ofstream(FileName.c_str());
    if (! *fMyFileStream) {
	delete fMyFileStream;
	fMyFileStream = nullptr;
	throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): unable to open file: " << FileName;
    }

    fFileStream = fMyFileStream;
}

int KPOutputFileObject::MethodIdOf(const string& MethodName)
{
    if (MethodName == "print") {
        return MethodId_Print;
    }
    else if ((MethodName == "printLine") || (MethodName == "println")) {
        return MethodId_PrintLine;
    }
    else if ((MethodName == "putByte") || (MethodName == "putc")) {
        return MethodId_PutByte;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPOutputFileObject::InvokeMethod(int MethodId, vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;
    
    switch (MethodId) {
      case MethodId_Print:
	Result = Print(ArgumentList, ReturnValue);
	break;
      case MethodId_PrintLine:
        Result = PrintLine(ArgumentList, ReturnValue);
	break;
      case MethodId_PutByte:
        Result = PutByte(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPOutputFileObject::Print(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    for (unsigned i = 0; i < ArgumentList.size(); i++) {
        (*fFileStream) << ArgumentList[i]->AsString();
    }
    (*fFileStream) << flush;

    return 1;
}

int KPOutputFileObject::PrintLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    Print(ArgumentList, ReturnValue);
    (*fFileStream) << endl;
    
    return 1;
}

int KPOutputFileObject::PutByte(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << fInternalClassName << "::putByte(int byte): too few argument[s]";
    }
    if (! ArgumentList[0]->IsLong()) {
	throw KPException() << fInternalClassName << "::putByte(int byte): invalid argument[s]";
    }
    
    long Value = ArgumentList[0]->AsLong();
    fFileStream->put((char) (Value & 0xff));

    return 1;
}



KPInputPipeObject::KPInputPipeObject()
: KPInputFileObject()
{
    fPipe = nullptr;
}

KPInputPipeObject::~KPInputPipeObject()
{
    if ((fPipe != nullptr) && (fPipe != nullptr)) {
	pclose(fPipe);
    }
}

KPObjectPrototype* KPInputPipeObject::Clone()
{
    return new KPInputPipeObject();
}

void KPInputPipeObject::Construct(const string& ClassName, vector<KPValue*>& ArgumentList) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << "InputPipe::InputPipe(string command): too few argument[s]";
    }
    if (! ArgumentList[0]->IsString()) {
	throw KPException() << "InputPipe::InputPipe(string command): too few argument[s]";
    }

    string CommandName = ArgumentList[0]->AsString();
    fPipe = popen(CommandName.c_str(), "r");
    if ((fPipe == nullptr) || (fPipe == nullptr)) {
	throw KPException() << "InputPipe::InputPipe(string command): unable to open pipe: '" << CommandName << "'";
    }
}

int KPInputPipeObject::GetLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    char LineTerminator = '\n';
    if (ArgumentList.size() > 0) {
	LineTerminator = ArgumentList[0]->AsString()[0];
    }

    ReturnValue = KPValue(string());
    string& Line = ReturnValue.AsStringReference();

    int ch;
    while ((ch = fgetc(fPipe)) != EOF) {
	Line += ch;
	if (ch == LineTerminator) {
	    break;
	}
    }

    return 1;
}

int KPInputPipeObject::GetByte(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    long Value = -1;

    int ch = fgetc(fPipe);
    if (ch != EOF) {
	Value = (long) ch;
    }

    ReturnValue = KPValue(Value);

    return 1;
}



KPOutputPipeObject::KPOutputPipeObject()
: KPOutputFileObject()
{
    fPipe = nullptr;
}

KPOutputPipeObject::~KPOutputPipeObject()
{
    if ((fPipe != nullptr) && (fPipe != nullptr)) {
	pclose(fPipe);
    }
}

KPObjectPrototype* KPOutputPipeObject::Clone()
{
    return new KPOutputPipeObject();
}

void KPOutputPipeObject::Construct(const string& ClassName, vector<KPValue*>& ArgumentList) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << "OutputPipe::OutputPipe(string command): too few argument[s]";
    }
    if (! ArgumentList[0]->IsString()) {
	throw KPException() << "OutputPipe::OutputPipe(string command): too few argument[s]";
    }

    string CommandName = ArgumentList[0]->AsString();
    fPipe = popen(CommandName.c_str(), "w");
    if ((fPipe == nullptr) || (fPipe == nullptr)) {
	throw KPException() << "InputPipe::InputPipe(string command): unable to open pipe: '" << CommandName << "'";
    }
}

int KPOutputPipeObject::Print(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    for (unsigned i = 0; i < ArgumentList.size(); i++) {
        fputs(ArgumentList[i]->AsString().c_str(), fPipe);
    }
    fflush(fPipe);

    return 1;
}

int KPOutputPipeObject::PrintLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    Print(ArgumentList, ReturnValue);
    fputc('\n', fPipe);
    
    return 1;
}



KPFormatterObject::KPFormatterObject()
: KPObjectPrototype("Formatter")
{
    fFormatStream = new ostringstream();
}

KPFormatterObject::~KPFormatterObject()
{
    delete fFormatStream;
}

KPObjectPrototype* KPFormatterObject::Clone()
{
    return new KPFormatterObject();
}

int KPFormatterObject::MethodIdOf(const std::string& MethodName)
{
    if (MethodName == "put") {
        return MethodId_Put;
    }
    else if (MethodName == "flush") {
        return MethodId_Flush;
    }
    else if ((MethodName == "setWidth") || (MethodName == "setw")) {
        return MethodId_SetWidth;
    }
    else if ((MethodName == "setPrecision") || (MethodName == "setprecision")) {
        return MethodId_SetPrecision;
    }
    else if ((MethodName == "setFill") || (MethodName == "setfill")) {
        return MethodId_SetFill;
    }
    else if ((MethodName == "setBase") || (MethodName == "setbase")) {
        return MethodId_SetBase;
    }
    else if (MethodName == "hex") {
        return MethodId_Hex;
    }
    else if (MethodName == "dec") {
        return MethodId_Dec;
    }
    else if (MethodName == "fixed") {
        return MethodId_Fixed;
    }
    else if (MethodName == "scientific") {
        return MethodId_Scientific;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPFormatterObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;
    
    switch (MethodId) {
      case MethodId_Put:
        Result = Put(ArgumentList, ReturnValue);
	break;
      case MethodId_Flush:
        Result = Flush(ArgumentList, ReturnValue);
	break;
      case MethodId_SetWidth:
        Result = SetWidth(ArgumentList, ReturnValue);
	break;
      case MethodId_SetPrecision:
        Result = SetPrecision(ArgumentList, ReturnValue);
	break;
      case MethodId_SetFill:
        Result = SetFill(ArgumentList, ReturnValue);
	break;
      case MethodId_SetBase:
        Result = SetBase(ArgumentList, ReturnValue);
	break;
      case MethodId_Hex:
        Result = Hex(ArgumentList, ReturnValue);
	break;
      case MethodId_Dec:
        Result = Dec(ArgumentList, ReturnValue);
	break;
      case MethodId_Fixed:
        Result = Fixed(ArgumentList, ReturnValue);
	break;
      case MethodId_Scientific:
        Result = Scientific(ArgumentList, ReturnValue);
	break;
      default:
	break;
    }

    return Result;
}

int KPFormatterObject::Flush(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    string Value = fFormatStream->str(); 

    delete fFormatStream;
    fFormatStream = new ostringstream();

    ReturnValue = KPValue(Value);

    return 1;
}

int KPFormatterObject::Put(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    for (unsigned i = 0; i < ArgumentList.size(); i++) {
	if (ArgumentList[i]->IsLong()) {
	    (*fFormatStream) << ArgumentList[i]->AsLong();
	}
	else if (ArgumentList[i]->IsDouble()) {
	    (*fFormatStream) << ArgumentList[i]->AsDouble();
	}
	else if (ArgumentList[i]->IsComplex()) {
	    complex<double> Value = ArgumentList[i]->AsComplex();
	    double Real = Value.real(), Imag = Value.imag();
#if 1
	    double R = abs(Value);
	    if (fabs(Real / R) < 1e-15) { Real = 0; }
	    if (fabs(Imag / R) < 1e-15) { Imag = 0; }
#endif
	    if (Real != 0) {
		(*fFormatStream) << Real;
	    }
	    if (Imag < 0) {
		(*fFormatStream) << "-";
	    }
	    else if (Real != 0) {
		(*fFormatStream) << "+";
	    }
	    (*fFormatStream) << fabs(Imag) << "i";
	}
	else {
	    (*fFormatStream) << ArgumentList[i]->AsString();
	}	
    }
    
    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::SetWidth(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "setWidth(): incorrect argument";
    }

    int Width = ArgumentList[0]->AsLong();
    (*fFormatStream) << setw(Width);

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::SetPrecision(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "setPrecision(): incorrect argument";
    }

    int Precision = ArgumentList[0]->AsLong();
    (*fFormatStream) << setprecision(Precision);

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::SetFill(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "setFill(): incorrect argument";
    }

    char Fill = ArgumentList[0]->AsString()[0];
    (*fFormatStream) << setfill(Fill);

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::SetBase(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "setBase(): incorrect argument";
    }

    int Base = ArgumentList[0]->AsLong();
    if (Base == 10) {
	(*fFormatStream) << dec;
    }
    else if (Base == 16) {
	(*fFormatStream) << hex;
    }
    else {
	(*fFormatStream) << setbase(Base);
    }

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::Hex(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    (*fFormatStream) << hex;

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::Dec(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    (*fFormatStream) << dec;

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::Fixed(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    fFormatStream->setf(ios::fixed);

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPFormatterObject::Scientific(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    fFormatStream->setf(ios::scientific);

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}



KPScannerObject::KPScannerObject()
: KPObjectPrototype("Scanner")
{
    fSourceStream = nullptr;
}

KPScannerObject::~KPScannerObject()
{
    delete fSourceStream;
}

KPObjectPrototype* KPScannerObject::Clone()
{
    return new KPScannerObject();
}

void KPScannerObject::Construct(const string& ClassName, vector<KPValue*>& ArgumentList) 
{
    if (ArgumentList.size() > 0) {
	if (! ArgumentList[0]->IsString()) {
	    throw KPException() << fInternalClassName << "::" << fInternalClassName << "(): invalid argumrnt[s]";
	}
	
	KPValue ReturnValue;
	Load(ArgumentList, ReturnValue);
    }
}

int KPScannerObject::MethodIdOf(const string& MethodName)
{
    if (MethodName == "load") {
        return MethodId_Load;
    }
    else if (MethodName == "get") {
        return MethodId_Get;
    }
    else if (MethodName == "getLine") {
        return MethodId_GetLine;
    }
    else if ((MethodName == "skipWhiteSpace") || (MethodName == "skipws")) {
        return MethodId_SkipWhiteSpace;
    }
    else if ((MethodName == "setBase") || (MethodName == "setbase")) {
        return MethodId_SetBase;
    }
    else if ((MethodName == "isGood") || (MethodName == "good")) {
        return MethodId_IsGood;
    }
    else if ((MethodName == "LastGetCount") || (MethodName == "gcount")) {
        return MethodId_LastGetCount;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPScannerObject::InvokeMethod(int MethodId, vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;
    
    switch (MethodId) {
      case MethodId_Load:
        Result = Load(ArgumentList, ReturnValue);
	break;
      case MethodId_Get:
        Result = Get(ArgumentList, ReturnValue);
	break;
      case MethodId_GetLine:
        Result = GetLine(ArgumentList, ReturnValue);
	break;
      case MethodId_SkipWhiteSpace:
        Result = SkipWhiteSpace(ArgumentList, ReturnValue);
	break;
      case MethodId_SetBase:
        Result = SetBase(ArgumentList, ReturnValue);
	break;
      case MethodId_IsGood:
        Result = IsGood(ArgumentList, ReturnValue);
	break;
      case MethodId_LastGetCount:
        Result = LastGetCount(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPScannerObject::Load(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << fInternalClassName << "::load(): too few argumrnt[s]";
    }
    if (! ArgumentList[0]->IsString()) {
	throw KPException() << fInternalClassName << "::load(): invalid argumrnt[s]";
    }

    delete fSourceStream;
    fSourceStream = new istringstream(ArgumentList[0]->AsString());

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPScannerObject::Get(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (fSourceStream == nullptr) {
	throw KPException() << fInternalClassName << "::get(): empty buffer";
    }

    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsLeftValue())) {
	throw KPException() << fInternalClassName + "::get(...): invalid argument";
    }

    if (ArgumentList[0]->IsVariant() || ArgumentList[0]->IsString()) {
	string Value;
	(*fSourceStream) >> Value;
	ArgumentList[0]->Assign(KPValue(Value));
    }
    else if (ArgumentList[0]->IsLong()) {
	long Value;
	(*fSourceStream) >> Value;
	ArgumentList[0]->Assign(KPValue(Value));
    }
    else if (ArgumentList[0]->IsDouble()) {
	double Value;
	(*fSourceStream) >> Value;
	ArgumentList[0]->Assign(KPValue(Value));
    }
    else if (ArgumentList[0]->IsComplex()) {
	string Value;
	ArgumentList[0]->Assign(KPValue(Value));
    }
    else {
	throw KPException() << fInternalClassName << "::get(...): invalid argument type";
    }

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPScannerObject::GetLine(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (fSourceStream == nullptr) {
	throw KPException() << fInternalClassName << "::getLine(string line): empty buffer";
    }

    if ((ArgumentList.size() < 1) || (! ArgumentList[0]->IsLeftValue())) {
	throw KPException() << fInternalClassName << "::getLine(string line): invalid argument";
    }

    char LineTerminator = '\n';
    if (ArgumentList.size() > 1) {
	LineTerminator = ArgumentList[1]->AsString()[0];
    }

    if (ArgumentList[0]->IsVariant() || ArgumentList[0]->IsString()) {
	string Value;
	getline((*fSourceStream), Value, LineTerminator);
	ArgumentList[0]->Assign(KPValue(Value));
    }
    else {
	throw KPException() << fInternalClassName << "::get(string line): invalid argument type";
    }

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPScannerObject::SkipWhiteSpace(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    (*fSourceStream) >> ws;

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPScannerObject::SetBase(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "setBase(): incorrect argument";
    }

    int Base = ArgumentList[0]->AsLong();
    if (Base == 10) {
	(*fSourceStream) >> dec;
    }
    else if (Base == 16) {
	(*fSourceStream) >> hex;
    }
    else {
	(*fSourceStream) >> setbase(Base);
    }

    ReturnValue = KPValue((KPObjectPrototype*) this);

    return 1;
}

int KPScannerObject::IsGood(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    ReturnValue = KPValue((long) fSourceStream->good());

    return 1;
}

int KPScannerObject::LastGetCount(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    ReturnValue = KPValue((long) fSourceStream->gcount());

    return 1;
}



KPArgumentObject::KPArgumentObject(int argc, char** argv)
: KPObjectPrototype("Argument")
{
    fArgc = argc;
    fArgv = argv;

    fIsParsed = false;
}

KPArgumentObject::~KPArgumentObject()
{
}

KPObjectPrototype* KPArgumentObject::Clone()
{
    return new KPArgumentObject(fArgc, fArgv);
}

int KPArgumentObject::MethodIdOf(const string& MethodName)
{
    if (MethodName == "numberOfArguments") {
        return MethodId_NumberOfArguments;
    }
    else if (MethodName == "getArgumentOf") {
        return MethodId_GetArgumentOf;
    }
    else if (MethodName == "numberOfParameters") {
        return MethodId_NumberOfParameters;
    }
    else if (MethodName == "getParameterOf") {
        return MethodId_GetParameterOf;
    }
    else if (MethodName == "isOptionSpecified") {
        return MethodId_IsOptionSpecified;
    }
    else if (MethodName == "getOptionValueOf") {
        return MethodId_GetOptionValueOf;
    }
    else if (MethodName == "isSwitchSpecified") {
        return MethodId_IsSwitchSpecified;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPArgumentObject::InvokeMethod(int MethodId, vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (! fIsParsed) {
	Parse();
    }

    int Result = 0;
    
    switch (MethodId) {
      case MethodId_NumberOfArguments:
        Result = NumberOfArguments(ArgumentList, ReturnValue);
	break;
      case MethodId_GetArgumentOf:
        Result = GetArgumentOf(ArgumentList, ReturnValue);
	break;
      case MethodId_NumberOfParameters:
        Result = NumberOfParameters(ArgumentList, ReturnValue);
	break;
      case MethodId_GetParameterOf:
        Result = GetParameterOf(ArgumentList, ReturnValue);
	break;
      case MethodId_IsOptionSpecified:
        Result = IsOptionSpecified(ArgumentList, ReturnValue);
	break;
      case MethodId_GetOptionValueOf:
        Result = GetOptionValueOf(ArgumentList, ReturnValue);
	break;
      case MethodId_IsSwitchSpecified:
        Result = IsSwitchSpecified(ArgumentList, ReturnValue);
	break;
      default:
	return KPObjectPrototype::InvokeMethod(MethodId, ArgumentList, ReturnValue);
    }
    
    return Result;
}

int KPArgumentObject::NumberOfArguments(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    ReturnValue = KPValue((long) fArgumentList.size());

    return 1;
}

int KPArgumentObject::GetArgumentOf(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "getArgumentOf(int index): incorrect argument";
    }

    int Index = ArgumentList[0]->AsLong();
    string Argument;
    if (Index < (int) fArgumentList.size()) {
	Argument = fArgumentList[Index];
    }

    ReturnValue = KPValue(Argument);

    return 1;
}

int KPArgumentObject::IsOptionSpecified(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if ((ArgumentList.size() != 1) && (ArgumentList.size() != 2)) {
        throw KPException() << "isOptionSpecified(): incorrect argument";
    }

    string Name = ArgumentList[0]->AsString();
    auto Result = (long) (fOptionTable.count(Name) > 0);

    if ((! Result) && (ArgumentList.size() > 1)) {
	string Switch = ArgumentList[1]->AsString();
	if (Switch.size() != 1) {
	    throw KPException() << "isOptionSpecified(): incorrect argument";
	}
	Result = (long) (fSwitchSet.count(Switch[0]) > 0);
    }

    ReturnValue = KPValue(Result);

    return 1;
}

int KPArgumentObject::GetOptionValueOf(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "getOptionValueOf(): incorrect argument";
    }

    string Name = ArgumentList[0]->AsString();
    string Value;
    if (fOptionTable.count(Name) > 0) {
	Value = fOptionTable[Name];
    }

    ReturnValue = KPValue(Value);

    return 1;
}

int KPArgumentObject::IsSwitchSpecified(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "isSwitchSpecified(): incorrect argument";
    }

    string Switch = ArgumentList[0]->AsString();
    if (Switch.size() != 1) {
        throw KPException() << "isSwitchSpecified(): incorrect argument";
    }

    ReturnValue = KPValue((long) (fSwitchSet.count(Switch[0]) > 0));

    return 1;
}

int KPArgumentObject::NumberOfParameters(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    ReturnValue = KPValue((long) fParameterList.size());

    return 1;
}

int KPArgumentObject::GetParameterOf(vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    if (ArgumentList.size() != 1) {
        throw KPException() << "getParameterOf(int index): incorrect argument";
    }

    int Index = ArgumentList[0]->AsLong();
    string Parameter;
    if (Index < (int) fParameterList.size()) {
	Parameter = fParameterList[Index];
    }

    ReturnValue = KPValue(Parameter);

    return 1;
}

void KPArgumentObject::Parse()
{
    if (fArgc > 0) {
	fArgumentList.push_back(fArgv[0]);
    }

    for (int i = 1; i < fArgc; i++) {
	string Argument = fArgv[i];
	fArgumentList.push_back(Argument);

	if ((Argument[0] != '-') || (Argument == "-") || (Argument == "--")) {
	    fParameterList.push_back(Argument);
	}
	else if ((Argument.size() > 1) && (isdigit(Argument[1]))) {
	    // negative number parameter //
	    fParameterList.push_back(Argument);
	}
	else {
	    string::size_type NameLength = Argument.find_first_of('=');
	    string Name = Argument.substr(0, NameLength);
	    string Value = "";

	    if (NameLength != string::npos) {
		Value = Argument.substr(NameLength + 1, Argument.size());
	    }

	    if (Value.empty() && (i+1 < fArgc)) {
		if (NameLength != string::npos) {
		    // already contains the assign operator// 
		    i++;
		    Value = fArgv[i];
		}
		else if (fArgv[i+1][0] == '=') {
		    i++;
		    if (fArgv[i][1] != '\0') {
			Value = (fArgv[i] + 1);
		    }
		    else if (i+1 < fArgc) {
			i++;
			Value = fArgv[i];
		    }
		}
	    }

	    fOptionTable[Name] = Value;

	    if ((Argument.size() > 1) && (Argument[1] != '-')) {
		for (unsigned j = 1; j < Argument.size(); j++) {
		    fSwitchSet.insert(Argument[j]);
		}
	    }
	}
    }

    fIsParsed = true;
}



KPStringObject::KPStringObject()
: KPObjectPrototype("String")
{
}

KPStringObject::~KPStringObject()
{
}

KPObjectPrototype* KPStringObject::Clone()
{
    return new KPStringObject();
}

int KPStringObject::MethodIdOf(const std::string& MethodName)
{
    if (MethodName == "chop") {
        return MethodId_Chop;
    }
    else if (MethodName == "chomp") {
        return MethodId_Chomp;
    }
    else if (MethodName == "substr") {
        return MethodId_Substr;
    }
    else if (MethodName == "index") {
        return MethodId_Index;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPStringObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;

    switch (MethodId) {
      case MethodId_Chop:
	Result = Chop(ArgumentList, ReturnValue);
	break;
      case MethodId_Chomp:
	Result = Chomp(ArgumentList, ReturnValue);
	break;
      case MethodId_Substr:
	Result = Substr(ArgumentList, ReturnValue);
	break;
      case MethodId_Index:
	Result = Index(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPStringObject::Chop(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsString())) {
	throw KPException() << "chop(string line): invalid argument[s]";
    }

    string& Line = ArgumentList[0]->AsStringReference();
    string LastCharacter;
    if (! Line.empty()) {
	LastCharacter = Line[Line.size() - 1];
	Line.erase(Line.end() - 1);
    }

    Result = KPValue(LastCharacter);

    return 1;
}

int KPStringObject::Chomp(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if ((ArgumentList.size() != 1) || (! ArgumentList[0]->IsString())) {
	throw KPException() << "chomp(string line): invalid argument[s]";
    }

    string& Line = ArgumentList[0]->AsStringReference();
    long NumberOfRemoved = 0;
    while ((! Line.empty()) && (*(Line.end()-1) == '\n')) {
	Line.erase(Line.end() - 1);
	NumberOfRemoved++;
    }

    Result = KPValue(NumberOfRemoved);

    return 1;
}

int KPStringObject::Substr(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if (
	(ArgumentList.size() < 2) || 
	(! ArgumentList[0]->IsString()) || 
	(! ArgumentList[1]->IsLong())
    ){
	throw KPException() << "substr(string line, int offset, int length=-1): invalid argument[s]";
    }
    string Line = ArgumentList[0]->AsString();

    int Offset = ArgumentList[1]->AsLong();
    if (Offset < 0) {
	Offset = Line.size() + Offset;
    }

    long Length;
    if (ArgumentList.size() > 2) {
	Length = ArgumentList[2]->AsLong();
	if (Length < 0) {
	    // perl compatible //
	    Length = max(0L, (long) (Line.size() - Offset + Length));
	}
    }
    else {
	Length = string::npos;
    }

    Result = KPValue(Line.substr(Offset, Length));

    return 1;
}

int KPStringObject::Index(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if (
	(ArgumentList.size() < 2) || 
	(! ArgumentList[0]->IsString()) || 
	(! ArgumentList[1]->IsString())
    ){
	throw KPException() << "substr(string str, string sub_str, int offset=0): invalid argument[s]";
    }
    string Line = ArgumentList[0]->AsString();
    string Substring = ArgumentList[1]->AsString();

    int Offset;
    if (ArgumentList.size() > 2) {
	Offset = ArgumentList[2]->AsLong();
    }
    else {
	Offset = 0;
    }

    string::size_type Position = Line.find(Substring, Offset);

    Result = KPValue((Position != string::npos) ? (long) Position : -1L);

    return 1;
}



KPSystemObject::KPSystemObject()
: KPObjectPrototype("System")
{
}

KPSystemObject::~KPSystemObject()
{
}

KPObjectPrototype* KPSystemObject::Clone()
{
    return new KPSystemObject();
}

int KPSystemObject::MethodIdOf(const std::string& MethodName)
{
    if (MethodName == "system") {
        return MethodId_System;
    }
    else if (MethodName == "shell") {
        return MethodId_Shell;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPSystemObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;

    switch (MethodId) {
      case MethodId_System:
        Result = System(ArgumentList, ReturnValue);
	break;
      case MethodId_Shell:
        Result = Shell(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPSystemObject::System(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << "system(string command, ...): invalid argument[s]";
    }

    string Command = ArgumentList[0]->AsString();
    for (unsigned i = 1; i < ArgumentList.size(); i++) {
	Command += " " + ArgumentList[i]->AsString();
    }

    long ReturnValue = system(Command.c_str());

    Result = KPValue(ReturnValue);

    return 1;
}

int KPSystemObject::Shell(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << "shell(string command, ...): invalid argument[s]";
    }

    string Command = ArgumentList[0]->AsString();
    for (unsigned i = 1; i < ArgumentList.size(); i++) {
	Command += " " + ArgumentList[i]->AsString();
    }

    FILE* Pipe = popen(Command.c_str(), "r");
    if (Pipe == nullptr) {
	throw KPException() << "shell(string command, ...): unable to execute command: " << Command;
    }

    string ReturnValue;
    char Buffer[1024];
    while (fgets(Buffer, sizeof(Buffer), Pipe) != nullptr) {
	ReturnValue += Buffer;
    }

    int ExitStatus = pclose(Pipe);
    if (ExitStatus < 0) {
	throw KPException() << "shell(string command, ...): error status returned: " << Command;
    }

    Result = KPValue(ReturnValue);

    return 1;
}



KPParserObject::KPParserObject(KPParser* Parser)
: KPObjectPrototype("Parser")
{
    fParser = Parser;

    fExpressionParser = nullptr;
    fSymbolTable = nullptr;
}

KPParserObject::~KPParserObject()
{
}

KPObjectPrototype* KPParserObject::Clone()
{
    return new KPParserObject(fParser);
}

int KPParserObject::MethodIdOf(const std::string& MethodName)
{
    if ((MethodName == "evaluate") || (MethodName == "eval")) {
        return MethodId_Evaluate;
    }

    return KPObjectPrototype::MethodIdOf(MethodName);
}

int KPParserObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) 
{
    int Result = 0;

    switch (MethodId) {
      case MethodId_Evaluate:
        Result = Evaluate(ArgumentList, ReturnValue);
	break;
      default:
	Result = 0;
    }
    
    return Result;
}

int KPParserObject::Evaluate(std::vector<KPValue*>& ArgumentList, KPValue& Result) 
{
    if (ArgumentList.size() < 1) {
	throw KPException() << "evaluate(string expression): invalid argument[s]";
    }

    if (fExpressionParser == nullptr) {
	fExpressionParser = fParser->GetExpressionParser();
	fSymbolTable = fParser->GetSymbolTable();
	fTokenTable = fParser->GetTokenTable();
    }

    istringstream InputStream(ArgumentList[0]->AsString());
    KPTokenizer Tokenizer(InputStream, fTokenTable);

    KPExpression* Expression = nullptr;
    try {
	Expression = fExpressionParser->Parse(&Tokenizer, fSymbolTable);
	Result = Expression->Evaluate(fSymbolTable);
    }
    catch (KPException &e) {
	delete Expression;
	throw;
    }
    delete Expression;

    return 1;
}

