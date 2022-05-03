/*
 * calibration.cc
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <memory>
#include <kebap/Kebap.h>
#include "sensor_table.hh"
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
    
    name_chain t_input_name = a_sensor.get_name();
    t_input_name[0] = f_variable_name;
    f_input = a_sensor_table[t_input_name];
    if (! f_input) {
        t_input_name = name_chain(f_variable_name, "./-_");
        auto a_candidates = a_sensor_table.find_like(t_input_name);
        if (a_candidates.size() == 1) {
            f_input = a_candidates.front();
            f_variable_name = t_input_name[0];
        }
        else if (a_candidates.size() > 0) {
            cerr << "ERROR: ambiguous calibration input: " << f_variable_name << endl;
            return;
        }
        else {
            f_input = a_sensor_table[{{f_variable_name}}];
        }
    }
    if (! f_input) {
        cerr << "ERROR: unable to find calibration input: " << f_variable_name << endl;
        return;
    }
    
    if ((f_variable_name == t_exp_text) || t_exp_text.empty()) {
        f_is_identity = true;
    }
    else {
        f_evaluator = make_shared<kebap::KPEvaluator>(t_exp_text);
        try {
            f_evaluator->operator[](f_variable_name) = 1;
            f_evaluator->operator()(0);
        }
        catch (kebap::KPException &e) {
            cerr << "ERROR: bad calibration expression: " << e.what() << endl;
            f_evaluator = 0;
        }
    }
}
