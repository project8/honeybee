#include <iostream>
#include <string>
#include <vector>
#include <honeybee/honeybee.hh>

namespace hb = honeybee;


int main(void)
{
    std::string t_db_uri = "p8_db_user:dripline@localhost:5432/p8_sc_db";
    std::string t_name_column = "endpoint_name";
    
    hb::pgsql t_pgsql(t_db_uri);

    std::vector<std::string> t_tables = t_pgsql.get_table_list();
    std::cout << "Tables: ";
    for (auto& t: t_tables) {
        std::cout << t << " ";
    }
    std::cout << std::endl;
    
    std::vector<std::string> t_fields = t_pgsql.get_column_list("numeric_data");
    std::cout << "Fields: ";
    for (auto& f: t_fields) {
        std::cout << f << " ";
    }
    std::cout << std::endl;
    
    std::vector<std::string> t_names; {
        std::string t_sql = "select distinct " + t_name_column + " from numeric_data";
        auto t_handler = [&](int a_row, int a_col, const char* a_value) {
            t_names.emplace_back(a_value);
        };
        t_pgsql.query(t_sql, t_handler);
    }
    std::cout << "SensorNames: ";
    for (auto& n: t_names) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
       
    std::vector<double> t_series; {
        std::string t_sql = (std::string("")
            + "select value_raw from numeric_data "
            + "where " + t_name_column + "='mbar_IG_Vac_MS' "
            + "limit 10 "
        );
        auto t_handler = [&](int a_row, int a_col, const char* a_value) {
            t_series.push_back(std::stod(a_value));
        };
        t_pgsql.query(t_sql, t_handler);
    }
    std::cout << "Values: ";
    for (auto& v: t_series) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
