// KTreeFile.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <fstream>
#include "KTree.h"
#include "KTreeFormat.h"
#include "KTreeFile.h"

#include "KKtfTreeFormat.h"
#include "KJsonTreeFormat.h"
#include "KInifileTreeFormat.h"
#include "KXpvpTreeFormat.h"

using namespace std;
using namespace tabree;


KTreeFile::KTreeFile(const std::string& fileName)
{
    fFileName = fileName;

    string fileExtension;
    string::size_type dot = fFileName.find_last_of('.');
    if (dot != string::npos) {
        fileExtension = fFileName.substr(dot, string::npos);
    }

    if (fileExtension == ".json") {
        fFormat = new KJsonTreeFormat();
    }
    else if (fileExtension == ".ini") {
        fFormat = new KInifileTreeFormat();
    }
    else if (fileExtension == ".ktf") {
        fFormat = new KKtfTreeFormat();
    }
    else if (fileExtension == ".xpvp") {
        fFormat = new KXpvpTreeFormat();
    }
    else {
        fFormat = nullptr;
    }
}

KTreeFile::~KTreeFile()
{
}

void KTreeFile::Read(KTree& tree) 
{
    if (fFileName.substr(0, 2) == "|.") {
        fFormat->Read(tree, cin);
    }
    else {
        ifstream input(fFileName.c_str());
        if (! input) {
            throw KException() << "unable to open tree file: " << fFileName;
        }
        if (fFormat == nullptr) {
            throw KException() << "unknown tree file type: " << fFileName;
        }
        fFormat->Read(tree, input);
    }
}

void KTreeFile::Write(const KTree& tree) 
{
    if (fFileName.substr(0, 2) == "|.") {
        fFormat->Write(tree, cout);
    }
    else {
        ofstream output(fFileName.c_str());
        if (! output) {
            throw KException() << "unable to write tree file: " << fFileName;
        }
        if (fFormat == nullptr) {
            throw KException() << "unknown tree file type: " << fFileName;
        }
        fFormat->Write(tree, output);
    }
}
