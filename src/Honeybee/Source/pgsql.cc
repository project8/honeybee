/*
 * pgsql.cc
 *
 *  Created on: Oct 21, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include "utils.hh"
#include "pgsql.hh"
#include <iostream>
#include <string>
#include <functional>
#include <libpq-fe.h>

using namespace std;
using namespace honeybee;


pgsql::pgsql(string a_uri)
: f_uri(a_uri)
{
    f_connection = 0;
}

void pgsql::set_db(string a_uri)
{
    f_uri = a_uri;
}

int pgsql::query(const string& a_sql, handler a_handler, bool a_header_enabled)
{
    if (! f_connection) {
        if (f_uri.substr(0, 12) != "postgresql://") {
            f_uri = "postgresql://" + f_uri;
        }
        hINFO(cerr << "connecting to DB (" + f_uri + ")..." << endl);
        auto connection = PQconnectdb(f_uri.c_str());
        if (PQstatus(connection) == CONNECTION_BAD) {
            throw std::runtime_error(string("DB Connection: ") + PQerrorMessage(connection));
        }
        f_connection = connection;
        hINFO(cerr << "    DB connected." << endl);
    }
    
    auto* resp = PQexec(f_connection, a_sql.c_str());
    if (PQresultStatus(resp) != PGRES_TUPLES_OK) {
        PQclear(resp);
        throw std::runtime_error(string("SQL: ") + PQerrorMessage(f_connection));
    }
    
    int n = PQntuples(resp);
    int m = PQnfields(resp);
    if (a_header_enabled) {
        for (int col = 0; col < m; col++) {
            a_handler(-1, col, PQfname(resp, col));
        }
    }
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            a_handler(row, col, PQgetvalue(resp, row , col));
        }
    }
    
    PQclear(resp);
    
    return n;
}

vector<string> pgsql::get_table_list()
{
    vector<string> t_tables;
    string t_sql = "select tablename from pg_tables where schemaname='public'";
    this->query(t_sql, [&](int a_row, int a_col, const char* a_value) {
        t_tables.emplace_back(a_value);
    });

    return t_tables;
}

vector<string> pgsql::get_column_list(const string& a_table_name)
{
    vector<string> t_fields;
    string t_sql = "select * from " + a_table_name + " limit 1";
    auto t_handler = [&](int a_row, int a_col, const char* a_value) {
        if (a_row < 0) {
            t_fields.emplace_back(a_value);
        }
    };
    this->query(t_sql, t_handler, true);

    return t_fields;
}
