// argument-list-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <cmath>
#include <tabree/KArgumentList.h>

using namespace std;
using namespace tabree;


void ShowHelp(const string& program_name);
void ShowUsage(const string& program_name);


int main(int argc, char** argv)
{
    KArgumentList args(argc, argv);
    //args.Dump();

    if (! args["--help"].IsVoid()) {
        ShowHelp(args.ProgramName());
        return -1;
    }
    if (args[1].IsVoid()) {
        // too few argument parameters //
        ShowUsage(args.ProgramName());
        return -1;
    }

    float sum = args["--start"].Or(0).As<float>();
    cout << sum;

    try {
        for (unsigned i = 0; i < args.Length(); i++) {
            float value = args[i];
            sum += value;
            cout << " + " << value;
        }
    }
    catch (KException &e) {
        ShowUsage(args.ProgramName());
        return -1;
    }

    cout << " = " << sum << endl;

    return 0;
}


void ShowHelp(const string& program_name)
{
    cerr << "Sum Calculator, Version 17.1" << endl;
    cerr << "*) reads input data from command argument" << endl;
    cerr << "*) calcurates count, sum, mean and rms" << endl;
    cerr << endl;

    ShowUsage(program_name);
}


void ShowUsage(const string& program_name)
{
    cerr << "usage): " << program_name << " ";
    cerr << "[ --start=VALUE ] [ --help ] ";
    cerr << "VALUE VALUE [ VALUE [ VALUE [ ... ]]]" << endl;
    cerr << "examples) " << endl;
    cerr << "  " << program_name << " 1 2 3 4 5" << endl;
    cerr << "  " << program_name << " --start=10 1 2 3" << endl;
    cerr << "  " << program_name << " --help" << endl;
}
