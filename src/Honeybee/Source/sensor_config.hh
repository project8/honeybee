/*
 * sensor_config.hh
 */

#ifndef HONEYBEE_SENSOR_CONFIG_HH_
#define HONEYBEE_SENSOR_CONFIG_HH_ 1

#include <string>
#include <vector>
#include "sensor_table.hh"

namespace honeybee {
    using namespace std;
    
    class sensor_table;
    
    class sensor_config {
      public:
        virtual ~sensor_config() {}
        
        // Load sensors from file, populate sensor_table
        virtual void load(sensor_table& a_table, const string& a_filename) = 0;
        
      protected:
        sensor_config() {}
    };

    class sensor_config_by_names {
      public:
        sensor_config_by_names(const string& a_name_space=""): f_name_space(a_name_space), f_input_delimiters("/.-_"), f_output_delimiter(".") {}
        void set_delimiters(const string& a_delimiters, const string& f_output_delimiter);
        void load(sensor_table& a_table, const vector<string>& a_name_list, name_chain a_basename=name_chain());
      protected:
        string f_name_space; 
        vector<string> f_basenames;
        string f_input_delimiters, f_output_delimiter;
    };
}

#endif
