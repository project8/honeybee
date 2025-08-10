// unit-converter.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <tabree/KArgumentList.h>
#include <tabree/KTreeFile.h>

using namespace std;
using namespace tabree;


float Convert(float input_value, KTree unit);
void ShowHelp(KArgumentSchema& argument_schema, KArgumentList& args);


int main(int argc, char** argv)
{
    KArgumentSchema argument_schema;
    argument_schema.Require("VALUE").InTypeOf(0.0);
    argument_schema.Require("UNIT").InTypeOf("")
        .WhichIs("unit defined in the config file");
    argument_schema.Take("--config=FILE").WithDefault("UnitTable.ktf")
        .WhichIs("config file path");
    argument_schema.Take("--help")
        .WhichIs("show this message");

    KArgumentList args(argc, argv);
    if (! args["--help"].IsVoid()) {
        ShowHelp(argument_schema, args);
        return -1;
    }

    try {
        argument_schema.Validate(args);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        cerr << "use \"--help\" to show program arguments" << endl;
        return -1;
    }

    string config_file = args["--config"];
    KTree config;
    try {
        KTreeFile(config_file).Read(config);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    float input_value = args[0];
    string input_unit = args[1];

    try {
        for (unsigned i = 0; i < config["Quantity"].Length(); i++) {
            KTree quantity = config["Quantity"][i];
            for (unsigned j = 0; j < quantity["Unit"].Length(); j++) {
                KTree unit = quantity["Unit"][j];
                if (unit["Symbol"].As<string>() == input_unit) {
                    string name = quantity["Name"];
                    float output_value = Convert(input_value, unit);
                    string output_unit = quantity["Unit[0]/Symbol"];
                    
                    cout << name << ": ";
                    cout << input_value << " " << input_unit << " = ";
                    cout << output_value << " " << output_unit << endl;
                }
            }
        }
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    return 0;
}


float Convert(float input_value, KTree unit)
{
    float offset = unit["Offset"].Or(0);
    float factor = unit["Factor"].Or(1);
    if (unit["Coeff"].Length()) {
        offset = unit["Coeff"][0].Or(0);
        factor = unit["Coeff"][1].Or(1);
    }
    
    return factor * input_value + offset;
}


void ShowHelp(KArgumentSchema& argument_schema, KArgumentList& args)
{
    cerr << "Unit Converter, Version 17.1" << endl;

    string program_name = args.ProgramName();
    cerr << "usage) " << program_name << "  [OPTIONS] PARAMETERS" << endl;
    argument_schema.Print(cerr);

    cerr << endl;
    cerr << "examples) " << endl;
    cerr << "  " << program_name << " 32 inch" << endl;
    cerr << "  " << program_name << " 64 F" << endl;
    cerr << endl;
}
