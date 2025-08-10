// demo-sensor-table.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <honeybee/honeybee.hh>

namespace hb = honeybee;


int main(int argc, char** argv)
{
    hb::sensor_table t_table;

#if 1
    hb::sensor_config_by_names t_config;
    hb::dripline_pgsql t_dripline_db("p8_db_user:dripline@localhost:5432/p8_sc_db", {{}}, "_", ".");
    t_config.load(t_table, t_dripline_db.get_data_names(), {{"ATDS"}});
#else
    hb::sensor_config_by_file t_config;
    t_config.set_variables({{"version", 3}, {"date", 20201010}});
    t_config.load(t_table, "../../../SensorTable/SensorTable_ATDS.ktf");
#endif
    
    hb::sensor t_sensor = t_table[{{"mbar","IG","Vac","MS","ATDS"}}];
    std::cout << "number: " << t_sensor.get_number() << std::endl;
    std::cout << "name: " << t_sensor.get_name().join(".") << std::endl;
    std::cout << "label: " << t_sensor.get_label().join(", ") << std::endl << std::endl;
    
    for (auto& t_number: t_table.find_like({{"mbar","IG"}})) {
        std::cout << t_table[t_number].get_name().join(".") << std::endl;
    }

#if 1
    // this will display all the chanels
    for (auto& t_number: t_table.find_like({{}})) {
        std::cout << t_table[t_number].get_name().join(".") << std::endl;
    }
#endif
    
    return 0;
}
