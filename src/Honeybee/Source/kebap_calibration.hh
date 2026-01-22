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
                         kebap::KPParser* a_parser, const string& a_ktf_path);
        ~kebap_calibration() override = default;
        
        double operator()(double x) override;
        string get_error_context() const override;
        
      private:
        string f_ktf_path;
        string f_expression_text;
        shared_ptr<evaluator> f_evaluator;
    };
    
}
#endif
