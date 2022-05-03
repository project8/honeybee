/*
 * utils.cc
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */


#include <algorithm>
#include <cctype>
#include <time.h>
#include "utils.hh"


using namespace std;
using namespace honeybee;

log_level_t honeybee::g_log_level = e_log_level_warn;


datetime::datetime(const string& a_datetime, const string& a_format) {
    if (std::all_of(
        a_datetime.cbegin(), a_datetime.cend(),
        [](char ch){return isdigit(ch);}
    )) {
        f_timestamp = std::stoi(a_datetime);
        return;
    }
    
    struct tm tm;
    if (strptime(a_datetime.c_str(), a_format.c_str(), &tm) == NULL) {
        cerr << "ERROR: bad date format: " << a_datetime << endl;
        f_timestamp = 0;
    }
    else {
        f_timestamp = timegm(&tm);
    }
}


string datetime::as_string(const string& a_format) {
    struct tm tm = *(gmtime((time_t *) &f_timestamp));
    char buff[64];
    strftime(buff, sizeof(buff), a_format.c_str(), &tm);
    return string(buff);
}
