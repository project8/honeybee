/*
 * pgsql.hh
 *
 *  Created on: Oct 21, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_PGSQL_HH_
#define HONEYBEE_PGSQL_HH_ 1

#include <string>
#include <vector>
#include <functional>
struct pg_conn;

namespace honeybee {
    using namespace std;
    
    class pgsql {
        using handler = function<void(int, int, const char*)>;
      public:
        pgsql(string a_uri="");
        void set_db(string a_uri);
        int query(const string& a_sql, handler a_handler, bool a_header_enabled = false);
        vector<string> get_table_list();
        vector<string> get_column_list(const string& a_table_name);
      protected:
        string f_uri;
        pg_conn* f_connection;
    };
}

#endif
