/*
 * kebap_calibration.cc
 */

#include <string>
#include <memory>
#include <sstream>
#include <regex>
#include <cctype>
#include <stdexcept>
#include <kebap/Kebap.h>
#include "sensor_table.hh"
#include "evaluator.hh"
#include "kebap_calibration.hh"

using namespace std;
using namespace honeybee;

// Helper function to fix Kebap 0-indexed line numbers
static string normalize_line_numbers(const string& error_msg)
{
    string result = error_msg;
    size_t pos = 0;
    while ((pos = result.find("line ", pos)) != string::npos) {
        pos += 5;  // skip past "line "
        if (pos < result.length() && isdigit(result[pos])) {
            int line_num = stoi(result.substr(pos));
            string old_num = to_string(line_num);
            string new_num = to_string(line_num + 1);
            result.replace(pos, old_num.length(), new_num);
            pos += new_num.length();
        }
    }
    return result;
}


kebap_calibration::kebap_calibration(const sensor& a_sensor, const sensor_table& a_sensor_table,
                                     kebap::KPParser* a_parser, const string& a_ktf_path, int a_line_number)
    : f_ktf_path(a_ktf_path), f_line_number(a_line_number)
{
    auto strip = [](const string& a_text)->string {
        string::size_type t_begin = 0, t_length = a_text.size();
        while (t_begin < t_length) {
            if (a_text[t_begin] != ' ') {
                break;
            }
            t_begin++;
        }
        while (t_length > t_begin) {
            if (a_text[t_length-1] != ' ') {
                break;
            }
            t_length--;
        }
        return a_text.substr(t_begin, t_length);
    };

    f_description = strip(a_sensor.get_calibration());
    f_input = sensor{}.get_number();
    f_evaluator = 0;
    
    if (f_description.empty()) {
        return;
    }

    auto colon = f_description.find_first_of(':');
    f_variable_name = strip(f_description.substr(0, colon));
    string t_exp_text;
    if (colon != string::npos) {
        t_exp_text = strip(f_description.substr(colon+1));
    }
    
    f_expression_text = t_exp_text;
    
    name_chain t_input_name_chain = a_sensor.get_name();
    name_chain t_variable_name_chain = name_chain(f_variable_name, "./-_");
    if (t_input_name_chain.size() < t_variable_name_chain.size()) {
        t_input_name_chain = t_variable_name_chain;
    }
    else {
        for (unsigned i = 0; i < t_variable_name_chain.size(); i++) {
            t_input_name_chain[i] = t_variable_name_chain[i];
        }
    }
    
    auto t_candidates = a_sensor_table.find_like(t_input_name_chain);
    if (t_candidates.size() == 1) {
        f_input = t_candidates.front();
    }
    else if (t_candidates.size() > 0) {
        cerr << "ERROR: ambiguous calibration input: " << f_variable_name << endl;
        return;
    }
    else {
        f_input = a_sensor_table[t_variable_name_chain];
    }
    
    if (! f_input) {
        cerr << "ERROR: unable to find calibration input: " << f_variable_name << endl;
        return;
    }
    
    if ((f_variable_name == t_exp_text) || t_exp_text.empty()) {
        f_is_identity = true;
        return;
    }

    // replace the variable in the expression with "x"
    string t_pattern = regex_replace(f_variable_name, regex("\\."), "\\.");
    t_exp_text = regex_replace(t_exp_text, regex("(^|[^a-zA-Z_])(" + t_pattern + ")($|[^a-zA-Z0-9_])"), "$1x$3");
    
    // Compile expression using provided parser
    try {
        kebap::KPExpressionParser* t_expr_parser = a_parser->GetExpressionParser();
        istringstream expr_stream(t_exp_text);
        
        kebap::KPTokenizer t_tokenizer(expr_stream, a_parser->GetTokenTable());
        kebap::KPExpression* t_expression = t_expr_parser->Parse(&t_tokenizer, a_parser->GetSymbolTable());
        kebap::KPSymbolTable* t_symbol_table = a_parser->GetSymbolTable();
        
        f_evaluator = make_shared<evaluator>(t_expression, t_symbol_table);
        
        // Test evaluate to catch early errors
        (*f_evaluator)(0);
    }
    catch (exception &e) {
        string error_msg = normalize_line_numbers(e.what());
        // Extract just the error part after "evaluator: "
        size_t eval_pos = error_msg.find("evaluator: ");
        if (eval_pos != string::npos) {
            error_msg = error_msg.substr(eval_pos + 11);  // Skip "evaluator: "
        }
        cerr << "ERROR: " << f_ktf_path << " - calibration: '" << f_expression_text << "'" << endl;
        cerr << endl;
        cerr << "(ISSUE) " << error_msg << endl;
        f_evaluator = 0;
    }
}

double kebap_calibration::operator()(double x)
{
    if (f_is_identity) {
        return x;
    }
    if (!f_evaluator) {
        return numeric_limits<double>::quiet_NaN();
    }
    
    try {
        return (*f_evaluator)(x);
    }
    catch (exception& e) {
        throw runtime_error(get_error_context() + " - " + normalize_line_numbers(e.what()));
    }
}

string kebap_calibration::get_error_context() const
{
    ostringstream oss;
    oss << f_ktf_path;
    if (f_line_number > 0) {
        oss << ":" << (f_line_number + 1);  // Convert to 1-indexed for user display
    }
    oss << " in expression: '" << f_expression_text << "'";
    return oss.str();
}
