/*
 * calibration.hh
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_CALIBRATION_HH_
#define HONEYBEE_CALIBRATION_HH_ 1

#include <string>
#include <memory>
#include "ktf_script.hh"   
#include "series.hh"
#include "sensor_table.hh"
#include "evaluator.hh"


namespace honeybee {
    using namespace std;

    class calibration {
      public:
        calibration(): f_is_identity(false) {}
        calibration(const sensor& a_sensor, const sensor_table& a_sensor_table);
        int get_input_sensor() const { return f_input; }
        string get_description() const { return f_description; }
    double operator()(double x) const {  // Evaluate calibration (uses script-backed execution if enabled)
            if (f_is_identity) {
                return x;
            }
            // If this calibration uses script-based UDF execution, route through parser helper.
            if (f_use_script_call) {
                // ...use g_ktf_script_contexts at runtime...
                auto iter = honeybee::g_ktf_script_contexts.find(f_ktf_source);
                if (iter == honeybee::g_ktf_script_contexts.end()) {
                    return std::numeric_limits<double>::quiet_NaN();
                }
                double outv = std::numeric_limits<double>::quiet_NaN();
                if (execute_script_call(iter->second, f_script_call_expression, std::vector<double>{x}, outv)) {
                    return outv;
                }
                return std::numeric_limits<double>::quiet_NaN();
            }

            if (! f_evaluator) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            return (*f_evaluator)(x);
        }
      protected:
        string f_description, f_variable_name;
        int f_input;
        bool f_is_identity;
        shared_ptr<evaluator> f_evaluator;

       // Script-backed execution, when udf called, f_script_call_expression is evaluated via
       // the per-file KTFScriptContext stored in g_ktf_script_contexts[f_ktf_source.
       bool f_use_script_call = false;
       string f_ktf_source;
       string f_script_call_expression;
    };
    
}
#endif
