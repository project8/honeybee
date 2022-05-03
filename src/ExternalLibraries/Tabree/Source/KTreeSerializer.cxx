// KTreeSerializer.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <deque>
#include "KTree.h"
#include "KTreeWalker.h"
#include "KTreeSerializer.h"

using namespace std;
using namespace tabree;



KTreeSerializer::KTreeSerializer(std::ostream& Output)
: fOutput(Output)
{
    fPrecision = 9;
}

KTreeSerializer::~KTreeSerializer()
{
}

void KTreeSerializer::SetPrecision(unsigned precision)
{
    fPrecision = precision;
}

void KTreeSerializer::Serialize(const KTree& Tree, const std::string& Name)
{
    KTreeWalker(this).Process(const_cast<KTree&>(Tree), Name);
}



KKtfTreeSerializer::KKtfTreeSerializer(std::ostream& Output)
: KTreeSerializer(Output)
{
    fHeaderChar = '#';
    fIndentStep = 2;

    fDepth = -1;
}

KKtfTreeSerializer::~KKtfTreeSerializer()
{
}

bool KKtfTreeSerializer::StartTree(KTree& Node) 
{
    fDepth = -1;
    return true;
}

bool KKtfTreeSerializer::StartNode(const std::string& Name, KTree& Node, KTree& AttributeList) 
{
    fDepth++;
    if (fDepth <= 0) {
        return true;
    }

    string Indent(fIndentStep * fDepth, ' ');
    Indent[0] = fHeaderChar;

    string EscapedName;
    bool IsQuoteNeeded = false;
    for (unsigned i = 0; i < Name.size(); i++) {
        if ((Name[i] == '"') || (Name[i] == '\'') || (Name[i] == '\\')) {
            EscapedName += '\\';
        }
        else if (! (isalnum(Name[i]) || (Name[i] == '_'))) {
            IsQuoteNeeded = true;
        }
        EscapedName += Name[i];
    }
    if (IsQuoteNeeded) {
        fOutput << Indent << '"' << EscapedName << "\": ";
    }
    else {
        fOutput << Indent << EscapedName << ": ";
    }

    if (Node.Value().IsInteger() || Node.Value().IsBool()) {
        fOutput << Node.Value() << endl;
    }
    else if (Node.Value().IsNumeric()) {
        string text = Node.Value().AsString(fPrecision);
        if (text == KVariant(Node.Value().AsLong()).AsString()) {
            text += ".";
        }
        fOutput << text << endl;
    }
    else {
        istringstream is(Node.As<string>());
        string Line;
        if (getline(is, Line) && (! Line.empty())) {
            bool IsQuoteNeeded = isspace(*Line.begin()) || isspace(*Line.end());
            if (! IsQuoteNeeded) {
                try {
                    Node.Value().AsDouble();
                    IsQuoteNeeded = true;
                }
                catch (KException &e) {
                    ;
                }
            }
            if (IsQuoteNeeded) {
                fOutput << '"' << Line << '"';
            }
            else {
                fOutput << Line;
            }
            fOutput << endl;
        }
        else if (Node.Value().IsString()) {
            fOutput << "\"\"" << endl;
        }
        else {
            fOutput << endl;
        }
        
        while(getline(is, Line)) {
            fOutput << Indent << ": " << Line << endl;
        }
    }

    string AttributeIndent(fIndentStep * (fDepth+1), ' ');
    AttributeIndent[0] = fHeaderChar;
    for (unsigned i = 0; i < AttributeList.ChildNodeList().size(); i++) {
        fOutput << AttributeIndent;
        fOutput << "@" << AttributeList.ChildNodeList()[i]->NodeName() << ": ";
        fOutput << AttributeList.ChildNodeList()[i]->Value() << endl;
    }

    return true;
}

void KKtfTreeSerializer::EndNode(const std::string& Name, KTree& Node) 
{
    fDepth--;
}

bool KKtfTreeSerializer::StartArray(const std::string& Name, KTree& Node) 
{
    if (Node.Length() == 0) {
        string Indent(fIndentStep * (fDepth+1), ' ');
        Indent[0] = fHeaderChar;
        fOutput << Indent << Name << ": []" << endl;
        return false;
    }

    bool IsSimpleArray = true;
    const vector<KTree*>& ChildNodeList = Node.ChildNodeList();
    for (unsigned i = 0; i < ChildNodeList.size(); i++) {
        if (! ChildNodeList[i]->IsLeaf()){
            IsSimpleArray = false;
            break;
        }
    }
    if (IsSimpleArray) {
        string Indent(fIndentStep * (fDepth+1), ' ');
        Indent[0] = fHeaderChar;
        fOutput << Indent << Name << ": [";
        for (unsigned i = 0; i < ChildNodeList.size(); i++) {
            const KVariant& value = ChildNodeList[i]->Value();
            fOutput << ((i == 0) ? " " : ", ");
            if (value.IsString()) {
                fOutput << '"' << value << '"';
            }
            else if (value.IsNumeric() && ! value.IsInteger()) {
                string text = value.AsString(fPrecision);
                if (text == KVariant(value.AsLong()).AsString()) {
                    text += ".";
                }
                fOutput << text;
            }
            else {
                fOutput << value;
            }
        }
        fOutput << " ]" << endl;
        return false;
    }
    
    return true;
}



KJsonTreeSerializer::KJsonTreeSerializer(std::ostream& Output, bool isInline)
: KTreeSerializer(Output), fIsInline(isInline)
{
}

KJsonTreeSerializer::~KJsonTreeSerializer()
{
}

bool KJsonTreeSerializer::StartTree(KTree& Node) 
{
    this->ProcessNode(Node, "", "");
    fIsInline || fOutput << endl;

    return false;
}

void KJsonTreeSerializer::ProcessNode(KTree& Node, std::string NodeName, const std::string& Indent)
{
    //... BUG: if the Node has both value and children, the value is ignored

    (fIsInline || fOutput << Indent);
    if (! NodeName.empty()) {
        fOutput << "\"" << NodeName << "\": ";
    }

    if (Node.IsLeaf()) {
        this->WriteJsonValue(fOutput, Node.Value(), fPrecision);
    }
    else if (this->WriteJsonInlineIfSimple(fOutput, Node, fPrecision)) {
        ;
    }
    else if (Node.IsArray()) {
        unsigned Length = Node.Length();
        fOutput << "[ "; (fIsInline || fOutput << endl);
        for (unsigned i = 0; i < Length; i++) {
            ProcessNode(Node[i], "", Indent + "  ");
            if (i+1 < Length) {
                fOutput << ','; (fIsInline || fOutput << endl);
            }
            else {
                fIsInline || fOutput << endl;
            }
        }
        if (fIsInline) {
            fOutput << " ]";
        }
        else {
            fOutput << Indent << "]";
        }
    }
    else {
        unsigned Size = Node.KeyList().size();
        fOutput << "{ "; fIsInline || fOutput << endl;
        for (unsigned i = 0; i < Size; i++) {
            string Key = Node.KeyList()[i];
            ProcessNode(Node[Key], Key, Indent + "  ");
            if (i+1 < Size) {
                fOutput << ", "; fIsInline || fOutput << endl;
            }
            else {
                fIsInline || fOutput << endl;
            }
        }
        if (fIsInline) {
            fOutput << " }";
        }
        else {
            fOutput << Indent << "}";
        }
    }
}

bool KJsonTreeSerializer::WriteJsonInlineIfSimple(std::ostream& os, KTree& Node, int Precision, int Looseness)
{
    const vector<KTree*>& ChildNodeList = Node.ChildNodeList();
    for (unsigned i = 0; i < ChildNodeList.size(); i++) {
        if (! ChildNodeList[i]->IsLeaf()){
            return false;
        }
    }
    
    if (Node.IsArray()) {
        os << "[";
        for (unsigned i = 0; i < ChildNodeList.size(); i++) {
            os << ((i == 0) ? " " : ", ");
            WriteJsonValue(os, ChildNodeList[i]->Value(), Precision, Looseness);
        }
        os << " ]";
    }
    else {
        if (Node.Length() > 5) {
            return false;
        }
        os << "{";
        for (unsigned i = 0; i < ChildNodeList.size(); i++) {
            string Key = ChildNodeList[i]->NodeName();
            bool IsQuoteNeeded = (Looseness == Looseness_Strict);
            for (auto Ch = Key.begin(); Ch != Key.end(); Ch++) {
                if (! isalnum(*Ch) && (*Ch != '_') && (*Ch != ' ')) {
                    IsQuoteNeeded = true;
                    break;
                }
            }
            os << ((i == 0) ? " " : ", ") << (IsQuoteNeeded ? "\"" : "");
            WriteJsonString(os, Key);
            os << (IsQuoteNeeded ? "\": " : ": ");
            WriteJsonValue(os, ChildNodeList[i]->Value(), Precision, Looseness);
        }
        os << " }";
    }
    
    return true;
}

void KJsonTreeSerializer::WriteJsonValue(std::ostream& os, KVariant& Value, int Precision, int Looseness)
{
    if (Value.IsVoid()) {
        os << "null";
    }
    else if (Value.IsNumeric()) {
        if (! Value.IsInteger()) {
            string text = Value.AsString(Precision);
            if ((text == "nan") || (text == "-nan")) {
                text = "NaN";   // python can read this
            }
            else if (text == "inf") {
                text = "Infinity";
            }
            else if (text == "-inf") {
                text = "-Infinity";
            }
            else if (text == KVariant(Value.AsLong()).AsString()) {
                text += ".0";
            }
            os << text;
        }
        else {
            os << Value.AsLong();
        }
    }
    else if (Value.IsBool()) {
        os << (Value.AsBool() ? "true" : "false");
    }
    else {
        string StringValue = Value.AsString();
        bool IsQuoteNeeded = (Looseness == Looseness_Strict) || StringValue.empty();
        if (Looseness <= Looseness_Inline) {
            for (auto Ch = StringValue.begin(); Ch != StringValue.end(); Ch++) {
                if (! isalnum(*Ch) && (*Ch != '_') && (*Ch != ' ')) {
                    IsQuoteNeeded = true;
                    break;
                }
            }
        }
        try {
            Value.AsDouble();
            IsQuoteNeeded = true;
        }
        catch (KException &e) {
            ;
        }
        try {
            Value.AsBool();
            IsQuoteNeeded = true;
        }
        catch (KException &e) {
            ;
        }
        os << (IsQuoteNeeded ? "\"" : "");
        WriteJsonString(os, StringValue);
        os << (IsQuoteNeeded ? "\"" : "");
    }
}

void KJsonTreeSerializer::WriteJsonString(std::ostream& os, const std::string& Value)
{
    for (string::const_iterator Ch = Value.begin(); Ch != Value.end(); Ch++) {
        switch (*Ch) {
          case '"':
            os << "\\\"";
            break;
          case '\t':
            os << "\\t";
            break;
          case '\v':
            os << "\\v";
            break;
          case '\n':
            os << "\\n";
            break;
          case '\r':
            os << "\\r";
            break;
          case '\f':
            os << "\\f";
            break;
          case '\\':
            os << "\\\\";
            break;
        default:
            os << *Ch;
            break;
        }
    }
}



KYamlTreeSerializer::KYamlTreeSerializer(std::ostream& Output)
: KJsonTreeSerializer(Output)
{
}

KYamlTreeSerializer::~KYamlTreeSerializer()
{
}

bool KYamlTreeSerializer::StartTree(KTree& Node) 
{
    this->ProcessYamlObject(Node, "", false, true);
    fOutput << endl;

    return false;
}

void KYamlTreeSerializer::ProcessYamlNode(KTree& Node, std::string NodeName, const std::string& Indent, const std::string ThisIndent)
{
    if (NodeName.empty()) {
        fOutput << ThisIndent << "- ";
    }
    else {
        string EscapedName;
        bool IsQuoteNeeded = false;
        for (unsigned i = 0; i < NodeName.size(); i++) {
            if ((NodeName[i] == '"') || (NodeName[i] == '\'') || (NodeName[i] == '\\')) {
                EscapedName += '\\';
            }
            else if (NodeName[i] == ':') {
                IsQuoteNeeded = true;
            }
            EscapedName += NodeName[i];
        }
        if (IsQuoteNeeded) {
            fOutput << '"' << ThisIndent << EscapedName << "\": ";
        }
        else {
            fOutput << ThisIndent << EscapedName << ": ";
        }
    }

    ProcessYamlObject(Node, Indent, NodeName.empty());
}
    
void KYamlTreeSerializer::ProcessYamlObject(KTree& Node, const std::string& Indent, bool IsArrayElement, bool IsRoot)
{
    if (Node.IsLeaf()) {
        this->WriteJsonValue(fOutput, Node.Value(), fPrecision, Looseness_Block);
    }
    else if (this->WriteJsonInlineIfSimple(fOutput, Node, fPrecision, Looseness_Inline)) {
        ;
    }
    else if (Node.IsArray()) {
        unsigned Length = Node.Length();
        for (unsigned i = 0; i < Length; i++) {
            fOutput << endl;
            ProcessYamlNode(Node[i], "", Indent + "  ", Indent + "  ");
        }
    }
    else {
         unsigned Size = Node.KeyList().size();
        for (unsigned i = 0; i < Size; i++) {
            string Key = Node.KeyList()[i];
            string NewIndent = (IsRoot ? "" : Indent + "  ");
            string ThisIndent = NewIndent;
            if (IsArrayElement && (i == 0)) {
                ThisIndent = "";
            }
            else if (! IsRoot) {
                fOutput << endl;
            }
            ProcessYamlNode(Node[Key], Key, NewIndent, ThisIndent);
        }
    }
}



KXmlTreeSerializer::KXmlTreeSerializer(std::ostream& Output)
: KTreeSerializer(Output)
{
    fNameForEmptyName = "Node";
}

KXmlTreeSerializer::~KXmlTreeSerializer()
{
}

bool KXmlTreeSerializer::StartTree(KTree& Node) 
{
    fOutput << "<?xml version=\"1.0\"?>" << endl << endl;
    fIndentStack.push_back("");
    return true;
}

bool KXmlTreeSerializer::StartNode(const std::string& Name, KTree& Node, KTree& AttributeList) 
{
    string Indent = fIndentStack.back();
    fIndentStack.push_back(Indent + "  ");

    string ElementName = GetXmlNameOf(Name);
    if (ElementName.empty()) {
        ElementName = fNameForEmptyName;
    }

    fOutput << Indent;
    fOutput << "<" << ElementName;
    for (unsigned i = 0; i < AttributeList.ChildNodeList().size(); i++) {
        fOutput << " ";
        fOutput << GetXmlNameOf(AttributeList.ChildNodeList()[i]->NodeName());
        fOutput << "=\"";
        WriteXmlString(fOutput, AttributeList.ChildNodeList()[i]->As<string>());
        fOutput << "\"";
    }
    fOutput << ">";

    if (! Node.IsVoid()) {
        if (Node.Value().IsNumeric() && ! Node.Value().IsInteger()) {
            fOutput << Node.Value().AsString(fPrecision);
        }
        else {
            WriteXmlString(fOutput, Node.As<string>());
        }
    }
    if (! Node.IsLeaf()) {
        fOutput << endl;
    }
    
    return true;
}

void KXmlTreeSerializer::EndNode(const std::string& Name, KTree& Node) 
{
    fIndentStack.pop_back();
    if (! Node.IsLeaf()) {
        fOutput << fIndentStack.back();
    }

    string ElementName = GetXmlNameOf(Name);
    if (ElementName.empty()) {
        ElementName = fNameForEmptyName;
    }

    fOutput << "</" << ElementName << ">";
    fOutput << endl;
}

bool KXmlTreeSerializer::StartArray(const std::string& Name, KTree& Node) 
{
    if (Node.Length() == 0) {
        // empty array using inline JSON //
        string Indent = fIndentStack.back() + "  ";
        fOutput << Indent;
        fOutput << "<" << Name << ">[]</" << Name << ">" << endl;
        return false;
    }

    return true;
}

void KXmlTreeSerializer::WriteXmlString(std::ostream& os, const std::string& Value)
{
    for (string::const_iterator Ch = Value.begin(); Ch != Value.end(); Ch++) {
        switch (*Ch) {
          case '<':
            os << "&lt;";
            break;
          case '>':
            os << "&gt;";
            break;
          case '&':
            os << "&amp;";
            break;
          case '"':
            os << "&quot;";
            break;
          case '\'':
            os << "&apos;";
            break;
          default:
            os << *Ch;
            break;
        }
    }
}

string KXmlTreeSerializer::GetXmlNameOf(const std::string& Name)
{
    string XmlName;
    for (unsigned i = 0; i < Name.size(); i++) {
        if ((i == 0) && (isdigit(Name[i]))) {
            XmlName += '_';
        }
        if (! (isalnum(Name[i]))) {
            XmlName += '_';
        }
        else {
            XmlName += Name[i];
        }
    }
    
    return XmlName;
}


KPlistTreeSerializer::KPlistTreeSerializer(std::ostream& Output)
: KTreeSerializer(Output)
{
}

KPlistTreeSerializer::~KPlistTreeSerializer()
{
}

bool KPlistTreeSerializer::StartTree(KTree& Node) 
{
    fOutput << R"(<?xml version="1.0" encoding="UTF-8"?>)" << endl;
    fOutput << R"(<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">)" << endl;

    fOutput << "<plist version=\"1.0\">" << endl;
    this->ProcessNode(Node, "", "");
    fOutput << "</plist>" << endl;

    return false;
}

void KPlistTreeSerializer::ProcessNode(KTree& Node, std::string NodeName, const std::string& Indent)
{
    //... BUG: if the Node has both value and children, the value is ignored

    if (! NodeName.empty()) {
        fOutput << Indent << "<key>" << NodeName << "<key>" << endl;
    }

    if (Node.IsLeaf()) {
        fOutput << Indent;
        if (Node.IsVoid()) {
            fOutput << "<null/>";
        }
        else if (Node.Value().IsBool()) {
            fOutput << (Node.As<bool>() ? "<true/>" : "<false/>");
        }
        else if (Node.Value().IsInteger()) {
            fOutput << "<integer>" << Node << "</integer>";
        }
        else if (Node.Value().IsNumeric()) {
            std::streamsize precision = fOutput.precision(fPrecision);
            fOutput << "<real>";
            fOutput << Node.Value().AsDouble();
            fOutput.precision(precision);
            fOutput << "</real>";
        }
        else {
            fOutput << "<string>";
            this->WritePlistString(fOutput, Node.As<string>());
            fOutput << "</string>";
        }
        fOutput << endl;
    }
    else if (Node.IsArray()) {
        fOutput << Indent << "<array>" << endl;
        for (unsigned i = 0; i < Node.Length(); i++) {
            ProcessNode(Node[i], "", Indent + "    ");
        }
        fOutput << Indent << "</array>" << endl;
    }
    else {
        unsigned Size = Node.KeyList().size();
        fOutput << Indent << "<dict>" << endl;
        for (unsigned i = 0; i < Size; i++) {
            string Key = Node.KeyList()[i];
            ProcessNode(Node[Key], Key, Indent + "    ");
        }
        fOutput << Indent << "</dict>" << endl;
    }
}

void KPlistTreeSerializer::WritePlistString(std::ostream& os, const std::string& Value)
{
    for (string::const_iterator Ch = Value.begin(); Ch != Value.end(); Ch++) {
        switch (*Ch) {
          case '<':
            os << "&lt;";
            break;
          case '>':
            os << "&gt;";
            break;
          case '&':
            os << "&amp;";
            break;
          case '"':
            os << "&quot;";
            break;
          case '\'':
            os << "&apos;";
            break;
          default:
            os << *Ch;
            break;
        }
    }
}



KInifileTreeSerializer::KInifileTreeSerializer(std::ostream& Output)
: KTreeSerializer(Output)
{
}

KInifileTreeSerializer::~KInifileTreeSerializer()
{
}

bool KInifileTreeSerializer::StartTree(KTree& Node)
{
    for (string Key: Node.KeyList()) {
        auto& Child = Node[Key];
        unsigned n = Child.IsArray() ? Child.Length() : 1;
        for (unsigned i = 0; i < n; i++) {
            fOutput << endl;
            fOutput << "[" << Key << "]" << endl;
            ProcessChildren("", Child[i]);
        }
    }

    return false;
}

void KInifileTreeSerializer::ProcessChildren(const std::string& Path, KTree& Node)
{
    for (string Key: Node.KeyList()) {
        auto& Child = Node[Key];
        string Name = Path.empty() ? Key : Path + "." + Key;
        unsigned n = Child.IsArray() ? Child.Length() : 1;
        for (unsigned i = 0; i < n; i++) {
            string ThisName = Name;
            if (Child.IsArray()) {
                ThisName = Name + "[" + KVariant(i).As<string>() + "]";
            }
            WriteNode(ThisName, Child[i]);
            ProcessChildren(ThisName, Child[i]);
        }
    }
}

void KInifileTreeSerializer::WriteNode(const std::string& Path, KTree& Node)
{
    if (Node.IsVoid()) {
        return;
    }
    fOutput << Path << " = ";

    if (Node.Value().IsInteger() || Node.Value().IsBool()) {
        fOutput << Node.Value();
    }
    else if (Node.Value().IsNumeric()) {
        string text = Node.Value().AsString(fPrecision);
        if (text == KVariant(Node.Value().AsLong()).AsString()) {
            text += ".";
        }
        fOutput << text;
    }
    else {
        istringstream is(Node.As<string>());
        string Line;
        auto Escape = [](const string& Text, bool IsQuoted) {
            string Escaped;
            for (auto& ch: Text) {
                if (
                    (ch == '\\') || (ch == '"') ||
                    ((! IsQuoted) && (ch == ';'))
                ){
                    Escaped += '\\';
                }
                Escaped += ch;
            }
            return Escaped;
        };
        if (getline(is, Line) && (! Line.empty())) {
            bool IsQuoteNeeded = isspace(*Line.begin()) || isspace(*Line.end());
            if (! IsQuoteNeeded) {
                try {
                    Node.Value().AsDouble();
                    IsQuoteNeeded = true;
                }
                catch (KException &e) {
                    ;
                }
            }
            if (IsQuoteNeeded) {
                fOutput << '"' << Escape(Line, IsQuoteNeeded) << '"';
            }
            else {
                fOutput << Escape(Line, IsQuoteNeeded);
            }
        }
        else if (Node.Value().IsString()) {
            fOutput << "\"\"";
        }
        while(getline(is, Line)) {
            fOutput << endl  << ": " << Escape(Line, false);
        }
    }
    fOutput << ';' << endl;
}



KXpvpTreeSerializer::KXpvpTreeSerializer(std::ostream& Output)
: KTreeSerializer(Output)
{
}

KXpvpTreeSerializer::~KXpvpTreeSerializer()
{
}

bool KXpvpTreeSerializer::StartNode(const std::string& Name, KTree& Node, KTree& AttributeList)
{
    string delimiter = ""; // ";";
    
    if (Node.IsVoid()) {
        return true;
    }
    fOutput << Node.NodePath() << ": ";
    
    if (Node.Value().IsInteger() || Node.Value().IsBool()) {
        fOutput << Node.Value() << delimiter << endl;
    }
    else if (Node.Value().IsNumeric()) {
        string text = Node.Value().AsString(fPrecision);
        if (text == KVariant(Node.Value().AsLong()).AsString()) {
            text += ".";
        }
        fOutput << text << delimiter << endl;
    }
    else {
        istringstream is(Node.As<string>());
        string Line;
        auto Escape = [](const string& Text, bool IsQuoted) {
            string Escaped;
            for (auto& ch: Text) {
                if (
                    (ch == '\\') || (ch == '"') ||
                    ((! IsQuoted) && (ch == ';'))
                ){
                    Escaped += '\\';
                }
                Escaped += ch;
            }
            return Escaped;
        };
        if (getline(is, Line) && (! Line.empty())) {
            bool IsQuoteNeeded = isspace(*Line.begin()) || isspace(*Line.end());
            if (! IsQuoteNeeded) {
                try {
                    Node.Value().AsDouble();
                    IsQuoteNeeded = true;
                }
                catch (KException &e) {
                    ;
                }
            }
            if (IsQuoteNeeded) {
                fOutput << '"' << Escape(Line, IsQuoteNeeded) << '"' << delimiter << endl;
            }
            else {
                fOutput << Escape(Line, IsQuoteNeeded) << delimiter << endl;
            }
        }
        else if (Node.Value().IsString()) {
            fOutput << "\"\"" << delimiter << endl;
        }
        while(getline(is, Line)) {
            fOutput << ": " << Escape(Line, false) << delimiter << endl;
        }
    }

    for (unsigned i = 0; i < AttributeList.ChildNodeList().size(); i++) {
        fOutput << Node.NodePath() << "@";
        fOutput << AttributeList.ChildNodeList()[i]->NodeName() << ": ";
        fOutput << AttributeList.ChildNodeList()[i]->Value() << delimiter << endl;
    }

    return true;
}
