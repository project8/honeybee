// bb-get-data.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <tabree/KArgumentList.h>
#include "honeybee.hh"

namespace hb = honeybee;


int main(int argc, char** argv)
{
    //// Program Arguments ////
    
    tabree::KArgumentList args(argc, argv);
    if (args.ParameterList().empty() || ! args["--help"].IsVoid()) {
        std::cerr << "USAGE: " << argv[0];
        std::cerr << " SENSOR+ [--from=DATETIME_UTC|--length=SEC] [--to=DATETIME_UTC|--to-ts=UNIXTIME] [OPTIONS]" << std::endl;
        std::cerr << "  SENSOR: sensor name(s), use list-sensors command for defined sensors" << std::endl;
        std::cerr << "  DATETIME: yyyy-mm-ddThh:mm:ss (ex: 2020-10-28T18:51:32)" << std::endl;
        std::cerr << "Other Options:" << std::endl;
        std::cerr << "  --config=FILE            config file (sensor table etc)" << std::endl;
        std::cerr << "  --dripline-db=DB_URI     dripline database" << std::endl;
        std::cerr << "  --series                 output time-series of each sensor"<< std::endl;
        std::cerr << "  --resample=SEC,REDUCER   resampling interval and reducer" << std::endl;
        std::cerr << "  --summary=REDUCER+       output n,mean,std,sem,min,max,first,last"<< std::endl;
        std::cerr << "  --var-KEY=VALUE          set parameter values (used in config files)"<< std::endl;
        std::cerr << "  --verbose                make it verbose"<< std::endl;
        return -1;
    }
    
    std::vector<std::string> t_sensor_names;
    for (std::string t_name: args.ParameterList()) {
        t_sensor_names.push_back(t_name);
    }
    
    std::string t_config_file = args["--config"].Or("");
    std::string t_dripline_db = args["--dripline-db"].Or("");
    
    double t_to_ts = args["--to-ts"].Or(long(hb::datetime::now()));
    std::string t_to = args["--to"].Or(hb::datetime(t_to_ts).as_string());
    double length = args["--length"].Or(60);
    std::string t_from_ts = args["--from-ts"].Or(long(hb::datetime(t_to)-length));
    std::string t_from = args["--from"].Or(hb::datetime(t_from_ts).as_string());
    if (
        (args["--to-ts"].IsVoid() && args["--to"].IsVoid()) &&
        (! args["--from-ts"].IsVoid() || ! args["--from"].IsVoid())
    ){
        t_to = hb::datetime((hb::datetime(t_from)+length)).as_string();
    }
            
    bool t_output_series = ! args["--series"].IsVoid();
    
    double t_resampling_enabled = ! args["--resample"].IsVoid();
    double t_resampling_interval = args["--resample"].SplitBy(",")[0].Or(0); // 0 for auto
    std::string t_resampling_reducer = args["--resample"].SplitBy(",")[1].Or("first");
    
    bool t_output_summary = ! args["--summary"].IsVoid();
    std::vector<std::string> t_summary_items; {
        for (auto t_item: args["--summary"].SplitBy(",")) {
            t_summary_items.push_back(t_item.second);
        }
    }
    if (t_output_summary && t_summary_items.empty()) {
        t_summary_items = {{"n", "mean", "std"}};
    }
    
    std::vector<std::pair<std::string, std::string>> t_variables; {
        for (auto param: args.OptionTable()) {
            if (param.first.substr(0, 6) == "--var-") {
                t_variables.emplace_back(param.first.substr(6), param.second);
            }
        }
    }
    
    if (! args["--verbose"].IsVoid()) {
        hb::g_log_level = hb::e_log_level_debug;
    }

    
    //// Fetch ////

    hb::honeybee_app t_honeybee_app;
    t_honeybee_app.add_config_file(t_config_file);
    t_honeybee_app.add_dripline_db(t_dripline_db);
    for (auto& variable: t_variables) {
        t_honeybee_app.add_variable(variable.first, variable.second);
    }
    
    auto t_series_bundle = t_honeybee_app.read(t_sensor_names, hb::datetime(t_from), hb::datetime(t_to));

    
    //// Reducing (if necessary)  ////
    std::map<std::string, std::function<double(const hb::series&)>> t_reducer_list = {
        {"mean", hb::reduce_to_mean},
        {"std", hb::reduce_to_std},
        {"sem", hb::reduce_to_sem},
        {"min", hb::reduce_to_min},
        {"max", hb::reduce_to_max},
        {"median", hb::reduce_to_mean},
        {"count", hb::reduce_to_count},
        {"sum", hb::reduce_to_sum},
        {"first", hb::reduce_to_first},
        {"last", hb::reduce_to_last},
        {"middle", hb::reduce_to_middle}
    };

    // output summary (reduced values) //
    if (t_output_summary) {
        std::string row_delim = "", col_delim="";
        std::cout << "{"; 
        for (auto t_iter: t_series_bundle.items()) {
            std::cout << row_delim << std::endl; row_delim = ","; col_delim=" ";
            std::cout << "    \"" << t_iter.first << "\": ";
            std::cout << "{";
            for (const std::string& t_item_name: t_summary_items) {
                std::cout << col_delim; col_delim=", ";
                std::cout << "\"" << t_item_name << "\": ";
                double x = t_iter.second.reduce(t_reducer_list[t_item_name]);
                if (std::isnan(x)) {
                    std::cout << "null";
                }
                else {
                    std::cout << x;
                }
            }
            std::cout << " }";
        }
        std::cout << std::endl << "}" << std::endl;

        return 0;
    }

            
    //// Resampling (if necessary) ////

    if (! t_output_series && t_series_bundle.size() > 1) {
        t_resampling_enabled = true;
    }
    
    hb::data_frame t_data_frame;
    if (t_resampling_enabled) {
        auto reducer = t_reducer_list[t_resampling_reducer];
        if (! reducer) {
            reducer = hb::reduce_to_middle;
        }
        if (t_resampling_interval > 0) {
            t_data_frame = hb::data_frame(
                t_series_bundle,
                hb::resampler(hb::group_by_time(t_resampling_interval), reducer)
            );
        }
        else {
            t_data_frame = hb::data_frame(
                t_series_bundle,
                hb::resampler(hb::group_to_align(t_series_bundle), reducer)
            );
        }
    }

    
    //// Outputs ////

    // output series in JSON //
    if (t_output_series) {
        std::string row_delim = "";
        std::cout << "{"; 
        for (unsigned i: honeybee::arange(t_series_bundle)) {
            std::cout << row_delim << std::endl; row_delim = ",";
            std::cout << "    \"" << t_series_bundle.keys()[i] << "\": ";
            if (! t_resampling_enabled) {
                std::cout << t_series_bundle[i].to_json("    ");
            }
            else {
                // resampled series stored in dataframe
                std::cout << t_data_frame.columns()[i].to_json("    ");
            }
        }
        std::cout << std::endl << "}" << std::endl;
    }

    // output dataframe in CSV //
    else if (t_resampling_enabled) {
        std::cout << t_data_frame.to_csv();
    }
    
    // output single time-series in CSV //
    else if (t_series_bundle.size() == 1) {
        std::cout << t_series_bundle[0].to_csv(t_series_bundle.keys()[0]);
    }
    
    return 0;
}
