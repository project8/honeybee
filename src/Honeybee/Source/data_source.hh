/*
 * data_source.hh
 *
 *  Created on: Oct 19, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_DATA_SOURCE_HH_
#define HONEYBEE_DATA_SOURCE_HH_ 1

#include <string>
#include <vector>
#include "utils.hh"
#include "series.hh"
#include "sensor_table.hh"
#include "calibration.hh"
#include "pgsql.hh"


namespace honeybee {
    using namespace std;

    class data_source {
      public:
        data_source() {}
        virtual ~data_source() {}
        virtual vector<string> get_data_names() = 0;
        virtual void bind(sensor_table& a_sensor_table);
        virtual vector<series> read(const vector<int>& a_sensor_list, double a_from, double a_to);
      protected:
        virtual void bind_inputs(sensor_table& sensor_table) = 0;
        virtual vector<series> fetch(const vector<int>& a_sensor_list, double a_from, double a_to);
        virtual void fetch_single(series& a_series, int a_sensor, double a_from, double a_to) = 0;
      protected:
        int find_input(int);
        void apply_calibration(int a_sensor, series& a_series);
      protected:
        map<int, calibration> f_calibration_table;
    };

    
    class empty_data_source: public data_source {
      public:
        vector<string> get_data_names() override { return vector<string>(); }
      protected:
        void bind_inputs(sensor_table& sensor_table) override {}
        void fetch_single(series& a_series, int a_sensor, double a_from, double a_to) override {}
    };

    
    class dripline_pgsql: public data_source {
      public:
        dripline_pgsql(string a_uri, name_chain a_basename=name_chain());
        vector<string> get_data_names() override;
      protected:
        void bind_inputs(sensor_table& a_sensor_table) override;
        vector<series> fetch(const vector<int>& a_sensor, double a_from, double a_to) override;
        void fetch_single(series& a_series, int a_sensor, double a_from, double a_to) override;
      protected:
        string f_db_uri;
        vector<string> f_basename;
      protected:
        pgsql f_pgsql;
        map<int, string> f_endpoint_table;
        vector<string> f_data_names;
      protected:
        bool f_has_idmap;
        string f_sensorname_column;
    };

    
    class csv_file: public data_source {
      public:
        vector<string> get_data_names() override;
      protected:
        void fetch_single(series& a_series, int a_sensor, double a_from, double a_to) override;
      protected:
        map<int, unsigned> f_column_map;
    };

}
#endif
