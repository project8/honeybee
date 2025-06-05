// demo-data-source.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <honeybee/honeybee.hh>

namespace hb = honeybee;


int main(int argc, char** argv)
{
    hb::dripline_pgsql t_data_source("p8_db_user:dripline@localhost:5432/p8_sc_db", {{}}, "_", ".");
   
    hb::sensor_table t_sensor_table;
#if 1
    {
        // Actually this is not necessary as data_source::bind() below will do the same.
        // Doing this here does not hurt though.
        hb::sensor_config_by_names t_config;
        t_config.load(t_sensor_table, t_data_source.get_data_names());
    }
#else
    {
        hb::sensor_config_by_file t_config;
        t_config.load(t_sensor_table, "../../../SensorTable/SensorTable_ATDS.ktf");
    }
#endif

    t_data_source.bind(t_sensor_table);
    
    std::vector<int> t_sensors = t_sensor_table.find_like({{"Alicat"}});
    hb::datetime t_from("2022-01-26T08:00:00"), t_to("2022-01-26T08:01:00");
    
    std::vector<hb::series> t_series_list = t_data_source.read(t_sensors, t_from, t_to);

    for (unsigned i: hb::arange(t_sensors)) {
        std::cout << t_sensor_table[t_sensors[i]].get_name().join(".") << ": ";
        std::cout << t_series_list[i].to_json() << std::endl;
    }
    
    return 0;
}
