/*
 * sensor_table.cc
 *
 *  Created on: Oct 7, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */


#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <cmath>
#include <regex>
#include <kebap/Kebap.h>
#include <kebap/KPParser.h>
#include <kebap/KPStatement.h>
#include <kebap/KPFunction.h>
#include <kebap/KPTokenizer.h>
#include <tabree/KTreeFile.h>
#include "utils.hh"
#include "sensor_table.hh"
#include "data_source.hh"
#include "evaluator.hh"   
#include <functional>   //function wrapper

using namespace std;
using namespace honeybee;

int sensor_table::f_unique_sequence = 0;


// Extracts and combines all "#%" script lines from a KTF file into a single script, inlining any included KTF files.
// The combined script is stored in out_ctx
// recursive include is flagged 
static bool extract_ktf_scripts_from_file(const std::string& filename, honeybee::KTFScriptContext& out_ctx) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "ERROR: Cannot open KTF file for script extraction: " << filename << std::endl;
        return false;
    }

    // collect lines that start with "#%" (after optional whitespace)
    std::vector<std::string> collected;
    std::string line;
    while (std::getline(ifs, line)) {
        size_t pos = line.find_first_not_of(" \t");
        if (pos != std::string::npos && pos + 1 < line.size() && line[pos] == '#' && line[pos+1] == '%') {
            size_t start = pos + 2;
            if (start < line.size() && line[start] == ' ') start++;
            collected.push_back(line.substr(start));
        }
    }
    ifs.close();

    // recursive include processing using std::set for visited files
    std::function<void(const std::string&, std::string&, std::set<std::string>&)> process_file;
    process_file = [&](const std::string& file, std::string& out_script, std::set<std::string>& visited) {
        if (visited.find(file) != visited.end()) {
            std::cerr << "WARNING: recursive include skipped: " << file << std::endl;
            return;
        }
        visited.insert(file);

        std::ifstream s(file);
        if (!s.is_open()) {
            std::cerr << "ERROR: include: cannot open " << file << std::endl;
            return;
        }

        std::string ln;
        std::vector<std::string> local_collected;
        while (std::getline(s, ln)) {
            size_t pos = ln.find_first_not_of(" \t");
            if (pos != std::string::npos && pos + 1 < ln.size() && ln[pos] == '#' && ln[pos+1] == '%') {
                size_t start = pos + 2;
                if (start < ln.size() && ln[start] == ' ') start++;
                local_collected.push_back(ln.substr(start));
            }
        }
        s.close();

        std::regex include_regex(R"(^\s*include\s*\"([^\"]+)\"\s*$)");
        for (const auto& l : local_collected) {
            std::smatch m;
            if (std::regex_match(l, m, include_regex)) {
                std::string incfile = m[1].str();
                std::string resolved = incfile;
                if (!incfile.empty() && incfile[0] != '/' && file.find('/') != std::string::npos) {
                    std::string dir = file.substr(0, file.find_last_of('/'));
                    resolved = dir + "/" + incfile;
                }
                process_file(resolved, out_script, visited);
            } else {
                out_script += l;
                out_script += "\n";
            }
        }
    };

    // build combined script starting from the main file (and inline includes found in that file)
    std::string combined;
    std::set<std::string> visited;
    std::regex include_regex(R"(^\s*include\s*\"([^\"]+)\"\s*$)");
    for (const auto& l : collected) {
        std::smatch m;
        if (std::regex_match(l, m, include_regex)) {
            std::string incfile = m[1].str();
            std::string resolved = incfile;
            if (!incfile.empty() && incfile[0] != '/' && filename.find('/') != std::string::npos) {
                std::string dir = filename.substr(0, filename.find_last_of('/'));
                resolved = dir + "/" + incfile;
            }
            process_file(resolved, combined, visited);
        } else {
            combined += l;
            combined += "\n";
        }
    }

    out_ctx.filename = filename;
    out_ctx.combined_script = combined;
    out_ctx.parsed = false;
    // parser will be created on demand by evaluator::parse_script
    return true;
}

string sensor::to_json(vector<string> a_field_list, const std::string& a_delimiter) const
{
    if (a_field_list.empty()) {
        a_field_list = {{"number", "name", "label", "default_calibration", "options"}};
    }
    
    ostringstream os;
    string delim = " ";
    
    os << "{";
    for (auto& f: a_field_list) {
        if (f == "number") {
            //os << delim << "\"" << f << "\": 0x" << hex << f_number << dec;
            os << delim << "\"" << f << "\": " << f_number;
        }
        else if (f == "name") {
            os << delim << "\"" << f << "\": \"" << f_name.join(a_delimiter) << "\"";
        }
        else if (f == "label") {
            os << delim << "\"" << f << "\": \"" << f_label.join(", ") << "\"";
        }
        else if ((f.find("calibration") != string::npos)  && ! f_calibration.empty()) {
            os << delim << "\"default_calibration\": \"" << f_calibration << "\"";
        }
        else if ((f.substr(0,3) == "opt") && ! f_options.empty()) {
            bool t_is_first_opt = true;
            os << delim << "\"options\": {";
            for (auto& t_opt: f_options) {
                os << (t_is_first_opt ? " " : ", "); 
                os << "\"" << t_opt.first << "\": \"" << t_opt.second << "\"";
                t_is_first_opt = false;
            }
            os << " }";
        }
        delim = ", ";
    }
    os << " }";
    
    return os.str();
}


vector<int> sensor_table::find_like(const name_chain& a_chain) const
{
    const auto& t_pattern = a_chain.get_chain();
    vector<int> t_matches;
    
    for (const auto& t_sensor: f_table) {
        auto iter = t_pattern.begin();
        for (const string& node: t_sensor.second.get_name().get_chain()) {
            if (iter == t_pattern.end()) {
                break;
            }
            if (node == *iter) {
                iter++;
            }
        }
        if (iter == t_pattern.end()) {
            t_matches.push_back(t_sensor.first);
        }
    }
    
    return t_matches;
}

int sensor_table::find_one_like(const name_chain& a_chain) const
{
    auto all = this->find_like(a_chain);
    if (all.empty()) {
        cerr << "ERROR: unable to find sensor: " << a_chain.join(".") << endl;
        return f_null_sensor;
    }
    if (all.size() > 1) {
        cerr << "ERROR: multiple possibilities for: " << a_chain.join(".") << endl;
        for (auto& each: all) {
            cerr << "  " << this->operator[](each).get_name().join(".");
        }
        cerr << endl;
    }
    
    return all.front();
}



void sensor_config_by_file::set_variables(const sensor_config_by_file::variables& a_variables)
{
    f_variables.insert(f_variables.end(), a_variables.begin(), a_variables.end());
}

void sensor_config_by_file::load(sensor_table& a_table, const string& a_filename)
{   
    // record the source filename for sensors added by this loader
    f_source_filename = a_filename;

    // Register KTF script context for this file
    extract_ktf_scripts_from_file(a_filename, g_ktf_script_contexts[a_filename]);

    // Parse the combined KTF script immediately so top-level parser contains UDFs/vars/imports.
    try {
        auto &ctx = g_ktf_script_contexts[a_filename];
        if (!ctx.combined_script.empty()) {
            if (!parse_script(ctx)) {
                std::cerr << "WARNING: failed to parse KTF script for " << a_filename << std::endl;
            } else {
                hINFO(cerr << "Parsed KTF script: " << a_filename << endl);
            }
        }
    } catch (const std::exception &e) {
        cerr << "WARNING: failed to parse KTF script for " << a_filename << ": " << e.what() << endl;
    }

    tabree::KTree t_tree;
    try {
        tabree::KTreeFile(a_filename).Read(t_tree);
    }
    catch (tabree::KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return;
    }

    context t_context;
    load_layer(a_table, t_tree["sensor_table"], t_context);
}

void sensor_config_by_file::load_layer(sensor_table& a_table, const tabree::KTree& a_node, sensor_config_by_file::context a_context)
{
    if (a_node.NodeName() == "channel") {
        add_sensor(a_table, a_node, a_context);
        return;
    }
    
    auto append_index = [](const string& text, int length, unsigned index)->string {
        if (length < 0) {
            return text;
        }
        int width = (length==0) ? 1 : int(log10(length-0.5)+1);
        ostringstream os;
        os << text << setw(width) << setfill('0') << index;
        return os.str();
    };

    static const char* t_subnode_types[] = {
        "experiment", "setup", "teststand", "system",
        "section", "subsection", "division", "segment", "crate",
        "module", "device", /*"unit",*/ "card", "board",
        "channel", "endpoint", "metric"
    };
    for (const char* t_subnode_type: t_subnode_types) {
        for (unsigned i = 0; i < a_node[t_subnode_type].Length(); i++) {
            const auto& t_node = a_node[t_subnode_type][i];
            string t_name = t_node["id"]["name"];
            string t_label = t_node["id"]["label"].Or(t_name);
            int t_array_length = t_node["array_length"].Or(-1);
            string t_condition = t_node["valid_if"].Or("");
            
            // check guard conditions //
            if (! t_condition.empty()) {
                kebap::KPEvaluator f(t_condition);
                for (const auto& var: f_variables) {
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
            
            //Nobel: checks if the x-'s retrieved is object of a string 
            for (int j = 0; j < std::max<int>(1, t_array_length); j++) {
                auto t_context = a_context;
                t_context.f_name.push_front(append_index(t_name, t_array_length, j));
                t_context.f_label.push_front(append_index(t_label, t_array_length, j));
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
                            // Object format: x-dripline_endpoint: { tag: ..., field: ... }
                            if (t_opt_name == "dripline_endpoint") {
                                string tag = t_node[t_key]["tag"].As<string>();
                                string field = t_node[t_key]["field"].Or("raw");
                                t_context.f_opts.emplace_back("dripline_endpoint", tag);
                                t_context.f_opts.emplace_back("dripline_endpoint_field", field);
                            }
                        }
                    }
                }
                load_layer(a_table, t_node, t_context);
            }
        }
    }
}

void sensor_config_by_file::add_sensor(sensor_table& a_table, const tabree::KTree& a_node, sensor_config_by_file::context a_context)
{
    int t_number = sensor_table::create_unique_number();
    vector<string> t_name_chain(a_context.f_name.begin(), a_context.f_name.end());
    vector<string> t_label_chain(a_context.f_label.begin(), a_context.f_label.end());
    sensor t_sensor(t_number, t_name_chain, t_label_chain);

    t_sensor.set_calibration(a_node["default_calibration"]);

    map<string, string> t_options;
    // this step is to allow overriding //
    for (auto& t_opt: a_context.f_opts) {
        t_options[t_opt.first] = t_opt.second;
    }
    // tag sensor with the KTF source filename so calibration code can find script context
    if (! f_source_filename.empty()) {
        t_options["ktf_source"] = f_source_filename;
    }
    for (auto& t_opt: t_options) {
        t_sensor.set_option(t_opt.first, t_opt.second);
    }
    
    a_table.add(t_sensor);
}



void sensor_config_by_names::set_delimiters(const string& a_input_delimiters, const string& a_output_delimiter)
{
    f_input_delimiters = a_input_delimiters;
    f_output_delimiter = a_output_delimiter;
}

void sensor_config_by_names::load(sensor_table& a_table, const vector<string>& a_name_list, name_chain a_basename)
{
    hINFO(cerr << "Sensor ID matching or creation" << endl);
    if (! f_name_space.empty()) {
        hINFO(cerr << "    Namespace: " << f_name_space << endl);
        hINFO(cerr << "    Basename: " << a_basename.join() << endl);
    }

    map<string, string> t_binding;
    if (! f_name_space.empty()) {
        for (int t_number: a_table.find_like({{}})) {
             const sensor& t_sensor = a_table[t_number];
             string t_endpoint = t_sensor.get_option(f_name_space, "");
             if (! t_endpoint.empty()) {
                 t_binding[t_endpoint] = t_sensor.get_name().join(f_output_delimiter);
             }
        }
    }

    for (const string& t_name: a_name_list) {
        // explicit matching
        auto t_explicit_iter = t_binding.find(t_name);
        if (t_explicit_iter != t_binding.end()) {
            hINFO(cerr << "    Explicit: " << t_name << " => " << t_explicit_iter->second << endl);
            continue;
        }
        
        // inference by loose matching
        vector<string> t_chain = name_chain{t_name, f_input_delimiters}.get_chain();
        t_chain.insert(t_chain.end(), a_basename.get_chain().begin(), a_basename.get_chain().end());

        sensor t_sensor;
        auto t_sensor_matches = a_table.find_like(t_chain);
        if (t_sensor_matches.size() == 1) {
            t_sensor = a_table[t_sensor_matches.front()];
            hINFO(cerr << "    Inferred: " << t_name << " => " << t_sensor.get_name().join(f_output_delimiter) << endl);
        }

        // non-unique matching, error, skipped
        else if (t_sensor_matches.size() > 1) {
            hERROR(cerr << "    Mutiple possibilities on binding: " << t_name << ": " << endl);
            for (auto& s: t_sensor_matches) {
                hERROR(cerr << "        " << a_table[s].get_name().join(f_output_delimiter) << endl);
            }
            hERROR(cerr << "      hint: use explicit binding to resolve ambiguity" << endl);
            continue;
        }

        // create a new sensor entry
        if (! t_sensor) {
            auto t_number = a_table.create_unique_number();
            t_sensor = sensor{t_number, t_chain, t_chain};
            hINFO(cerr << "    Created: " << t_name << " => " << t_sensor.get_name().join(f_output_delimiter) << endl);
        }
        if (! f_name_space.empty()) {
            t_sensor.set_option(f_name_space, t_name);
        }
        a_table.add(t_sensor);
    }
}
