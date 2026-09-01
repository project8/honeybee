/*
 * psql_calibration_accessor.hh
 */

#ifndef HONEYBEE_PSQL_CALIBRATION_ACCESSOR_HH_
#define HONEYBEE_PSQL_CALIBRATION_ACCESSOR_HH_ 1

#include "calibration_accessor.hh"
#include "pgsql.hh"
#include <string>

namespace honeybee {

    using namespace std;
    class psql_calibration_accessor : public calibration_accessor {
        public:
            explicit psql_calibration_accessor(string connection_string);

            string get_lambda( const string& entity_key, double query_from, double query_to) 
                                override;
        private:
            string build_sql(const string& entity_key, double query_from, double query_to) const;
            string parse_table_name(const string& connection_string) const;
            pgsql f_pgsql;
            string f_table_name;
    };         
}

#endif