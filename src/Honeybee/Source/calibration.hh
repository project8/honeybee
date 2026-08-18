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
#include "series.hh"


namespace honeybee {
    using namespace std;
    
    class sensor;
    class sensor_table;

    class calibration {
      public:
        calibration() : f_input(0), f_is_identity(false) {}
        virtual ~calibration() = default;
        
        int get_input_sensor() const { return f_input; }
        string get_description() const { return f_description; }
        
        virtual double operator()(double x) = 0;
        virtual string get_error_context() const = 0;
        
      protected:
        string f_description;
        string f_variable_name;
        int f_input;
        bool f_is_identity;
    };
    
}
#endif


