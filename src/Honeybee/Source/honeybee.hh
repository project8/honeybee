/*
 * honeybee.hh
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_HH_
#define HONEYBEE_HH_ 1

#include <string>
#include <memory>
#include <tabree/KVariant.h>
#include "utils.hh"
#include "series.hh"
#include "sensor_table.hh"
#include "calibration.hh"
#include "data_source.hh"


namespace honeybee {

    class honeybee_app {
      public:
        honeybee_app();
        virtual ~honeybee_app() {}
        void add_config_file(const std::string& filepath);
        void add_dripline_db(const std::string& db_uri);
        void add_variable(const std::string& key, const tabree::KVariant& value);
        void set_delimiter(const std::string& delimiters);
        std::shared_ptr<sensor_table> get_sensor_table();
        std::shared_ptr<data_source> get_data_source();
        std::vector<std::string> find_like(const std::string a_name);
        series_bundle read(const vector<std::string>& a_sensor_list, double a_start, double a_stop);
      protected:
        void construct();
        void find_default_config();
      protected:
        std::string f_config_file_path;
        std::string f_dripline_db_uri;
        std::string f_delimiters;
      protected:
        bool f_is_constructed;
        std::shared_ptr<sensor_table> f_sensor_table;
        std::shared_ptr<data_source> f_data_source;
        sensor_config_by_file::variables f_variables;
    };
    
}
#endif
