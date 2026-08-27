// hb-list-sensors.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <tabree/KArgumentList.h>
#include "honeybee.hh"
#include "error_logger.hh"

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
        std::cerr << "  --calibration-uri=URI    calibration database" << std::endl;
        std::cerr << "  --fields                 list of sensor data fields to display"<< std::endl;
        std::cerr << "  --var-KEY=VALUE          set parameter values (used in config files)"<< std::endl;
        std::cerr << "  --delimiter=VALUE        set channel name delimiter"<< std::endl;
        std::cerr << "  --delimiter-input=VALUE  set channel name delimiter in the data store"<< std::endl;
        std::cerr << "  --delimiter-output=VALUE set channel name delimiter for output"<< std::endl;
        std::cerr << "  --verbose                make it verbose"<< std::endl;
        std::cerr << "  --log-mode=MODE          all|first|counted|summary"<< std::endl;
        std::cerr << "  --log-level=LEVEL        set minimum log level: panic,error,warn,info,debug or 1..5" << std::endl;
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
    std::string t_calibration_uri = args["--calibration-uri"].Or("");
    std::string t_delimiter = args["--delimiter"].Or("");
    std::string t_delimiter_input = args["--delimiter-input"].Or(t_delimiter);
    std::string t_delimiter_output = args["--delimiter-output"].Or(t_delimiter.substr(0,1));
    
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
    
    auto& t_logger = hb::error_logger::instance();
    std::string t_log_mode = args["--log-mode"].Or("");
    if (t_log_mode == "all") {
        t_logger.set_message_mode(hb::error_logger::e_message_all);
    }
    else if (t_log_mode == "first") {
        t_logger.set_message_mode(hb::error_logger::e_message_first_only);
    }
    else if (t_log_mode == "counted") {
        t_logger.set_message_mode(hb::error_logger::e_message_first_with_count);
    }
    else if (t_log_mode == "summary") {
        t_logger.set_message_mode(hb::error_logger::e_message_summary);
    }

    auto parse_log_level = [](const std::string& s) -> hb::log_level_t {
        if (s.empty()) {
            return hb::e_log_level_warn;
        }
        std::string t = s;
        for (auto &c: t) c = std::tolower(c);
        if (t == "panic" || t == "1") {
            return hb::e_log_level_panic;
        }
        if (t == "error" || t == "2") {
            return hb::e_log_level_error;
        }
        if (t == "warn"  || t == "3") {
            return hb::e_log_level_warn;
        }
        if (t == "info"  || t == "4") {
            return hb::e_log_level_info;
        }
        if (t == "debug" || t == "5") {
            return hb::e_log_level_debug;
        }
        return hb::e_log_level_warn;
    };

    std::string t_log_level = args["--log-level"].Or("");
    if (! t_log_level.empty()) {
        t_logger.set_min_level(parse_log_level(t_log_level));
    }
    else if (! args["--verbose"].IsVoid()) {
        t_logger.set_min_level(hb::e_log_level_info);
    }
    
    
    //// Construction ////

    hb::honeybee_app t_honeybee_app;
    t_honeybee_app.add_config_file(t_config_file);
    t_honeybee_app.add_dripline_db(t_dripline_db);
    t_honeybee_app.add_calibration_uri(t_calibration_uri);
    t_honeybee_app.set_delimiter(t_delimiter_input, t_delimiter_output);
    for (auto& variable: t_variables) {
        t_honeybee_app.add_variable(variable.first, variable.second);
    }
    auto t_sensor_table = t_honeybee_app.get_sensor_table();
    t_delimiter_output = t_honeybee_app.get_output_delimiter();
    

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
        std::cout << "    " << (*t_sensor_table)[t_number].to_json(t_fields, t_delimiter_output);
        delimiter = ",";
    }
    std::cout << std::endl << "]" << std::endl;

    t_logger.create_summary();

    return 0;
}
