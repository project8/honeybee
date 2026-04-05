/*
 * sensor_config_by_ktf.cc
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <kebap/Kebap.h>
#include <tabree/KTreeFile.h>
#include "sensor_config_by_ktf.hh"
#include "sensor_table.hh"
#include "kebap_calibration.hh"

using namespace std;
using namespace honeybee;

sensor_config_by_ktf::sensor_config_by_ktf()
    : f_standard_parser(nullptr)
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
    cout << "Loading KTF file: " << a_filename << endl;
    
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
        cout << "Extracted Kebap scripts (" << t_scripts.length() << " bytes)" << endl;
        try {
            f_standard_parser = make_shared<kebap::KPStandardParser>();
            std::istringstream script_stream(t_scripts);
            f_standard_parser->Parse(script_stream);
            
            // Ensure global variables from ktf script are registered in symbol table
            // Executing bare statements immediately after parsing
            f_standard_parser->GetModule()->ExecuteBareStatements(f_standard_parser->GetSymbolTable());
            
            cout << "Successfully compiled Kebap parser" << endl;
        }
        catch (kebap::KPException &e) {
            cerr << "ERROR: Failed to parse Kebap scripts: " << e.what() << endl;
            f_standard_parser = nullptr;
            return;
        }
    } else {
        cout << "No Kebap scripts found in ktf header" << endl;
    }
    
    // Recursively load and configure sensors from hierarchical KTF structure
    load_context t_context;
    load_layer(a_table, t_tree["sensor_table"], t_context);
    cout << "Completed loading ktf file" << endl;
}

string sensor_config_by_ktf::extract_scripts()
{
    ifstream t_file(f_ktf_path);
    if (!t_file.is_open()) {
        cout << "Could not open ktf file for script extraction" << endl;
        return "";  // File read error, cont without scripts
    }
    
    string t_scripts;
    string t_line;
    
    while (getline(t_file, t_line)) {
        // Trim leading whitespace
        size_t t_start = t_line.find_first_not_of(" \t");
        if (t_start == string::npos) {
            t_line = "";  // All whitespace
        } else {
            t_line = t_line.substr(t_start);
        }
        
        // Check for script line, begin #%
        if (t_line.size() >= 2 && t_line.substr(0, 2) == "#%") {
            // Extract content after "#%"
            if (t_line.size() > 3 && t_line[2] == ' ') {
                t_scripts += t_line.substr(3) + "\n";
            } else if (t_line.size() > 2) {
                t_scripts += t_line.substr(2) + "\n";
            } 
        }
        else if (!t_line.empty()) {
            // end of script section
            break;
        }
        // no worries about empty lines before sensor definition lines
    }
    
    return t_scripts;
}

void sensor_config_by_ktf::load_layer(sensor_table& a_table, const tabree::KTree& a_node, load_context& a_context)
{
    // Helper for array index formatting
    auto append_index = [](const string& text, int length, unsigned index)->string {
        if (length < 0) {
            return text;
        }
        int width = (length==0) ? 1 : int(log10(length-0.5)+1);
        ostringstream os;
        os << text << setw(width) << setfill('0') << index;
        return os.str();
    };
    
    // Check if this is a channel node
    if (a_node.NodeName() == "channel") {
        add_sensor(a_table, a_node, a_context);
        return;
    }
    
    static const char* t_subnode_types[] = {
        "experiment", "setup", "teststand", "system",
        "section", "subsection", "division", "segment", "crate",
        "module", "device", "card", "board",
        "channel", "endpoint", "metric"
    };
    
    for (const char* t_subnode_type: t_subnode_types) {
        for (unsigned i = 0; i < a_node[t_subnode_type].Length(); i++) {
            const auto& t_node = a_node[t_subnode_type][i];
            string t_name = t_node["id"]["name"].As<string>();
            string t_label = t_node["id"]["label"].Or(t_name);
            int t_array_length = t_node["array_length"].Or(-1);
            string t_condition = t_node["valid_if"].Or("");
            
            // Check guard conditions
            if (! t_condition.empty()) {
                kebap::KPEvaluator f(t_condition);
                for (const auto& var: get_variables()) {
                    f[var.first] = var.second;
                }
                try {
                    if (! f(0)) {
                        continue;
                    }
                }
                catch (kebap::KPException &e) {
                    cerr << "ERROR: " << e.what() << ": " << t_node.NodePath() << endl;
                }
            }
            
            // Handle array expansion
            for (int j = 0; j < std::max<int>(1, t_array_length); j++) {
                auto t_context = a_context;
                t_context.f_name.push_front(append_index(t_name, t_array_length, j));
                t_context.f_label.push_front(append_index(t_label, t_array_length, j));
                
                // Extract options (x- prefixed keys)
                for (const auto& t_key: t_node.KeyList()) {
                    if ((t_key.substr(0, 2) == "x_") || (t_key.substr(0, 2) == "x-")) {
                        string t_opt_name = t_key.substr(2);
                        
                        if (t_node[t_key].IsLeaf()) {
                            // Simple string format
                            string t_opt_value = t_node[t_key].As<string>();
                            if (! t_opt_name.empty()) {
                                t_context.f_opts.emplace_back(t_opt_name, t_opt_value);
                            }
                        } else {
                            // Object format: x-dripline_endpoint
                            if (t_opt_name == "dripline_endpoint") {
                                string tag = t_node[t_key]["tag"].As<string>();
                                string field = t_node[t_key]["field"].Or("");
                                t_context.f_opts.emplace_back("dripline_endpoint", tag);
                                t_context.f_opts.emplace_back("dripline_endpoint_field", field);
                            }
                        }
                    }
                }
                
                // Recurse on the child node
                load_layer(a_table, t_node, t_context);
            }
        }
    }
}

void sensor_config_by_ktf::add_sensor(sensor_table& a_table, const tabree::KTree& a_node, 
                                      const load_context& a_context)
{
    int t_number = sensor_table::create_unique_number();
    vector<string> t_name_chain(a_context.f_name.begin(), a_context.f_name.end());
    vector<string> t_label_chain(a_context.f_label.begin(), a_context.f_label.end());
    
    sensor t_sensor(t_number, t_name_chain, t_label_chain);
    
    // Set calibration string
    string t_calibration = a_node["default_calibration"].Or("");
    t_sensor.set_calibration(t_calibration);
    
    // Create kebap_calibration if calibration string exists and parser is valid
    if (!t_calibration.empty() && f_standard_parser) {
        cout << "Attaching calibration to " << t_name_chain.front() 
             << ": \"" << t_calibration << "\"" << endl;
        try {
            auto t_calib = make_shared<kebap_calibration>(t_sensor, a_table, f_standard_parser.get(), f_ktf_path);
            t_sensor.set_calibration_object(t_calib);
            cout << "Successfully compiled calibration expression" << endl;
        }
        catch (exception &e) {
            cerr << "WARNING: Could not create calibration for " 
                 << t_name_chain.front() << ": " << e.what() << endl;
        }
    } else if (!t_calibration.empty() && !f_standard_parser) {
        cout << "Calibration string exists but no Kebap parser available: " 
             << t_calibration << endl;
    } else {
        cout << "No calibration for sensor: " << t_name_chain.front() << endl;
    }
    
    // Extract and set options
    map<string, string> t_options;
    for (const auto& t_opt: a_context.f_opts) {
        t_options[t_opt.first] = t_opt.second;
    }
    
    if(!f_ktf_path.empty()) {
        t_options["ktf_source"] = f_ktf_path;
    }
    
    for (const auto& t_key: a_node.KeyList()) {
        if ((t_key.substr(0, 2) == "x_") || (t_key.substr(0, 2) == "x-")) {
            string t_opt_name = t_key.substr(2);
            
            if (a_node[t_key].IsLeaf()) {
                string t_opt_value = a_node[t_key].As<string>();
                if (!t_opt_name.empty()) {
                    t_options[t_opt_name] = t_opt_value;
                }
            } else {
                // Handle dripline_endpoint format
                if (t_opt_name == "dripline_endpoint") {
                    string tag = a_node[t_key]["tag"].As<string>();
                    string field = a_node[t_key]["field"].Or("");
                    t_options["dripline_endpoint"] = tag;
                    t_options["dripline_endpoint_field"] = field;
                }
            }
        }
    }
    
    for (auto& t_opt: t_options) {
        t_sensor.set_option(t_opt.first, t_opt.second);
    }
    
    a_table.add(t_sensor);
}
