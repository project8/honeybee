// KXpvpTreeFormat.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <cctype>
#include <limits>
#include "KTreeSerializer.h"
#include "KXpvpTreeFormat.h"

using namespace std;
using namespace tabree;


KXpvpTreeFormat::KXpvpTreeFormat()
{
}

KXpvpTreeFormat::~KXpvpTreeFormat()
{
}

void KXpvpTreeFormat::Read(KTree& tree, std::istream& input)
{
    auto trimEnd = [](const string& text) {
        int count = 0;
        for (auto iter = text.rbegin(); iter != text.rend() && isspace(*iter); iter++) {
            count++;
        }
        return text.substr(0, text.size() - count);
    };
    
    auto GetUntil = [&](char delim) {
        string token;
        char ch;
        bool isEscaped = false, isQuoted = false, isBeginning = true;
        while (input.get(ch)) {
            if (isBeginning) {
                if (isspace(ch)) {
                    continue;
                }
                isBeginning = false;
            }
            if (isEscaped) {
                token += ch;
                isEscaped = false;
                continue;
            }
            if (ch == '\\') {
                isEscaped = true;
                continue;
            }
            if (ch == '"') {
                isQuoted = ! isQuoted;
            }
            if (! isQuoted && (ch == delim)) {
                if (token.empty()) {
                    token = delim;
                }
                break;
            }
            if ((ch == '\n') || (ch == '\r') || (ch == '\v')) {
                break;
            }
            token += ch;
        }
        return trimEnd(token);
    };

    string path, value, next;
    path = GetUntil(':');
    while (! path.empty()) {
        value = GetUntil(';');
        if (value == ";") {
            value = "";
        }
        while (true) {
            next = GetUntil(':');
            if (next == ":") {
                value += '\n' + GetUntil(';');
            }
            else {
                break;
            }
        };

        this->FillNodeValue(tree[path], value);
        
        path = next;
    }
}

void KXpvpTreeFormat::Write(const KTree& tree, std::ostream& output)
{
    KXpvpTreeSerializer(output).Serialize(tree);
}
