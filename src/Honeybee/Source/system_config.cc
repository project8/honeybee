/*
 * system_config.cc
 */

#include "system_config.hh"

#include <stdexcept>
#include <regex>
#include "error_logger.hh"

using namespace std;

namespace honeybee {

system_config::system_config(const tabree::KTree& a_config)
    : f_config(a_config)
{
    string t_calibration_uri = f_config["calibration_source"]["uri"].Or("");
    if (!t_calibration_uri.empty()) {
        auto t_parts = parse_uri(sanitize(t_calibration_uri), "calibration_projection");
        f_calibration_uri = t_parts.connection_uri;
        f_CS_table_name = t_parts.table_name;
    }

    string t_data_source_uri = f_config["data_source"]["dripline_psql"]["uri"].Or("");
    if (!t_data_source_uri.empty()) {
        auto t_parts = parse_uri(sanitize(t_data_source_uri), "numeric_data");
        f_data_source_uri = t_parts.connection_uri;
        f_DS_table_name = t_parts.table_name;
    }
}

string system_config::calibration_uri() const
{
    return f_calibration_uri;
}

string system_config::data_source_uri() const
{
    return f_data_source_uri;
}

string system_config::data_source_basename() const
{
    return f_config["data_source"]["dripline_psql"]["basename"].Or("");
}

string system_config::sanitize(const string& text, const string& pattern)
{
    auto& t_logger = error_logger::instance();
    try {
        regex t_regex(pattern);
        if (!regex_match(text, t_regex)) {
            string t_message = string("sanitization fault (pattern: ") + pattern + "): " + text;
            t_logger.error("system_config", "sanitization_fault", t_message);
            throw runtime_error(t_message);
        }
    }
    catch (const regex_error& e) {
        string t_message = string("sanitization fault: ") + e.what() + ": " + text;
        t_logger.error("system_config", "sanitization_fault", t_message);
        throw runtime_error(t_message);
    }

    return text;
}

system_config::uri_parts system_config::parse_uri(
    const string& connection_string, const string& default_table_name) const
{
    if (connection_string.empty()) {
        throw runtime_error("invalid database URI: empty connection string");
    }

    string t_uri = connection_string;
    while (!t_uri.empty() && t_uri.back() == '/') {
        t_uri.pop_back();
    }

    size_t t_authority_end = t_uri.find("://");
    size_t t_first_path_slash;
    if (t_authority_end == string::npos) {
        t_first_path_slash = t_uri.find('/');
    }
    else {
        t_first_path_slash = t_uri.find('/', t_authority_end + 3);
    }

    if (t_first_path_slash == string::npos || t_first_path_slash + 1 >= t_uri.size()) {
        throw runtime_error("invalid database URI: expected a database path segment");
    }

    size_t t_second_path_slash = t_uri.find('/', t_first_path_slash + 1);
    if (t_second_path_slash == string::npos) {
        return {t_uri, default_table_name};
    }

    if (t_second_path_slash + 1 >= t_uri.size()
        || t_uri[t_second_path_slash - 1] == '/') {
        throw runtime_error("invalid database URI: empty path segment");
    }

    if (t_uri.find('/', t_second_path_slash + 1) != string::npos) {
        throw runtime_error("invalid database URI: too many path segments");
    }

    return {t_uri.substr(0, t_second_path_slash), t_uri.substr(t_second_path_slash + 1)};
}

}
