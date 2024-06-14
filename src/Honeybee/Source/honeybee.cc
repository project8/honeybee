/*
 * honeybee.cc
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */


#include <tabree/KTreeFile.h>
#include "honeybee.hh"

using namespace std;
using namespace honeybee;


honeybee_app::honeybee_app()
{
    f_variables.emplace_back("version", 1);
    f_variables.emplace_back("date", datetime::now().as_string("%Y%m%d"));
    
    f_default_delimiters = "./:;-_";
    f_input_delimiters = "";
    f_output_delimiter = "";
   
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

void honeybee_app::add_variable(const string& key, const tabree::KVariant& value)
{
    f_variables.emplace_back(key, value);
}

void honeybee_app::set_delimiter(const std::string& input_delimiters, const std::string& output_delimiter)
{
    if (! input_delimiters.empty()) {
        f_input_delimiters = input_delimiters;
    }
    if (! output_delimiter.empty()) {
        f_output_delimiter = output_delimiter;
    }
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
        catch (std::exception &e) {
            hERROR(cerr << e.what());
            return;
        }
    }
    if (! f_dripline_db_uri.empty()) {
        t_config["data_source"]["dripline_psql"]["uri"] = f_dripline_db_uri;
    }

    if (! f_config_file_path.empty()) {
        hINFO(cerr << "loading " << f_config_file_path << endl);
        sensor_config_by_file t_sensor_config;
        t_sensor_config.set_variables(f_variables);
        t_sensor_config.load(*f_sensor_table, f_config_file_path);
        hINFO(cerr << f_sensor_table->find_like({{}}).size() << " sensors defined" << endl);
    }

    if (t_config["data_source"]["dripline_psql"]["uri"].IsVoid()) {
        hINFO(cerr << "No data source defined");
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
        
    string t_db_uri = t_config["data_source"]["dripline_psql"]["uri"];
    string t_basename = t_config["data_source"]["dripline_psql"]["basename"];
    if (t_db_uri.empty()) {
        hERROR(cerr << "No Dripline Datasource found" << endl);
    }
    else {
        hINFO(cerr << "Dripline Datasource: " << t_db_uri << endl);
        f_data_source = make_shared<dripline_pgsql>(
            t_db_uri, name_chain{t_basename, f_input_delimiters}, f_input_delimiters, f_output_delimiter
        );
    }
    
    f_data_source->bind(*f_sensor_table);
}

std::vector<std::string> honeybee_app::find_like(const std::string a_name)
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

series_bundle honeybee_app::read(const vector<std::string>& a_sensor_list, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer)
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
            hINFO(cerr << "undefined sensor name: " << t_name << endl);
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

    hINFO(cerr << "getting data ");
    hINFO(cerr << "(" << datetime(a_from).as_string() << " to " << datetime(a_to).as_string() << ", ");
    hINFO(cerr << t_sensor_number_list.size() << " sensors)..." << flush);
    datetime start = datetime::now();
    vector<series> t_series_list = f_data_source->read(
        t_sensor_number_list, datetime(a_from), datetime(a_to),
        a_resampling_interval, a_reducer
    );
    datetime stop = datetime::now();
    hINFO(cerr << "done. (" << (stop-start) << " s)" << endl);

    // Resampling might have be done on the server-side, might not.
    // We will perform resampling on the returned result here; server-side resampling is to reduce the data size.
    return hb::zip(std::move(t_sensor_name_list), std::move(t_series_list));
}



#include <cstdlib>
#include <dirent.h>

void honeybee_app::find_default_config()
{
    auto path = getenv("HONEYBEE_CONFIG_PATH");
    if (path == NULL) {
        return;
    }
    hINFO(cerr << "HONEYBEE_CONFIG_PATH: " << path << endl);

    vector<string> file_list; {
        DIR *dir;
        if ((dir = opendir(path)) == NULL) {
            hERROR(cerr << "unable to open dir: " << path << endl);
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
            hINFO(cerr << "Adding config file: " << string(path) + "/" + file << endl);
        }
    }
}
