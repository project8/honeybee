/*
 * kebap_calibration.hh
 */

#ifndef HONEYBEE_KEBAP_CALIBRATION_HH_
#define HONEYBEE_KEBAP_CALIBRATION_HH_ 1

#include <string>
#include <memory>
#include <kebap/Kebap.h>
#include "calibration.hh"


namespace honeybee {
    using namespace std;
    
    class sensor;
    class sensor_table;
    class evaluator;

    class kebap_calibration : public calibration {
      public:
        kebap_calibration(const sensor& a_sensor, const sensor_table& a_sensor_table,
                         kebap::KPParser* a_parser, const string& a_ktf_path, int a_line_number = 0);
        ~kebap_calibration() override = default;
        
        double operator()(double x) override;
        string get_error_context() const override;
        
      private:
        string f_ktf_path;
        int f_line_number;
        string f_expression_text;
        shared_ptr<evaluator> f_evaluator;
    };


    // make factory function, factory pattern, where it handles the ktf context and make a class accordingly, so seperated 
    // in the sensor_config_by_ktf 

    // honeybee with need a python interface to use a python interface for the ui 

    // and for now, just saying it is users responsiblity to make sure that the calibration doesn't change in between time 

    // we need to decide after honeybee is working, it has the modes of doing different db_calibration, then into the interface\

    // then comes docuementation and clenaing up code 
    
}
#endif
