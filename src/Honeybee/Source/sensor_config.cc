/*
 * sensor_config.cc
 */

#include "sensor_config.hh"
#include "sensor_table.hh"
#include "utils.hh"
#include "error_logger.hh"
namespace honeybee {

void sensor_config_by_names::set_delimiters(const string& a_input_delimiters, const string& a_output_delimiter)
{
    f_input_delimiters = a_input_delimiters;
    f_output_delimiter = a_output_delimiter;
}

void sensor_config_by_names::load(sensor_table& a_table, const vector<string>& a_name_list, name_chain a_basename)
{
    hINFO("Sensor ID matching or creation");
    if (! f_name_space.empty()) {
        hINFO("    Namespace: " << f_name_space);
        hINFO("    Basename: " << a_basename.join());
    }

    map<string, string> t_binding;
    if (! f_name_space.empty()) {
        for (int t_number: a_table.find_like({{}})) {
             const sensor& t_sensor = a_table[t_number];
             string t_endpoint = t_sensor.get_option(f_name_space, "");
             if (! t_endpoint.empty()) {
                 t_binding[t_endpoint] = t_sensor.get_name().join(f_output_delimiter);
             }
        }
    }

    for (const string& t_name: a_name_list) {
        // Demo case: repeated warning
        hWARN("demo warning for binding candidate '" << t_name << "'");

        // explicit matching
        auto t_explicit_iter = t_binding.find(t_name);
        if (t_explicit_iter != t_binding.end()) {
            hINFO("    Explicit: " << t_name << " => " << t_explicit_iter->second);
            continue;
        }
        
        // inference by loose matching
        vector<string> t_chain = name_chain{t_name, f_input_delimiters}.get_chain();
        t_chain.insert(t_chain.end(), a_basename.get_chain().begin(), a_basename.get_chain().end());

        sensor t_sensor;
        auto t_sensor_matches = a_table.find_like(t_chain);
        if (t_sensor_matches.size() == 1) {
            t_sensor = a_table[t_sensor_matches.front()];
            hINFO("    Inferred: " << t_name << " => " << t_sensor.get_name().join(f_output_delimiter));
        }

        // non-unique matching, error, skipped
        else if (t_sensor_matches.size() > 1) {
            hERROR("    Mutiple possibilities on binding: " << t_name << ": ");
            for (auto& s: t_sensor_matches) {
                hERROR("        " << a_table[s].get_name().join(f_output_delimiter));
            }
            hERROR("      hint: use explicit binding to resolve ambiguity");
            continue;
        }

        // create a new sensor entry
        if (! t_sensor) {
            auto t_number = a_table.create_unique_number();
            t_sensor = sensor{t_number, t_chain, t_chain};
            hINFO("    Created: " << t_name << " => " << t_sensor.get_name().join(f_output_delimiter));
        }
        if (! f_name_space.empty()) {
            t_sensor.set_option(f_name_space, t_name);
        }
        a_table.add(t_sensor);
    }
}
}
