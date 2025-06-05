/*
 * calibration.cc
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <memory>
#include <regex>
#include "sensor_table.hh"
#include "evaluator.hh"
#include "calibration.hh"


using namespace std;
using namespace honeybee;


calibration::calibration(const sensor& a_sensor, const sensor_table& a_sensor_table)
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
    f_is_identity = false;
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
    
    f_evaluator = make_shared<evaluator>(t_exp_text);
    try {
        f_evaluator->operator()(0);
    }
    catch (std::exception &e) {
        cerr << "ERROR: bad calibration expression: " << e.what() << endl;
        f_evaluator = 0;
    }
}
