/*
 * honeybee.cc
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */


#include <functional>
#include <set>
#include <tabree/KTreeFile.h>
#include "honeybee.hh"
#include "error_logger.hh"
#include "sensor_config_by_ktf.hh"
#include "system_config.hh"
#include "psql_calibration_accessor.hh"
#include "calibration_factory.hh"

using namespace std;
using namespace honeybee;


honeybee_app::honeybee_app()
{
    f_variables.emplace_back("version", 1);
    f_variables.emplace_back("date", datetime::now().as_string("%Y%m%d"));
    
    f_default_delimiters = "./:;-_";
    f_input_delimiters = "";
    f_output_delimiter = "";
    f_value_column_default = "";
   
    f_sensor_table = make_shared<sensor_table>();
    f_data_source = make_shared<empty_data_source>();
    
    f_is_constructed = false;
}

void honeybee_app::add_config_file(const string& filepath)
{
    f_config_file_path = filepath;
}

void honeybee_app::add_dripline_db(const string& db_uri)
{
    f_dripline_db_uri = db_uri;
}

void honeybee_app::add_calibration_uri(const string& uri)
{
    f_calibration_uri = uri;
}

void honeybee_app::add_variable(const string& key, const tabree::KVariant& value)
{
    f_variables.emplace_back(key, value);
}

void honeybee_app::set_delimiter(const string& input_delimiters, const string& output_delimiter)
{
    if (! input_delimiters.empty()) {
        f_input_delimiters = input_delimiters;
    }
    if (! output_delimiter.empty()) {
        f_output_delimiter = output_delimiter;
    }
}

void honeybee_app::set_value_column_default(const string& value_column)
{
    f_value_column_default = value_column;
}

shared_ptr<sensor_table> honeybee_app::get_sensor_table()
{
    if (! f_is_constructed) {
        construct();
    }
    
    return f_sensor_table;
}

shared_ptr<data_source> honeybee_app::get_data_source()
{
    if (! f_is_constructed) {
        construct();
    }
    
    return f_data_source;
}

void honeybee_app::construct()
{
    if (f_is_constructed) {
        return;
    }
    f_is_constructed = true;
    
    tabree::KTree t_config;
    if (f_config_file_path.empty() && f_dripline_db_uri.empty()) {
        this->find_default_config();
    }
    
    if (! f_config_file_path.empty()) {
        try {
            tabree::KTreeFile(f_config_file_path).Read(t_config);
        }
        catch (exception &e) {
            hERROR(e.what());
            return;
        }
    }
    if (! f_dripline_db_uri.empty()) {
        t_config["data_source"]["dripline_psql"]["uri"] = f_dripline_db_uri;
    }

    // accesor creation steps 
    system_config t_system_config(t_config);
    string t_calibration_uri = f_calibration_uri;
    if (t_calibration_uri.empty()) {
        const char* t_env_uri = getenv("HONEYBEE_CALIBRATION_URI");
        if (t_env_uri) {
            t_calibration_uri = t_env_uri;
        }
    }
    if (t_calibration_uri.empty()) {
        t_calibration_uri = t_system_config.calibration_uri();
    }

    shared_ptr<calibration_accessor> t_calibration_accessor;
    if (! t_calibration_uri.empty()) {
        t_calibration_accessor = make_shared<psql_calibration_accessor>(t_calibration_uri);
    }

    auto t_calibration_factory = make_shared<calibration_factory>(
        t_calibration_accessor);

    if (! f_config_file_path.empty()) {
        hINFO("loading " << f_config_file_path);
        auto t_loader = make_shared<sensor_config_by_ktf>(t_calibration_factory);
        t_loader->set_variables(f_variables);
        t_loader->load(*f_sensor_table, f_config_file_path);
        f_loaders[f_config_file_path] = t_loader;
        hINFO(f_sensor_table->find_like({{}}).size() << " sensors defined");
    }

    const string t_db_uri = t_system_config.data_source_uri();
    if (t_db_uri.empty()) {
        hINFO("No data source defined");
    }

    if (f_input_delimiters.empty()) {
        if (! t_config["data_source"]["dripline_psql"]["delimiter"].IsVoid()) {
            f_input_delimiters = t_config["data_source"]["dripline_psql"]["delimiter"].As<string>();
        }
        else if (! t_config["options"]["delimiter_input"].IsVoid()) {
            f_input_delimiters = t_config["options"]["delimiter_input"].As<string>();
        }
        else if (! t_config["options"]["delimiter"].IsVoid()) {
            f_input_delimiters = t_config["options"]["delimiter"].As<string>();
        }
        else {
            f_input_delimiters = f_default_delimiters;
        }
    }
    if (f_output_delimiter.empty()) {
        if (! t_config["options"]["delimiter_output"].IsVoid()) {
            f_output_delimiter = t_config["options"]["delimiter_output"].As<string>();
        }
        else if (! t_config["options"]["delimiter"].IsVoid()) {
            f_output_delimiter = t_config["options"]["delimiter"].As<string>().substr(0, 1);
        }
        else {
            f_output_delimiter = f_input_delimiters.substr(0, 1);
        }
    }
        
    const string t_basename = t_system_config.data_source_basename();
    if (t_db_uri.empty()) {
        hERROR("No Dripline Datasource found");
    }
    else {
        hINFO("Dripline Datasource: " << t_db_uri);
        f_data_source = make_shared<dripline_pgsql>(
            t_db_uri, name_chain{t_basename, f_input_delimiters}, f_input_delimiters, f_output_delimiter
        );
    }
    
    f_data_source->bind(*f_sensor_table);
}

vector<string> honeybee_app::find_like(const string a_name)
{
    vector<string> t_name_list;
    
    if (! f_is_constructed) {
        construct();
    }
    if (! f_sensor_table || ! f_data_source) {
        return t_name_list;
    }

    auto t_matched_sensors = f_sensor_table->find_like(name_chain(a_name, f_input_delimiters));
    for (auto& t_number: t_matched_sensors) {
        t_name_list.push_back((*f_sensor_table)[t_number].get_name().join(f_output_delimiter));
    }

    return t_name_list;
}


series_bundle honeybee_app::read(const vector<string>& a_sensor_list, double a_from, double a_to, double a_resampling_interval, const string& a_reducer)
{
    if (! f_is_constructed) {
        construct();
    }
    if (! f_sensor_table || ! f_data_source) {
        return series_bundle();
    }

    vector<string> t_sensor_name_list;
    vector<int> t_sensor_number_list;
    for (auto& t_name: a_sensor_list) {
        auto t_matched_sensors = f_sensor_table->find_like(name_chain(t_name, f_input_delimiters));
        if (t_matched_sensors.empty()) {
            hINFO("undefined sensor name: " << t_name);
            t_sensor_number_list.push_back(0);
            t_sensor_name_list.push_back(t_name);
            continue;
        }
        if (t_matched_sensors.size() > 1) {
            for (auto& t_number: t_matched_sensors) {
                t_sensor_number_list.push_back(t_number);
                t_sensor_name_list.push_back((*f_sensor_table)[t_number].get_name().join(f_output_delimiter));
            }
        }
        else {
            t_sensor_number_list.push_back(t_matched_sensors.front());
            t_sensor_name_list.push_back(t_name);
        }
    }

    hINFO("getting data ");
    hINFO("(" << datetime(a_from).as_string() << " to " << datetime(a_to).as_string() << ")...");
    datetime start = datetime::now();
    
    vector<series> t_series_list;
    try {
        string t_default_column = f_value_column_default;
        if (t_default_column.empty()) {
            t_default_column = "value_raw";
        }
        t_series_list = f_data_source->read(
            t_sensor_number_list, t_default_column, a_from, a_to,
            a_resampling_interval, a_reducer
        );
    }
    catch (exception& e) {
        hERROR("Error reading data: " << e.what());
        return series_bundle();
    }
    
    datetime stop = datetime::now();
    hINFO("done. (" << (stop-start) << " s)");

    
    // Resampling might have be done on the server-side, might not.
    // We will perform resampling on the returned result here; server-side resampling is to reduce the data size.
    // combine sensor name w/ its data 
    return hb::zip(move(t_sensor_name_list), move(t_series_list));
}



#include <cstdlib>
#include <dirent.h>

void honeybee_app::find_default_config()
{
    auto path = getenv("HONEYBEE_CONFIG_PATH");
    if (path == NULL) {
        return;
    }
    hINFO("HONEYBEE_CONFIG_PATH: " << path);

    vector<string> file_list; {
        DIR *dir;
        if ((dir = opendir(path)) == NULL) {
            hERROR("unable to open dir: " << path);
            return;
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                file_list.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }

    for (string file: file_list) {
        if (file.substr(file.size()-4) == ".ktf") {
            this->add_config_file(string(path) + "/" + file);
            hINFO("Adding config file: " << string(path) + "/" + file);
        }
    }
}
