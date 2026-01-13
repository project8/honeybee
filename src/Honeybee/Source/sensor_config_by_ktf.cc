/*
 * sensor_config_by_ktf.cc
 */

#include <iostream>
#include <kebap/Kebap.h>
#include <tabree/KTreeFile.h>
#include "sensor_config_by_ktf.hh"
#include "sensor_table.hh"
#include "kebap_calibration.hh"

using namespace std;
using namespace honeybee;

sensor_config_by_ktf::sensor_config_by_ktf()
    : f_parser(nullptr)
{
}

sensor_config_by_ktf::~sensor_config_by_ktf()
{
}

void sensor_config_by_ktf::set_variables(const sensor_config_by_ktf::variables& a_variables)
{
    f_variables.insert(f_variables.end(), a_variables.begin(), a_variables.end());
}

void sensor_config_by_ktf::load(sensor_table& a_table, const string& a_filename)
{
    f_ktf_path = a_filename;
    
    // Read KTF file
    tabree::KTree t_tree;
    try {
        tabree::KTreeFile(a_filename).Read(t_tree);
    }
    catch (tabree::KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return;
    }
    
    // Extract and compile scripts
    string t_scripts = extract_scripts();
    if (!t_scripts.empty()) {
        try {
            f_parser = make_shared<kebap::KPParser>();
            kebap::KPTokenizer t_tokenizer;
            kebap::KPInputBuffer t_input(t_scripts);
            t_tokenizer.Scan(t_input);
            f_parser->Parse(&t_tokenizer);
        }
        catch (kebap::KPException &e) {
            cerr << "ERROR: Failed to parse Kebap scripts: " << e.what() << endl;
            f_parser = nullptr;
            return;
        }
    }
    
    // Load sensor hierarchy
    load_layer(t_tree["sensor_table"], a_table);
}

string sensor_config_by_ktf::extract_scripts()
{
    // TODO: Extract <kebap_script> nodes from KTF file
    return "";
}

void sensor_config_by_ktf::load_layer(const tabree::KVariant& a_node, sensor_table& a_table)
{
    // TODO: Recursively traverse sensor hierarchy (experiment/setup/system/...)
}

void sensor_config_by_ktf::add_sensor(const tabree::KVariant& a_node, sensor_table& a_table, int a_line_offset)
{
    // TODO: Parse <channel> node, create sensor with kebap_calibration if f_parser available
}
