/*
 * data_source.cc
 *
 *  Created on: Oct 19, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <regex>
#include <limits>
#include <stdexcept>
#include "sensor_table.hh"
#include "sensor_config.hh"
#include "pgsql.hh"
#include "error_logger.hh"
#include "data_source.hh"

using namespace std;
using namespace honeybee;


static string sanitize(const string& text, const string& pattern=R"([a-zA-Z0-9_]+)")
{
    auto& t_logger = error_logger::instance();
    try {
        regex re(pattern);
        if (! regex_match(text, re)) {
            string t_message = string("sanitization fault (pattern: ") + pattern + "): " + text;
            t_logger.error("data_source", "sanitization_fault", t_message);
            throw std::runtime_error(t_message);
        }
    }
    catch (const std::regex_error& e) {
        string t_message = string("sanitization fault: ") + e.what() + ": " + text;
        t_logger.error("data_source", "sanitization_fault", t_message);
        throw std::runtime_error(t_message);
    }

    return text;
}





void data_source::bind(sensor_table& a_sensor_table)
{
    // NOTE: Calibration objects are now created and attached directly when loading KTF files
    // via sensor_config_by_ktf and kebap_calibration.
    hINFO("Calibration Chain (from sensor attached objects):");
    for (int t_sensor_number: a_sensor_table.find_like({{}})) {
        auto& t_sensor = a_sensor_table[t_sensor_number];
        if (t_sensor.get_calibration().empty()) {
            continue;
        }
        hINFO(
              "    " << t_sensor.get_name().join(".") << " : "
              << t_sensor.get_calibration()
        );
        
        // Store calibration object in f_calibration_table for orchestration
        auto t_calib_obj = t_sensor.get_calibration_object();
        if (t_calib_obj) {
            f_calibration_table[t_sensor_number] = t_calib_obj;
        }
    }
    
    this->bind_inputs(a_sensor_table);
}

vector<series> data_source::read(const vector<int>& a_sensor_list, const std::string& value_column, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer)
{

    
    // Find the base (input) sensors for each requested sensor, resolve dependencies
    vector<int> t_input_sensor_list;
    for (unsigned i = 0; i < a_sensor_list.size(); i++) {
        t_input_sensor_list.emplace_back(find_input(a_sensor_list[i]));
    }

    // Fetch data from base sensors
    vector<series> t_series_list = this->fetch(t_input_sensor_list, a_from, a_to, a_resampling_interval, a_reducer, value_column);
    
    // Apply calibrations for each requested sensor
    for (unsigned i = 0; i < a_sensor_list.size(); i++) {
        apply_calibration(a_sensor_list[i], t_series_list[i]);
    }

    return t_series_list;
}


int data_source::find_input(int a_sensor)
{
    auto iter = f_calibration_table.find(a_sensor);
    if (iter == f_calibration_table.end()) {
        return a_sensor;
    }
    
    const auto& t_calib = iter->second;
    return this->find_input(t_calib->get_input_sensor());
}


void data_source::apply_calibration(int a_sensor, series& a_series)
{
    // Recursive orchestration: applies calibrations from input sensor up through the dependency chain
    auto& t_logger = error_logger::instance();
    auto iter = f_calibration_table.find(a_sensor);
    if (iter == f_calibration_table.end()) {
        return;  // Base sensor - no calibration
    }
    
    const auto& t_calib = iter->second;
    
    // Recursively apply calibration to input sensor first
    this->apply_calibration(t_calib->get_input_sensor(), a_series);
    
    // Then apply this sensor's calibration to all values
    try {
        for (auto& xk: a_series.x()) {
            xk = t_calib->operator()(xk);
        }
    }
    catch (exception& e) {
        string t_message = string("Sensor ID ") + to_string(a_sensor) + ": " + e.what();
        t_logger.error("data_source", "calibration_apply_error", t_message);
        throw runtime_error(t_message);
    }
    hINFO("Calibration: " << t_calib->get_description());
}


vector<series> data_source::fetch(const vector<int>& a_sensor_list, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
    // default implemantation, might be overriden as needed //
    
    vector<series> t_series_list;
    for (auto& t_sensor: a_sensor_list) {
        t_series_list.emplace_back(a_from, a_to);
        fetch_single(t_series_list.back(), t_sensor, a_from, a_to, a_resampling_interval, a_reducer, value_column);
    }

    return t_series_list;
}



dripline_pgsql::dripline_pgsql(string a_uri, name_chain a_basename, const string& a_input_delimiters, const string& a_output_delimiter)
: f_db_uri(a_uri), f_basename(a_basename.get_chain()), f_input_delimiters(a_input_delimiters), f_output_delimiter(a_output_delimiter)
{
    auto& t_logger = error_logger::instance();
    f_pgsql.set_db(f_db_uri);

    f_has_idmap = false; {
        vector<string> t_tables = f_pgsql.get_table_list();
        for (auto& t: t_tables) {
            if (t == "endpoint_id_map") {
                hINFO("Found Dripline ID-Map");
                f_has_idmap = true;
                break;
            }
        }
    }

    f_sensorname_column = ""; {
        vector<string> t_fields = f_pgsql.get_column_list("numeric_data");
        for (auto& f: t_fields) {
            if ((f == "endpoint_name") || (f == "sensor_name")) {
                f_sensorname_column = f;
                break;
            }
        }
    }
    if (f_sensorname_column.empty()) {
        t_logger.error(
            "db",
            "missing_sensorname_column",
            "unable to identify sensor-name column in Dripline Table"
        );
        throw std::runtime_error("unable to identify sensor-name column in Dripline Table");
    }
    hINFO("Dripline Sensor-Name Column: " << f_sensorname_column);
}

vector<string> dripline_pgsql::get_data_names()
{
    if (! f_data_names.empty()) {
        return f_data_names;
    }
        
    hINFO("getting Dripline end-point names...");
    string t_sql = "select distinct " + f_sensorname_column;
    t_sql += (f_has_idmap ? " from endpoint_id_map" : " from numeric_data");
    auto t_handler = [&](int a_row, int a_col, const char* a_value) {
        f_data_names.emplace_back(a_value);
    };
    f_pgsql.query(t_sql, t_handler);
    hINFO("    " << f_data_names.size() << " end-points found.");

    return f_data_names;
}

void dripline_pgsql::bind_inputs(sensor_table& a_sensor_table)
{
    // 1: get data names
    vector<string> t_dripline_names = this->get_data_names();
    hINFO("Dripline Endpoints: ");
    for (auto& name: t_dripline_names) {
        hINFO("    " << name);
    }

    // 2: construct sensor entries from Dripline endpoints
    sensor_config_by_names t_config("dripline_endpoint");
    if (! f_input_delimiters.empty()) {
        t_config.set_delimiters(f_input_delimiters, f_output_delimiter);
    }
    t_config.load(a_sensor_table, t_dripline_names, f_basename);

    // 3: make a Dripline endpoint table
    hINFO("Dripline Endpoint Binding: ");
    set<string> t_endpoint_list(t_dripline_names.begin(), t_dripline_names.end());
    for (int t_number: a_sensor_table.find_like({{}})) { // --> getting all sensors
        const sensor& t_sensor = a_sensor_table[t_number];
        string t_endpoint = t_sensor.get_option("dripline_endpoint", ""); 
        string t_field = t_sensor.get_option("dripline_endpoint_field", ""); // otherwise keep empty so the app default can apply later

        if (t_endpoint_list.count(t_endpoint) > 0) {
            f_endpoint_n_field_table[t_number] = {t_endpoint, t_field}; //---> HERE, STORES THE ENDPOINT MAPPING, like 131 --> {name, field pref(calibrated or raw)}
            hINFO("    " << t_endpoint << " => " << t_sensor.get_name().join(f_output_delimiter));
        }
    }
}

void dripline_pgsql::fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
    try {
        auto t_series_list = this->fetch({a_sensor}, a_from, a_to, a_resampling_interval, a_reducer, value_column); //added for calibration
        if (t_series_list.size() == 1) {
            a_series = std::move(t_series_list[0]);
        }
    }
    catch (std::runtime_error &e) {
        throw e;
    }
}

vector<series> dripline_pgsql::fetch(const vector<int>& a_sensor_list, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
    //seperating endpoints name based on their data type pref using the f_endpoint_n_field_table

    vector<series> t_series_list;
    auto& t_logger = error_logger::instance();

    map<string, map<string, vector<unsigned>>> t_column_index_table;
    map<string, set<string>> t_column_target_sets;
    vector<string> t_valid_columns = f_pgsql.get_column_list("numeric_data");
    // --verbose, in that case you can return, invalid default data column
    // better to have a nan output rather than a crash 

    // so for running, right now, all the metedata that user gets before the actual data 
    // should also be option within the logger class

    // for a repetitive error message, you can have a cache of error messages and when an error comes
    // based on tracked printed error: 

    // These can be options within verbose and error 
    //printing all errors, 
    //first lelvel: do not repeat the error mess
    // second: have a count of haw many times times an error message appear, message -> count mapping 
    // last: summary 

    // having a class to do all this pre-processing error logging, and use that in these situation , a global instance. 
    bool valid_col = find(t_valid_columns.begin(), t_valid_columns.end(), value_column) != t_valid_columns.end();
    if (!valid_col) { 
        for (auto t_sensor: a_sensor_list) {
            // Demo case: same warning 
            hWARN("invalid default data column '" << value_column << "' for sensor '" << t_sensor << "'; returning NaN series");
            t_series_list.emplace_back(a_from, a_to);
            t_series_list.back().emplace_back(a_from, numeric_limits<double>::quiet_NaN());
        }
        return t_series_list;
    }
    
    for (auto t_sensor: a_sensor_list) {
        t_series_list.emplace_back(a_from, a_to);
        auto iter = f_endpoint_n_field_table.find(t_sensor);
        if (iter != f_endpoint_n_field_table.end()) {
            const string& endpoint = iter->second.first; // endpoint_name
            
            // indentify column of endpoint, use default if not specified
            string field = iter->second.second; // field pref
            string t_column = field.empty() ? value_column : field; 

            t_column_target_sets[t_column].insert(endpoint);
            t_column_index_table[t_column][endpoint].push_back(t_series_list.size() - 1);
        }
    }
    
    for (const auto& t_group: t_column_index_table) {
        string t_targets;
        for (const auto& t_endpoint: t_column_target_sets[t_group.first]) {
            t_targets += (t_targets.empty() ? "'" : ",'") + t_endpoint + "'";
        }
        fetch_column(t_series_list, t_group.second, t_targets, t_group.first, a_from, a_to, a_resampling_interval, a_reducer);
    }

    return t_series_list;
}

void dripline_pgsql::fetch_column(vector<series>& a_series_list, const map<string, vector<unsigned>>& a_endpoint_index_table, const string& a_targets, const string& a_column, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer)
{
    auto& t_logger = error_logger::instance();

    if (a_targets.empty()) {
        return;
    }

    string t_sql; {
        string date_from = datetime(a_from).as_string() + "Z";
        string date_to = datetime(a_to).as_string() + "Z";
        string tag = f_sensorname_column;
        string tag_values = a_targets;
        string field = a_column;
        string bucket = std::to_string(a_resampling_interval);
        string to = std::to_string(a_to);
        
#if 0
        string time_selector; {
            static const std::map<std::string, std::string> func_list = {
                {"first", "min"},
                {"last", "max"}
                // missing: middle
            };
            auto iter = func_list.find(a_reducer);
            if (iter != func_list.end()) {
                time_selector = iter->second;
            }
        }
        string value_aggregator; {
            static const std::map<std::string, std::string> func_list = {
                //{"mean", "avg"},     // not correct if non-linear calibration is applied after this
                //{"sum", "sum"},      // not correct if non-linear calibration is applied after this
                //{"std", "stddev"},   // multiple application of this will cause a problem
                //{"count", "count"},  // multiple application of this will cause a problem
                {"min", "min"},        // min and max might flip depending on the calibration 
                {"max", "max"}         // min and max might flip depending on the calibration 
                // missing: median, sem
            };
            auto iter = func_list.find(a_reducer);
            if (iter != func_list.end()) {
                value_aggregator = iter->second;
            }
        }
                
        string cte_data = (string("")
            + "SELECT"
            + "  timestamp, " + tag + ", " + field + " "
            + "FROM"
            + "  numeric_data "
            + "WHERE "
            + "  " + tag + " IN (" + tag_values + ") "
            + "  AND timestamp>='" + date_from + "' AND timestamp<'" + date_to + "'"
        );
        
        if (a_resampling_interval > 0) {
            if (! time_selector.empty()) {
                string cte_last_bucket = (string("")
                    + "SELECT "
                    + "  floor(("+to+"-extract(epoch from timestamp))/"+bucket+") AS bucket, "
                    + "  " + tag + ", "
                    + "  " + time_selector + "(timestamp) AS picked_timestamp "
                    + "FROM "
                    + "  numeric_data "
                    + "WHERE "
                    + "  timestamp>='" + date_from + "' AND timestamp<'" + date_to + "' "
                    + "  AND " + tag + " in (" + tag_values + ") "
                    + "GROUP BY "
                    + "  bucket, " + tag
                );
                t_sql = (string("")
                    + "WITH "
                    + "  cte_bucket AS (" + cte_last_bucket + "), "
                    + "  cte_data AS (" + cte_data + ") "
                    + "SELECT"
                    + "  " + to + "-" + bucket + "*(bucket+0.5) AS timestamp, t." + tag + ", " + field + " "
                    + "FROM"
                    + "  cte_data as t "
                    + "JOIN"
                    + "  cte_bucket as b "
                    + "ON "
                    + "  t.timestamp = b.picked_timestamp AND t." + tag + " = b." + tag + " "
                    + "ORDER BY"
                    + "  timestamp asc "
                );
            }
            else if (! value_aggregator.empty()) {
                string cte_avg_bucket = (string("")
                    + "SELECT"
                    + "  floor((" + to + "-extract(epoch from timestamp))/" + bucket + ") AS bucket, "
                    + "  " + tag + ", "
                    + "  " + value_aggregator + "(" + field + ") AS " + field + " "
                    + "FROM"
                    + "  numeric_data "
                    + "WHERE"
                    + "  timestamp>='" + date_from + "' AND timestamp<'" + date_to + "' "
                    + "  and " + tag + " in (" + tag_values + ") "
                    + "GROUP BY"
                    + "  bucket, " + tag
                );
                t_sql = (string("")
                    + "WITH "
                    + "  cte_bucket AS (" + cte_avg_bucket + ") "
                    + "SELECT"
                    + "  " + to + "-" + bucket + "*(bucket+0.5) AS timestamp, " + tag + ", " + field + " "
                    + "FROM"
                    + "  cte_bucket "
                    + "ORDER BY"
                    + "  timestamp asc "
                );
            }
        }

        if (t_sql.empty()) {
            t_sql = (string("")
                + "WITH"
                + "  cte_data AS (" + cte_data + ") "
                + "SELECT"
                + "  extract(epoch from timestamp), " + tag + ", " + field + " "
                + "FROM"
                + "  cte_data "
                + "ORDER BY"
                + "  timestamp asc"
            );
        }
#else            // Nobel: current direct query path
        t_sql = (string("")
            + "SELECT"
            + "  extract(epoch from timestamp), " + tag + ", " + field + " "
            + "FROM"
            + "  numeric_data "
            + "WHERE "
            + "  " + tag + " IN (" + tag_values + ") "
            + "  AND timestamp>='" + date_from + "' AND timestamp<'" + date_to + "'"
            + "ORDER BY"
            + "  timestamp asc"
        );

        hINFO("SQL: ");
        hINFO("    " << t_sql);

        double time;
        map<string, vector<unsigned>>::const_iterator t_channel_iter;
        auto t_handler = [&](int a_row, int a_col, const char* a_value) {
            if (a_col == 0) {
                time = stod(a_value);
            }
            else if (a_col == 1) {
                t_channel_iter = a_endpoint_index_table.find(a_value);
            }
            else {
                for (unsigned index: t_channel_iter->second) {
                    a_series_list[index].emplace_back(time, stod(a_value));
                }
            }
        };
        if (f_pgsql.query(t_sql, t_handler) < 0) {
            t_logger.error(
                "db",
                "db_query_error",
                string("DB Query Error: SQL: ") + t_sql
            );
            throw std::runtime_error("DB Query Error: SQL: " + t_sql);
        }
#endif
    }
}


vector<string> csv_file::get_data_names()
{
    return vector<string>();
}

void csv_file::fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
}