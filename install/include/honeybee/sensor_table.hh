/*
 * sensor_table.hh
 *
 *  Created on: Oct 7, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_SENSOR_TABLE_HH_
#define HONEYBEE_SENSOR_TABLE_HH_ 1

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <tabree/KTree.h>


namespace honeybee {
    using namespace std;
    
    class name_chain {
      public:
        name_chain() {}
        name_chain(const vector<string>& a_chain): f_chain(a_chain) {}
        name_chain(string a_joined, string a_sep) {
            for (char t_sep: a_sep) {
                auto t_end = a_joined.find_first_of(t_sep);
                if (t_end == string::npos) {
                    continue;
                }
                while (true) {
                    t_end = a_joined.find_first_of(t_sep);
                    f_chain.push_back(a_joined.substr(0, t_end));
                    if (t_end == string::npos) {
                        break;
                    }
                    a_joined = a_joined.substr(t_end+1);
                }
                break;
            }
            if (f_chain.empty() && ! a_joined.empty()) {
                f_chain.push_back(a_joined);
            }
        }
        inline const string& operator[](int index) const {
            while (index < 0) index += f_chain.size();
            return f_chain[index % f_chain.size()];
        }
        inline string& operator[](int index) {
            return const_cast<string&>(static_cast<const name_chain*>(this)->operator[](index));
        }
        inline unsigned size() const {
            return f_chain.size();
        }
        inline const vector<string>& get_chain() const { return f_chain; }
        inline vector<string>& get_chain() { return f_chain; }
        string join(const string& a_sep=".", bool a_collapse_empty=true) const {
            string t_joined;
            for (unsigned i = 0; i < f_chain.size(); i++) {
                if (a_collapse_empty && f_chain[i].empty()) {
                    continue;
                }
                t_joined += (i == 0) ? f_chain[i] : (a_sep + f_chain[i]);
            }
            return t_joined;
        }
      protected:
        vector<string> f_chain;
    };


    class sensor {
      public:
        sensor(): f_number(0), f_name(), f_label() {}
        sensor(int a_number, const name_chain& a_name, const name_chain& a_label): f_number(a_number), f_name(a_name), f_label(a_label) {}
        sensor(int a_number, name_chain&& a_name, name_chain&& a_label): f_number(a_number), f_name(a_name), f_label(a_label) {}
        inline operator bool() const { return f_number > 0; }
        inline operator int() const { return f_number; }
        inline int get_number() const { return f_number; }
        inline const name_chain& get_name() const { return f_name; }
        inline const name_chain& get_label() const { return f_label; }
        inline const string& get_calibration() const { return f_calibration; }
        inline string get_option(const string& name, const string& default_value="") const {
            auto iter = f_options.find(name);
            return (iter == f_options.end()) ? default_value : iter->second;
        }
        string to_json(vector<string> a_field_list = {{}}, const std::string& a_delimiter=".") const;
      public:
        // used by sensor_config
        void set_calibration(const string& calibration) { f_calibration = calibration; }
        void set_option(const string& name, const string& value) { f_options[name] = value; }
      protected:
        int f_number;
        name_chain f_name;
        name_chain f_label;
        string f_calibration;
        map<string, string> f_options;
    };

    
    class sensor_table {
      public:
        sensor_table(): f_null_sensor(0, {{"undefined"}}, {{"undefined"}}) {}
        inline const sensor& operator[](int a_number) const {
            auto iter = f_table.find(a_number);
            if (iter == f_table.end()) {
                return f_null_sensor;
            }
            return iter->second;
        }
        inline const sensor& operator[](const name_chain& a_name) const {
            auto iter = f_reverse_table.find(a_name.join(f_internal_separator));
            if (iter == f_reverse_table.end()) {
                return f_null_sensor;
            }
            return this->operator[](iter->second);
        }
        vector<int> find_like(const name_chain& a_chain) const;
        int find_one_like(const name_chain& a_chain) const;
      public:
        void add(const sensor& a_sensor) {
            f_table[a_sensor.get_number()] = a_sensor;
            f_reverse_table[a_sensor.get_name().join(f_internal_separator)] = a_sensor.get_number();
        }
        static int create_unique_number(void) {
            return 0x10000000 + f_unique_sequence++;
        }
      protected:
        sensor f_null_sensor;
        string f_internal_separator = "\t";
        map<int, sensor> f_table;
        unordered_map<string, int> f_reverse_table;
        static int f_unique_sequence;
    };


    class sensor_config_by_file {
      public:
        using variables = vector<pair<string, tabree::KVariant>>;
        void set_variables(const variables& a_variables);
        void load(sensor_table& a_table, const string& a_filename);
      protected:
        struct layer_conf {
            string f_name;
        };
        struct context {
            deque<string> f_name, f_label;
            deque<pair<string, string>> f_opts;
        };
        void load_layer(sensor_table& a_table, const tabree::KTree& a_node, context a_context);
        void add_sensor(sensor_table& a_table, const tabree::KTree& a_node, context a_context);
      protected:
        variables f_variables;
    };



    class sensor_config_by_names {
      public:
        sensor_config_by_names(const string& a_name_space=""): f_name_space(a_name_space), f_input_delimiters("/.-_"), f_output_delimiter(".") {}
        void set_delimiters(const string& a_delimiters, const string& f_output_delimiter);
        void load(sensor_table& a_table, const vector<string>& a_name_list, name_chain a_basename=name_chain());
      protected:
        string f_name_space;  // "dripline_endpoint" etc
        vector<string> f_basenames;
        string f_input_delimiters, f_output_delimiter;
    };
    
}
#endif
