#include "psql_calibration_accessor.hh"

#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "error_logger.hh"

using namespace std;

namespace honeybee {

static string build_lambda(const string& t_lambda_template,
                                 const vector<string>& actuals)
{
    // Parse ex: "x1,x2,x3: function(x1, x2, x3)"
    size_t colon_pos = t_lambda_template.find(':');
    if (colon_pos == string::npos) {
        throw runtime_error("lambda template invalid: missing colon");
    }
    
    string dummy_list = t_lambda_template.substr(0, colon_pos);
    string function_expr = t_lambda_template.substr(colon_pos + 2);  // skip ": "
    
    // Count dummies by counting commas in dummy_list
    size_t num_dummies = 1 + count(dummy_list.begin(), dummy_list.end(), ',');
    
    if (num_dummies != actuals.size()) {
        throw runtime_error("parameter count mismatch: " + to_string(num_dummies) 
                            + " dummies vs " + to_string(actuals.size()) + " actuals");
    }
    
    // Build actual parameter list: "sensor_a, sensor_b, sensor_c"
    string actual_params;
    for (size_t i = 0; i < actuals.size(); ++i) {
        if (i > 0) {
            actual_params += ", ";
        }
        actual_params += actuals[i];
    }
    
    // Substitute x1->sensor_a, x2->sensor_b, .. in function expression
    // Assumes dummies follow x1, x2, x3... pattern(will enforce in the UI, inputing stage)
    for (size_t i = 0; i < actuals.size(); ++i) {
        string dummy = "x" + to_string(i + 1);
        size_t pos = 0;
        
        while ((pos = function_expr.find(dummy, pos)) != string::npos) {
            function_expr.replace(pos, dummy.length(), actuals[i]);
            pos += actuals[i].length();
        }
    }
    
    return actual_params + ": " + function_expr;
}

psql_calibration_accessor::psql_calibration_accessor(string connection_string)
    : f_pgsql(std::move(connection_string)), f_table_name("calibration_projection")
{
    f_table_name = parse_table_name(connection_string);
}

string psql_calibration_accessor::parse_table_name(const string& connection_string) const
{
    if (connection_string.empty()) {
        throw runtime_error("invalid calibration URI: empty connection string");
    }

    string t_uri = connection_string;
    while (!t_uri.empty() && t_uri.back() == '/') {
        t_uri.pop_back();
    }

    auto t_last_slash = t_uri.find_last_of('/');
    if (t_last_slash == string::npos || t_last_slash + 1 >= t_uri.size()) {
        throw runtime_error("invalid calibration URI: expected final path segment to be a table name");
    }

    string t_table = t_uri.substr(t_last_slash + 1);
    if (t_table.empty() || t_table.find('/') != string::npos) {
        throw runtime_error("invalid calibration URI: expected final path segment to be a table name");
    }

    return t_table;
}

string psql_calibration_accessor::build_sql(const string& entity_key, double query_from, double query_to) const
{
    string t_sql = (string("")
        + "SELECT input_source, additional_input_sources, lambda "
        + "FROM " + f_table_name + " "
        + "WHERE entity_key = '" + entity_key + "' "
        + "AND " + to_string(query_from) + " >= valid_from "
        + "AND " + to_string(query_from) + " <= valid_to "
        + "ORDER BY change_made desc, entry_made desc limit 1"
    );
    return t_sql;
}

string psql_calibration_accessor::get_lambda(const string& entity_key, double query_from, double query_to)
{
    auto& t_logger = error_logger::instance();
    
    string t_input_source;
    string t_additional_input_sources;
    string t_lambda_template;
    string t_sql = build_sql(entity_key, query_from, query_to);
    
    hINFO("CALIBRATION SQL: ");
    hINFO("    " << t_sql);
    
    int t_result = f_pgsql.query(t_sql, [&](int a_row, int a_col, const char* a_value) {
        if (a_row == 0) {
            if (a_col == 0 && a_value != nullptr) {
                t_input_source = a_value;
            }
            else if (a_col == 1 && a_value != nullptr) {
                t_additional_input_sources = a_value;
            }
            else if (a_col == 2 && a_value != nullptr) {
                t_lambda_template = a_value;
            }
        }
    });
    
    if (t_result < 0) {
        string t_message = string("psql_calibration_accessor: query failed for entity_key: ") 
                                    + entity_key + " SQL: " + t_sql;
        t_logger.error("calibration_accessor", "db_query_error", t_message);
        throw runtime_error(t_message);
    }
    
    if (t_lambda_template.empty()) {
        string t_message = string("psql_calibration_accessor: no calibration found for entity_key: ")
                                     + entity_key;
        t_logger.error("calibration_accessor", "no_calibration", t_message);
        throw runtime_error(t_message);
    }
    
    // combine primary + additional (comma-separated)
    vector<string> t_actuals;
    t_actuals.push_back(t_input_source);
    
    if (!t_additional_input_sources.empty()) {
        istringstream iss(t_additional_input_sources);
        string token;
        while (getline(iss, token, ',')) {
            size_t start = token.find_first_not_of(" \t");
            if (start != string::npos) {
                size_t end = token.find_last_not_of(" \t");
                t_actuals.push_back(token.substr(start, end - start + 1));
            }
        }
    }
    
    // Reconstruct lambda with actual sensor names
    return build_lambda(t_lambda_template, t_actuals);
}

}  // namespace honeybee