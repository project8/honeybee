// KInifileTreeFormat.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

//  Restrictions
//  ============
//  To Ini-file:
//      *) the first-level node (=section) cannot have a value
//  Standard:
//      *) Line staring with a semi-colon is a comment line
//         -> line that has a semi-colon before an equal is a comment line
//         -> line that does not have an equal is a comment line
//      *) dot or slash in key name is used as a path separator
//      *) empty lines are allowed
//  Extension to Standard:
//      *) Values are typed (void, int, float, bool, string)
//      *) true, false, null, nan, inf, NaN, and Infinity literals are added
//      *) Semi-colon can be used instead of new-line
//      *) Multiple sections with same name make an array


#include <string>
#include <iostream>
#include <cctype>
#include <limits>
#include "KTreeSerializer.h"
#include "KInifileTreeFormat.h"

using namespace std;
using namespace tabree;


KInifileTreeFormat::KInifileTreeFormat()
{
    fBreakChar = ';';
}

KInifileTreeFormat::~KInifileTreeFormat()
{
}

void KInifileTreeFormat::SetBreakChar(char Char)
{
    fBreakChar = Char;
}

void KInifileTreeFormat::Read(KTree& tree, std::istream& input)
{
    auto trim = [](const string& text) {
        int c0 = 0, c1 = 0;
        for (auto i = text.begin(); i != text.end() && isspace(*i); i++, c0++) {
            ;
        }
        for (auto i = text.rbegin(); i != text.rend() && isspace(*i); i++, c1++) {
            ;
        }
        return text.substr(c0, text.size() - c0 - c1);
    };
    
    KTree* sectionNode = &tree;

    string section, key, value, token;
    enum {
        State_Init, State_Comment, State_Section, State_Key, State_Value
    } state = State_Init;
    bool isEscaped = false, isQuoted = false;
    char ch;
    while (input.get(ch)) {
        if (isEscaped) {
            token += ch;
            isEscaped = false;
            continue;
        }
        if (ch == '\\') {
            isEscaped = true;
            continue;
        }

        if (state == State_Init) {
            if (ch == '[') {
                state = State_Section;
            }
            else if (ch == ';') {
                state = State_Comment;
            }
            else if (isspace(ch)) {
                ;
            }
            else {
                state = State_Key;
                token += ch;
            }
        }
        else if (state == State_Comment) {
            if ((ch == '\n') || (ch == '\r') || (ch == '\v')) {
                state = State_Init;
            }
        }
        else if (state == State_Section) {
            if (ch == ']') {
                section = trim(token);
                token.clear();
                sectionNode = &tree.AppendNode(section);
                state = State_Init;
            }
            else {
                token += ch;
            }
        }
        else if (state == State_Key) {
            if (ch == '=') {
                key = trim(token);
                token.clear();
                state = State_Value;
                isQuoted = false;
            }
            else if (ch == '.') {
                token += '/';
            }
            else if ((ch == '\n') || (ch == '\r') || (ch == '\v') || (ch == ';')) {
                state = State_Comment;
            }
            else {
                token += ch;
            }
        }
        else if (state == State_Value) {
            if (ch == '"') {
                isQuoted = ! isQuoted;
            }
            if (
                ((ch == '\n') || (ch == '\r') || (ch == '\v')) ||
                (! isQuoted && ((ch == ';') || (ch == fBreakChar)))
            ){
                value = trim(token);
                token.clear();
                this->FillNodeValue((*sectionNode)[key], value);
                state = ((ch != fBreakChar) && (ch == ';')) ? State_Comment: State_Init;
            }
            else {
                token += ch;
            }
        }
        else {
            token.clear();
            state = State_Init;
        }
    }
}

void KInifileTreeFormat::Write(const KTree& tree, std::ostream& output)
{
    KInifileTreeSerializer(output).Serialize(tree);
}
