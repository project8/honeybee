// argument-schema-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <cmath>
#include <tabree/KArgumentList.h>

using namespace std;
using namespace tabree;


void ShowHelp(KArgumentSchema& argument_schema, KArgumentList& args);


int main(int argc, char** argv)
{
    KArgumentSchema argument_schema;
    argument_schema
        .Require("VALUE1").InTypeOf(0.0)
        .WhichIs("first element to add");
    argument_schema
        .Require("VALUE2").InTypeOf(0.0)
        .WhichIs("second element to add");
    argument_schema.
        TakeMultiple("VALUE").InTypeOf(0.0)
        .WhichIs("optional more elements to add");
    argument_schema
        .Take("--help,-h")
        .WhichIs("print this message");
    argument_schema
        .Take("--start=VALUE").WithDefault(0.0)
        .WhichIs("set initial value");

    KArgumentList args(argc, argv);
    if (! args["--help"].IsVoid()) {
        ShowHelp(argument_schema, args);
        return -1;
    }

    try {
        argument_schema.AllowExcess().AllowUnknown().Validate(args);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        cerr << "use \"--help\" to show program arguments" << endl;
        return -1;
    }

    float sum = args["--start"];
    cout << sum;

    // another way to loop over the argument parameters //
    while (args.Length()) {
        float value = args.Pop();
        sum += value;
        cout << " + " << value;
    }

    cout << " = " << sum << endl;

    return 0;
}


void ShowHelp(KArgumentSchema& argument_schema, KArgumentList& args)
{
    cerr << "Sum Calculator, Version 17.1" << endl;
    cerr << "*) reads input data from command argument" << endl;
    cerr << "*) calcurates count, sum, mean and rms" << endl;
    cerr << endl;

    string program_name = args.ProgramName();
    cerr << "usage) " << program_name << "  [OPTIONS] PARAMETERS" << endl;
    argument_schema.Print(cerr);

    cerr << endl;
    cerr << "examples) " << endl;
    cerr << "  " << program_name << " 1 2 3 4 0x10" << endl;
    cerr << "  " << program_name << " --start=10 1 2 3" << endl;
    cerr << "  " << program_name << " --help" << endl;
    cerr << endl;
}
