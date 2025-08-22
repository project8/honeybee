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
#include <regex>
#include "sensor_table.hh"
#include "pgsql.hh"
#include "data_source.hh"

using namespace std;
using namespace honeybee;

// Nobel: global registry for user-defined calibration functions
map<string, UserCalibrateFunction> honeybee::g_user_calibrate_functions;


static string sanitize(const string& text, const string& pattern=R"([a-zA-Z0-9_]+)")
{
    try {
        regex re(pattern);
        if (! regex_match(text, re)) {
            throw std::runtime_error(string("sanitization fault (pattern: ") + pattern + "): " + text);
        }
    }
    catch (std::exception &e) {
        throw std::runtime_error(string("sanitization fault: ") + e.what() + ": " + text);
    }

    return text;
}




void data_source::bind(sensor_table& a_sensor_table)
{
    hINFO(cerr << "Calibration Chain:" << endl);
    for (int t_sensor_number: a_sensor_table.find_like({{}})) {
        auto& t_sensor = a_sensor_table[t_sensor_number];
        if (t_sensor.get_calibration().empty()) {
            continue;
        }
        f_calibration_table[t_sensor_number] = calibration(t_sensor, a_sensor_table);
        hINFO(cerr
              << "    " << t_sensor.get_name().join(".") << " <= "
              << a_sensor_table[f_calibration_table[t_sensor_number].get_input_sensor()].get_name().join(".") << " : "
              << f_calibration_table[t_sensor_number].get_description() << endl
        );
    }
    
    this->bind_inputs(a_sensor_table);
}
//modified for ability to switch between raw and cal column, added for calibration 
vector<series> data_source::read(const vector<int>& a_sensor_list, const std::string& value_column, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer)
{
    vector<int> t_input_sensor_list;
    for (unsigned i = 0; i < a_sensor_list.size(); i++) {
        t_input_sensor_list.emplace_back(find_input(a_sensor_list[i]));
    }

    vector<series> t_series_list = this->fetch(t_input_sensor_list, a_from, a_to, a_resampling_interval, a_reducer, value_column);
    
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
    return this->find_input(t_calib.get_input_sensor());
}

void data_source::apply_calibration(int a_sensor, series& a_series)
{
    auto iter = f_calibration_table.find(a_sensor);
    if (iter == f_calibration_table.end()) {
        return;
    }
    const auto& t_calib = iter->second;
    
    this->apply_calibration(t_calib.get_input_sensor(), a_series);
    
    for (auto& xk: a_series.x()) {
        xk = t_calib(xk);
    }
    hINFO(cerr << "Calibration: " << endl);
    hINFO(cerr << "    " << t_calib.get_description() << endl);
}

//added another 
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
    f_pgsql.set_db(f_db_uri);

    f_has_idmap = false; {
        vector<string> t_tables = f_pgsql.get_table_list();
        for (auto& t: t_tables) {
            if (t == "endpoint_id_map") {
                hINFO(cerr << "Found Dripline ID-Map" << endl);
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
        throw std::runtime_error("unable to identify sensor-name column in Dripline Table");
    }
    hINFO(cerr << "Dripline Sensor-Name Column: " << f_sensorname_column << endl);
}

vector<string> dripline_pgsql::get_data_names()
{
    if (! f_data_names.empty()) {
        return f_data_names;
    }
        
    hINFO(cerr << "getting Dripline end-point names..." << endl);
    string t_sql = "select distinct " + f_sensorname_column;
    t_sql += (f_has_idmap ? " from endpoint_id_map" : " from numeric_data");
    auto t_handler = [&](int a_row, int a_col, const char* a_value) {
        f_data_names.emplace_back(a_value);
    };
    f_pgsql.query(t_sql, t_handler);
    hINFO(cerr << "    " << f_data_names.size() << " end-points found." << endl);

    return f_data_names;
}

void dripline_pgsql::bind_inputs(sensor_table& a_sensor_table)
{
    // 1: get data names
    vector<string> t_dripline_names = this->get_data_names();
    hINFO(cerr << "Dripline Endpoints: " << endl);
    for (auto& name: t_dripline_names) {
        hINFO(cerr << "    " << name << endl);
    }

    // 2: construct sensor entries from Dripline endpoints
    sensor_config_by_names t_config("dripline_endpoint"); //sensor storage in memory after the config load
    if (! f_input_delimiters.empty()) {
        t_config.set_delimiters(f_input_delimiters, f_output_delimiter);
    }
    t_config.load(a_sensor_table, t_dripline_names, f_basename);

    // 3: make a Dripline endpoint table
    hINFO(cerr << "Dripline Endpoint Binding: " << endl);
    set<string> t_endpoint_list(t_dripline_names.begin(), t_dripline_names.end());
    for (int t_number: a_sensor_table.find_like({{}})) { // --> getting all sensors
        const sensor& t_sensor = a_sensor_table[t_number];
        string t_endpoint = t_sensor.get_option("dripline_endpoint", ""); 
        string t_field = t_sensor.get_option("dripline_endpoint_field", "raw"); //Nobel: so here populate the added f_field_table 

        if (t_endpoint_list.count(t_endpoint) > 0) {
            f_endpoint_n_field_table[t_number] = {t_endpoint, t_field}; //---> HERE, STORES THE ENDPOINT MAPPING, would be like 131 --> {name, field pref(calibrated or raw)}
            hINFO(cerr << "    " << t_endpoint << " => " << t_sensor.get_name().join(f_output_delimiter) << endl);
        }
    }
}
//before: 6 parameter into , added for calibration 
//added another paramter for the string, need to do the same for the .hh framework design 
void dripline_pgsql::fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
    try {
        auto t_series_list = this->fetch({{a_sensor}}, a_from, a_to, a_resampling_interval, a_reducer, value_column); //added for calibration
        if (t_series_list.size() == 1) {
            a_series = std::move(t_series_list[0]);
        }
    }
    catch (std::runtime_error &e) {
        throw e;
    }
}
//before: 5 parameter into , added for calibration
//returning a array of series output from the db
//added another paramter , need to do the same for the .hh fm design
vector<series> dripline_pgsql::fetch(const vector<int>& a_sensor_list, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
    //seperating endpoints name based on their data type pref using the f_endpoint_n_field_table

    vector<series> t_series_list;
    
    map<string, vector<unsigned>> t_series_index_table;
    string t_raw_targets, t_cal_targets;
    string t_targets;      //Nobel: changed so that while the f_endpoint_n... hold int -> {tag, field}, stll be able to endpoint list and index table
    
    for (auto t_sensor: a_sensor_list) {
        auto iter = f_endpoint_n_field_table.find(t_sensor);
        if (iter != f_endpoint_n_field_table.end()) {
            const string& endpoint = iter->second.first; // endpoint_name
            
            if (t_series_index_table.count(endpoint) == 0) {
                if (iter->second.second == "raw") {
                    t_raw_targets += (t_raw_targets.empty() ? "'" : ",'") + endpoint + "'";
                } else if (iter->second.second == "calibrated") {
                    t_cal_targets += (t_cal_targets.empty() ? "'" : ",'") + endpoint + "'";
                }
            }
            t_series_index_table[endpoint].push_back(t_series_list.size());
        }
        t_series_list.emplace_back(a_from, a_to);
    }

    t_targets = t_raw_targets + (t_cal_targets.empty() ? "" : "," + t_cal_targets); //Nobel: combining for initial query

    if (t_targets.empty()) {
        return t_series_list;
    }
    
    string t_sql_raw, t_sql_cal; //Nobel: for seperating querying of raw and cal 
    string t_sql; {
        string date_from = datetime(a_from).as_string() + "Z";
        string date_to = datetime(a_to).as_string() + "Z";
        string tag = f_sensorname_column;
        string tag_values = t_targets;
        string field = value_column; //Nobel: Top level --calibration prioritized
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
#else            //Nobel: new adjument to querying 
        if(field == "value_cal") { //if --calibrated then all is calibrated, otherwise
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

            hINFO(cerr << "SQL: " << endl);
            hINFO(cerr << "    " << t_sql << endl);

            double time;
            map<string, vector<unsigned>>::iterator t_channel_iter;
            auto t_handler = [&](int a_row, int a_col, const char* a_value) {
                if (a_col == 0) {
                    time = stod(a_value);
                }
                else if (a_col == 1) {
                    t_channel_iter = t_series_index_table.find(a_value);
                }
                else {
                    for (unsigned index: t_channel_iter->second) {
                        t_series_list[index].emplace_back(time, stod(a_value));
                    }
                }
            };
            if (f_pgsql.query(t_sql, t_handler) < 0) {
                throw std::runtime_error("DB Query Error: SQL: " + t_sql);
            }

        } else {
            if (!t_raw_targets.empty()) {
                t_sql_raw = (string("")
                + "SELECT"
                + "  extract(epoch from timestamp), " + tag + ", " + "value_raw" + " "
                + "FROM"
                + "  numeric_data "
                + "WHERE "
                + "  " + tag + " IN (" + t_raw_targets + ") "
                + "  AND timestamp>='" + date_from + "' AND timestamp<'" + date_to + "'"
                + "ORDER BY"
                + "  timestamp asc"
                );
                
                hINFO(cerr << "SQL: " << endl);
                hINFO(cerr << "    " << t_sql_raw << endl);

                double time;
                map<string, vector<unsigned>>::iterator t_channel_iter;
                auto t_handler_raw = [&](int a_row, int a_col, const char* a_value) {
                    if (a_col == 0) {
                        time = stod(a_value);
                    }
                    else if (a_col == 1) {
                        t_channel_iter = t_series_index_table.find(a_value);
                    }
                    else {
                        for (unsigned index : t_channel_iter->second) {
                            // Only fill for sensors that want raw
                            auto iter = f_endpoint_n_field_table.find(a_sensor_list[index]);
                            if (iter != f_endpoint_n_field_table.end() && iter->second.second == "raw") {
                                t_series_list[index].emplace_back(time, stod(a_value));
                            }
                        }
                    }
                };
                if (f_pgsql.query(t_sql_raw, t_handler_raw) < 0) {
                    throw std::runtime_error("DB Query Error: SQL: " + t_sql_raw);
                }
            }
            if(!t_cal_targets.empty()) {
                t_sql_cal = (string("")
                + "SELECT"
                + "  extract(epoch from timestamp), " + tag + ", " + "value_cal" + " "
                + "FROM"
                + "  numeric_data "
                + "WHERE "
                + "  " + tag + " IN (" + t_cal_targets + ") "
                + "  AND timestamp>='" + date_from + "' AND timestamp<'" + date_to + "'"
                + "ORDER BY"
                + "  timestamp asc"
                );
                
                double time;
                map<string, vector<unsigned>>::iterator t_channel_iter;
                auto t_handler_cal = [&](int a_row, int a_col, const char* a_value) {
                    if (a_col == 0) {
                        time = stod(a_value);
                    }
                    else if (a_col == 1) {
                        t_channel_iter = t_series_index_table.find(a_value);
                    }
                    else {
                        for (unsigned index : t_channel_iter->second) {
                            // Only fill for sensors that want cal
                            auto iter = f_endpoint_n_field_table.find(a_sensor_list[index]);
                            if (iter != f_endpoint_n_field_table.end() && iter->second.second == "calibrated") {//Nobel: if issues, make sure its calibrated not cal
                                t_series_list[index].emplace_back(time, stod(a_value));
                            }
                        }
                    }
                };
                if (f_pgsql.query(t_sql_cal, t_handler_cal) < 0) {
                    throw std::runtime_error("DB Query Error: SQL: " + t_sql_cal);
                }
            }
           
        }
        
#endif
    }

    return t_series_list;
}



vector<string> csv_file::get_data_names()
{
    return vector<string>();
}

void csv_file::fetch_single(series& a_series, int a_sensor, double a_from, double a_to, double a_resampling_interval, const std::string& a_reducer, const std::string& value_column)
{
}
