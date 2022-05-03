// KTabreeFile.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "KTabree.h"
#include "KTabreeFormat.h"
#include "KTabreeFile.h"

using namespace std;
using namespace tabree;


KTabreeFile::KTabreeFile(const std::string& fileName)
{
    fFileName = fileName;

    string fileExtension;
    string::size_type dot = fFileName.find_last_of('.');
    if (dot != string::npos) {
        fileExtension = fFileName.substr(dot, string::npos);
    }

    fFormat = new KTabreeFormat();
    if ((fileExtension == ".csv") || (fileExtension == ".CSV")) {
        fFormat->EnableCsvHeader();
    }
}

KTabreeFile::~KTabreeFile()
{
}

void KTabreeFile::Read(KTabree& tabree) 
{
    ifstream input(fFileName.c_str());
    if (! input) {
        throw KException() << "unable to open tabree file: " << fFileName;
    }

    fFormat->Read(tabree, input);
}

void KTabreeFile::Write(KTabree& tabree) 
{
    ofstream output(fFileName.c_str());
    if (! output) {
        throw KException() << "unable to write tree file: " << fFileName;
    }

    fFormat->Write(tabree, output);
}



KCsvTabreeFile::KCsvTabreeFile(const std::string& fileName)
: KTabreeFile(fileName)
{
    fFormat->EnableCsvHeader();
}



KTabreeEmbedded::KTabreeEmbedded(const char* embeddedTabree)
{
    fEmbeddedTabree = embeddedTabree;
}

KTabreeEmbedded::~KTabreeEmbedded()
{
}

void KTabreeEmbedded::Read(KTabree& tabree) 
{
    istringstream is(fEmbeddedTabree);
    KTabreeFormat().Read(tabree, is);
}
