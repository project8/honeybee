/*
 * data_source.hh
 *
 *  Created on: Oct 19, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_DATA_SOURCE_HH_
#define HONEYBEE_DATA_SOURCE_HH_ 1

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "utils.hh"
#include "series.hh"
#include "sensor_table.hh"
#include "calibration.hh"
#include "pgsql.hh"

// Nobel: Forward declarations for Kebap integration
namespace kebap {
    class KPStatement;
    class KPFunction;
}

namespace honeybee {
    using namespace std;

    // Nobel: Extended UserCalibrateFunction to handle complex functions with control flow
    struct UserCalibrateFunction {
        string name;
        vector<string> arg_names;
        string body_expr;                                    // Keep for backward compatibility
        string return_type;                                  // optional, from UDF signature
        
        // Nobel: Added fields for complex function support
        bool f_is_complex_function = false;                  // Simple vs complex function flag
        string f_original_body;                              // Original multi-line function body for complex functions
        unique_ptr<kebap::KPFunction> f_kebap_function = nullptr;  // Proper Kebap function object for complex functions
    };

    // Global registry of user-defined calibration functions
    extern map<string, UserCalibrateFunction> g_user_calibrate_functions;

    // Nobel: Added function parsing utilities for Kebap statement tree parsing
    unique_ptr<kebap::KPFunction> parse_function_body(const string& body);
    bool is_complex_function_syntax(const string& body);
    void validate_function_signature(const string& return_type, const string& name, const vector<string>& args);

    class data_source {
      public:
        data_source() {}
        virtual ~data_source() {}
        virtual vector<string> get_data_names() = 0;
        virtual void bind(sensor_table& a_sensor_table);
        virtual vector<series> read(const vector<int>& a_sensor_list, const std::string& value_column, double a_from, double a_to, double a_resampling_interval=-1, const std::string& a_reducer="");
      protected:
        virtual void bind_inputs(sensor_table& sensor_table) = 0;
        virtual vector<series> fetch(const vector<int>& a_sensor_list, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column);
        virtual void fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column) = 0;
      protected:
        int find_input(int);
        void apply_calibration(int a_sensor, series& a_series);
      protected:
        map<int, calibration> f_calibration_table;
    };

    
    class empty_data_source: public data_source {
      public:
        vector<string> get_data_names() override { return vector<string>(); }
      protected:
        void bind_inputs(sensor_table& sensor_table) override {}
        void fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column) override {}
    };

    
    class dripline_pgsql: public data_source {
      public:
        dripline_pgsql(string a_uri, name_chain a_basename, const string& a_input_delimiters, const string& a_output_delimiters);
        vector<string> get_data_names() override;
      protected:
        void bind_inputs(sensor_table& a_sensor_table) override;
        vector<series> fetch(const vector<int>& a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column) override; //added for calibration
        void fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column) override;
      protected:
        string f_db_uri;
        vector<string> f_basename;
        string f_input_delimiters, f_output_delimiter;
      protected:
        pgsql f_pgsql;
        map<int, pair<string, string>> f_endpoint_n_field_table; //Nobel: mapping of sensor_id to (endpoint, field)
        //map<int, string> f_field_table; // Nobel: sensor_id -> field_preference ("raw" or "calibrated")
        vector<string> f_data_names;
      protected:
        bool f_has_idmap;
        string f_sensorname_column;
    };

    
    class csv_file: public data_source {
      public:
        vector<string> get_data_names() override;
      protected:
        void fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column) override;
      protected:
        map<int, unsigned> f_column_map;
    };

}
#endif
