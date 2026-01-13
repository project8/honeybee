/*
 * sensor_config_by_ktf.hh
 */

#ifndef HONEYBEE_SENSOR_CONFIG_BY_KTF_HH_
#define HONEYBEE_SENSOR_CONFIG_BY_KTF_HH_ 1

#include <string>
#include <vector>
#include <memory>
#include <tabree/KVariant.h>
#include "sensor_config.hh"

namespace kebap {
    class KPParser;
}

namespace honeybee {
    using namespace std;
    
    class sensor_config_by_ktf : public sensor_config {
      public:
        using variables = vector<pair<string, tabree::KVariant>>;
        
        sensor_config_by_ktf();
        virtual ~sensor_config_by_ktf();
        
        void set_variables(const variables& a_variables);
        void load(sensor_table& a_table, const string& a_filename) override;
        
      private:
        
        shared_ptr<kebap::KPParser> f_parser;
        string f_ktf_path;
        // honeybee likely passes runtime variables to do validation guard checks for all sensor loaders
        // was part of older design
        variables f_variables;
        
        string extract_scripts();
        void load_layer(const tabree::KVariant& a_node, sensor_table& a_table);
        void add_sensor(const tabree::KVariant& a_node, sensor_table& a_table, int a_line_offset);
    };
}

#endif
