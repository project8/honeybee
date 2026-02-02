/*
 * sensor_config_by_ktf.hh
 */

#ifndef HONEYBEE_SENSOR_CONFIG_BY_KTF_HH_
#define HONEYBEE_SENSOR_CONFIG_BY_KTF_HH_ 1

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <tabree/KVariant.h>
#include "sensor_config.hh"

namespace kebap {
    class KPStandardParser;
}

namespace honeybee {
    using namespace std;
    
    // Context for recursive load_layer traversal
    struct load_context {
        deque<string> f_name, f_label;
        deque<pair<string, string>> f_opts;
    };
    
    class sensor_config_by_ktf : public sensor_config {
      public:
        using variables = vector<pair<string, tabree::KVariant>>;
        
        sensor_config_by_ktf();
        virtual ~sensor_config_by_ktf();
        
        void set_variables(const variables& a_variables);
        const variables& get_variables() const { return f_variables; }
        void load(sensor_table& a_table, const string& a_filename) override;
        
      private:
        
        // shared_ptr<kebap::KPParser> f_parser;
        shared_ptr<kebap::KPStandardParser> f_standard_parser;
        string f_ktf_path;
        // honeybee likely passes runtime variables to do validation guard checks for all sensor loaders
        // was part of older design
        variables f_variables;
        
        string extract_scripts();
        void load_layer(const tabree::KTree& a_node, sensor_table& a_table);
        void add_sensor(sensor_table& a_table, const tabree::KTree& a_node,
                       const load_context& a_context);
        static void load_layer_implement(sensor_config_by_ktf* a_loader, const tabree::KTree& a_node, 
                                    sensor_table& a_table, load_context& a_context);
    };
}

#endif
