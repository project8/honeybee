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

using namespace std;
using namespace honeybee;

int sensor_table::f_unique_sequence = 0;

// Nobel: Global parser instance being reused across all function parsing
static kebap::KPStandardParser* g_function_parser = nullptr;

// Nobel: Detect functions that need full Kebap execution
// Any function with multiple statements, loops, or complex logic
bool honeybee::is_complex_function_syntax(const string& body) {
    // Check for control flow that requires full statement execution
    if (body.find("if (") != string::npos) return true;
    if (body.find("while (") != string::npos) return true;
    if (body.find("for (") != string::npos) return true;  // Include for loops
    if (body.find("var ") != string::npos) return true;
    
    // Check for multiple statements (semicolons indicate statement separation)
    if (body.find(';') != string::npos) return true;
    
    // Check for variable declarations
    if (body.find("double ") != string::npos) return true;
    if (body.find("float ") != string::npos) return true;
    if (body.find("int ") != string::npos) return true;
    
    return false;  // Simple expression function
}

// Nobel: Parse complex function body using proper KPFunction mechanism
unique_ptr<kebap::KPFunction> honeybee::parse_function_body(const string& body) {
    try {
        // Create a KPCxxFunction for proper C++ style function parsing
        unique_ptr<kebap::KPCxxFunction> kebap_function = make_unique<kebap::KPCxxFunction>();
        
        // Wrap function body in proper Kebap function syntax: "returnType functionName(params) { body }"
        // Extract return type, function name, and parameters from the original function definition
        string wrapped_function = "double temp_func(double base) { " + body + " }";
        
        // Create tokenizer and parsers for function parsing
        stringstream ss(wrapped_function);
        kebap::KPStandardParser parser;
        kebap::KPTokenizer tokenizer(ss, parser.GetTokenTable());
        kebap::KPSymbolTable* symbol_table = parser.GetSymbolTable();
        
        // Parse the function using Kebap's function parsing mechanism
        kebap_function->Parse(&tokenizer, parser.GetStatementParser(), symbol_table);
        
        cerr << "INFO: Complex function parsed successfully using KPFunction" << endl;
        
        return kebap_function;
    } catch (const exception& e) {
        cerr << "ERROR: KPFunction parsing failed: " << e.what() << endl;
        cerr << "ERROR: Function body was: '" << body << "'" << endl;
        return nullptr;
    }
}

// Nobel: Basic function signature validation
// Checks that return type and name are not empty
void honeybee::validate_function_signature(const string& return_type, const string& name, const vector<string>& args) {
    if (return_type.empty() || name.empty()) {
        throw runtime_error("Invalid function signature");
    }
}

// Nobel: Extract user-defined calibration functions from KTF file with kebap parsing(multi-line)
static void extract_user_functions_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open KTF file for UDF extraction: " << filename << std::endl;
        return;
    }
    
    // Nobel: Read entire file for multi-line function support
    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    file.close();

    // Nobel: regex for multi-line functions with nested braces
    regex udf_regex(R"(#%\s*(\w+)\s+(\w+)\s*\(([^)]*)\)\s*\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\})");
    
    // Nobel: Find all function matches in the file content
    std::sregex_iterator iter(content.begin(), content.end(), udf_regex);
    std::sregex_iterator end;
    
    // Nobel: Process each function definition found in the file
    // Loop through all regex matches and parse each function
    int function_count = 0;
    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        function_count++;
        
        try {
            UserCalibrateFunction udf;
            udf.return_type = match[1].str();
            udf.name = match[2].str();
            udf.body_expr = match[4].str();
            
            // Nobel: Parse to extract type and name pairs
            // like "float x, int count"
            string args_str = match[3].str();
            if (!args_str.empty()) {
                stringstream ss(args_str);
                string param;
                while (getline(ss, param, ',')) {
                    regex param_regex(R"(\s*(\w+)\s+(\w+)\s*)");
                    smatch param_match;
                    if (regex_match(param, param_match, param_regex)) {
                        string param_type = param_match[1].str();
                        string param_name = param_match[2].str();
                        udf.arg_names.push_back(param_name);
                        // Nobel: Parameter type info stored, could be used for future validation
                    }
                }
            }
            
            // Nobel: Check if function needs complex parsing or simple expression handling
            udf.f_is_complex_function = is_complex_function_syntax(udf.body_expr);
            
            if (udf.f_is_complex_function) {
                // Nobel: Store original body and create KPFunction for complex functions
                udf.f_original_body = udf.body_expr;
                udf.f_kebap_function = parse_function_body(udf.body_expr);
                if (!udf.f_kebap_function) {
                    cerr << "ERROR: Failed to parse complex function: " << udf.name << endl;
                    continue;
                }
                cerr << "INFO: Registered complex UDF: " << udf.name << endl;
            } else {
                // Nobel: Simple function - extract return expression for variable substitution
                string body = udf.body_expr;

                
                // Use regex to extract return expression, handling multiline and whitespace
                // First replace newlines with spaces to make it single line
                string single_line_body = body;
                replace(single_line_body.begin(), single_line_body.end(), '\n', ' ');
                regex return_regex(R"(.*return\s+([^;]+);?.*)");
                smatch body_match;
                if (regex_match(single_line_body, body_match, return_regex)) {
                    string return_expr = body_match[1].str();
                    // Trim whitespace and newlines
                    return_expr.erase(0, return_expr.find_first_not_of(" \t\n\r"));
                    return_expr.erase(return_expr.find_last_not_of(" \t\n\r") + 1);
                    udf.body_expr = return_expr;
                } else {
                    cerr << "WARNING: Could not extract return expression from: " << body << endl;
                }
                cerr << "INFO: Registered simple UDF: " << udf.name << endl;
            }
            
            // Nobel: Validate signature before registering
            validate_function_signature(udf.return_type, udf.name, udf.arg_names);
            g_user_calibrate_functions[udf.name] = std::move(udf);
            
        } catch (const exception& e) {
            cerr << "ERROR: Function parsing failed for " << match[2].str() << ": " << e.what() << endl;
        }
    }
    
    std::cerr << "INFO: UDF extraction complete. Found " << g_user_calibrate_functions.size() << " UDFs" << std::endl;
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
    // Nobel: Extract user-defined calibration functions first, before KTF parsing
    extract_user_functions_from_file(a_filename);
    
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
                    if (! f(0)) {  //... TODO: implement evaluator with no parameter
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
