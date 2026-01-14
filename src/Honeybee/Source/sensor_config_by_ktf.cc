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

// Helper struct for load_layer recursion
struct load_context {
    deque<string> f_name, f_label;
    deque<pair<string, string>> f_opts;
};

// static helper for load layer
static void load_layer_implement(sensor_config_by_ktf* a_loader, const tabree::KVariant& a_node, 
                            sensor_table& a_table, load_context& a_context);

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
    ifstream t_file(f_ktf_path);
    if (!t_file.is_open()) {
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

void sensor_config_by_ktf::load_layer(const tabree::KVariant& a_node, sensor_table& a_table)
{
    load_context t_context;
    load_layer_impl(this, a_node, a_table, t_context);
}

static void load_layer_implement(sensor_config_by_ktf* a_loader, const tabree::KVariant& a_node, 
                            sensor_table& a_table, load_context& a_context)
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
    
    if (a_node["id"]["name"].IsVoid()) {
        return;
    }
    
    // Check if this is a channel node
    if (a_node.NodeName() == "channel") {
        add_sensor(a_node, a_table, a_context, a_node.LineOffset());
        return;
    }
    
    string t_name = a_node["id"]["name"].As<string>();
    string t_label = a_node["id"]["label"].Or(t_name);
    int t_array_length = a_node["array_length"].Or(-1);
    string t_condition = a_node["valid_if"].Or("");
    
    // Check guard conditions
    if (! t_condition.empty()) {
        kebap::KPEvaluator f(t_condition);
        for (const auto& var: a_loader->f_variables) {
            f[var.first] = var.second;
        }
        try {
            if (! f(0)) {
                return;
            }
        }
        catch (kebap::KPException &e) {
            cerr << "ERROR: " << e.what() << ": " << a_node.NodePath() << endl;
        }l
    }
    
    // Handle array expansion
    for (int j = 0; j < std::max<int>(1, t_array_length); j++) {
        auto t_context = a_context;
        t_context.f_name.push_front(append_index(t_name, t_array_length, j));
        t_context.f_label.push_front(append_index(t_label, t_array_length, j));
        
        // Extract options (x- prefixed keys)
        for (const auto& t_key: a_node.KeyList()) {
            if ((t_key.substr(0, 2) == "x_") || (t_key.substr(0, 2) == "x-")) {
                string t_opt_name = t_key.substr(2);
                
                if (a_node[t_key].IsLeaf()) {
                    // Simple string format
                    string t_opt_value = a_node[t_key].As<string>();
                    if (! t_opt_name.empty()) {
                        t_context.f_opts.emplace_back(t_opt_name, t_opt_value);
                    }
                } else {
                    // Object format: x-dripline_endpoint
                    if (t_opt_name == "dripline_endpoint") {
                        string tag = a_node[t_key]["tag"].As<string>();
                        string field = a_node[t_key]["field"].Or("raw");
                        t_context.f_opts.emplace_back("dripline_endpoint", tag);
                        t_context.f_opts.emplace_back("dripline_endpoint_field", field);
                    }
                }
            }  
        }
        
        // Recursively traverse subnodes
        static const char* t_subnode_types[] = {
            "experiment", "setup", "teststand", "system",
            "section", "subsection", "division", "segment", "crate",
            "module", "device", "card", "board",
            "channel", "endpoint", "metric"
        };
        
        for (const char* t_subnode_type: t_subnode_types) {
            for (unsigned i = 0; i < a_node[t_subnode_type].Length(); i++) {
                load_layer_impl(a_loader, a_node[t_subnode_type][i], a_table, t_context);
            }
        }
    }
}

void sensor_config_by_ktf::add_sensor(sensor_table& a_table, const tabree::KVariant& a_node, 
                                      const load_context& a_context, int a_line_offset)
{
    int t_number = sensor_table::create_unique_number();
    vector<string> t_name_chain(a_context.f_name.begin(), a_context.f_name.end());
    vector<string> t_label_chain(a_context.f_label.begin(), a_context.f_label.end());
    
    sensor t_sensor(t_number, t_name_chain, t_label_chain);
    
    // Set calibration string
    string t_calibration = a_node["default_calibration"].Or("");
    t_sensor.set_calibration(t_calibration);
    
    // Create kebap_calibration if calibration string exists and parser is valid
    if (!t_calibration.empty() && f_parser) {
        try {
            auto t_calib = make_shared<kebap_calibration>(t_sensor, a_table, f_parser.get(), 
                                                          f_ktf_path, a_line_offset);
            t_sensor.set_calibration_object(t_calib);
        }
        catch (exception &e) {
            cerr << "WARNING: Could not create calibration for " 
                 << t_name_chain.front() << ": " << e.what() << endl;
        }
    }
    
    // Extract and set options
    map<string, string> t_options;
    for (const auto& t_opt: a_context.f_opts) {
        t_options[t_opt.first] = t_opt.second;
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
                    string field = a_node[t_key]["field"].Or("raw");
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
