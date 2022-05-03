// hb-list-sensors.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include <iostream>
#include <tabree/KArgumentList.h>
#include "honeybee.hh"

namespace hb = honeybee;


int main(int argc, char** argv)
{
    //// Program Arguments ////
    
    tabree::KArgumentList args(argc, argv);
    if (! args["--help"].IsVoid()) {
        std::cerr << "USAGE: " << argv[0];
        std::cerr << " SENSOR+ [OPTIONS]" << std::endl;
        std::cerr << "  SENSOR: sensor name(s), use list-sensors command for defined sensors" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  --config=FILE            config file (sensor table etc)" << std::endl;
        std::cerr << "  --dripline-db=DB_URI     dripline database" << std::endl;
        std::cerr << "  --fields                 list of sensor data fields to display"<< std::endl;
        std::cerr << "  --var-KEY=VALUE          set parameter values (used in config files)"<< std::endl;
        std::cerr << "  --verbose                make it verbose"<< std::endl;
        return -1;
    }

    
    std::vector<std::string> t_sensor_names;
    for (std::string t_name: args.ParameterList()) {
        t_sensor_names.push_back(t_name);
    }
    if (t_sensor_names.empty()) {
        t_sensor_names.push_back("");
    }
    
    std::string t_config_file = args["--config"].Or("");
    std::string t_dripline_db = args["--dripline-db"].Or("");
    
    std::vector<std::string> t_fields; {
        if (args["--fields"].As<std::string>() != "ALL") {
            for (auto f: args["--fields"].SplitBy(",")) {
                t_fields.push_back(f.second);
            }
            if (t_fields.empty()) {
                t_fields = {{"number", "name", "default_calibration", "options"}};
            }
        }
    }
    
    std::vector<std::pair<std::string, std::string>> t_variables; {
        for (auto param: args.OptionTable()) {
            if (param.first.substr(0, 6) == "--var-") {
                t_variables.emplace_back(param.first.substr(6), param.second);
            }
        }
    }
    
    if (! args["--verbose"].IsVoid()) {
        hb::g_log_level = hb::e_log_level_info;
    }
    
    
    //// Construction ////

    hb::honeybee_app t_honeybee_app;
    t_honeybee_app.add_config_file(t_config_file);
    t_honeybee_app.add_dripline_db(t_dripline_db);
    for (auto& variable: t_variables) {
        t_honeybee_app.add_variable(variable.first, variable.second);
    }
    auto t_sensor_table = t_honeybee_app.get_sensor_table();
    

    //// Serach in Sensor Table ////
    
    std::set<int> t_sensors;
    for (auto& t_name: t_sensor_names) {
        for (auto& t_number: t_sensor_table->find_like(hb::name_chain(t_name, "./-_"))) {
            t_sensors.insert(t_number);
        }
    }

    
    //// Output ////
    
    std::string delimiter = "";
    std::cout << "[";
    for (auto& t_number: t_sensors) {
        std::cout << delimiter << std::endl;
        std::cout << "    " << (*t_sensor_table)[t_number].to_json(t_fields);
        delimiter = ",";
    }
    std::cout << std::endl << "]" << std::endl;

    return 0;
}
