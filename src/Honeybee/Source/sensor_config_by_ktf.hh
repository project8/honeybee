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
#include "calibration_accessor.hh"
#include "db_calibration.hh"

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

        void set_cal_source(const std::string& t_uri);
        shared_ptr<calibration> create_calibration(sensor& t_sensor, sensor_table& t_sensor_table, const std::string& t_entity_key);


        
      private:
        shared_ptr<kebap::KPStandardParser> f_standard_parser;
        string f_ktf_path;
        // Runtime variables passed from caller for guard condition evaluation (e.g., device state checks)
        variables f_variables;
        shared_ptr<calibration_accessor> f_cal_accessor;
        string f_calibration_source_uri;
        
        string extract_scripts();
        void load_layer(sensor_table& a_table, const tabree::KTree& a_node, load_context& a_context);
        void add_sensor(sensor_table& a_table, const tabree::KTree& a_node,
                       const load_context& a_context);
        
        shared_ptr<calibration_accessor> create_cal_accessor(const std::string& t_uri);
        shared_ptr<calibration> create_db_calibration(sensor& t_sensor, sensor_table& t_sensor_table,
                                                      const std::string& t_entity_key);
        shared_ptr<calibration> create_kebap_calibration(sensor& t_sensor, sensor_table& t_sensor_table,
                                                      int t_line_number);
    };
}

#endif
