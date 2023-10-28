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
#include <kebap/Kebap.h>
#include "series.hh"
#include "sensor_table.hh"


namespace honeybee {
    using namespace std;

    class calibration {
      public:
        calibration(): f_is_identity(false) {}
        calibration(const sensor& a_sensor, const sensor_table& a_sensor_table);
        int get_input_sensor() const { return f_input; }
        string get_description() const { return f_description; }
        double operator()(double x) const {
            if (f_is_identity) {
                return x;
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
        shared_ptr<kebap::KPEvaluator> f_evaluator;
    };
    
}
#endif
