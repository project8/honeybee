/*
 * system_config.hh
 */

#ifndef HONEYBEE_SYSTEM_CONFIG_HH_
#define HONEYBEE_SYSTEM_CONFIG_HH_ 1

#include <string>
#include <tabree/KTree.h>

using namespace std;
using namespace tabree;

namespace honeybee {
  class system_config {
    public:
      explicit system_config(const tabree::KTree& a_config);

      string calibration_uri() const;
      string data_source_uri() const;
      string data_source_basename() const;

      // pattern is uri focused, may need to be updated later when handling other user-input
      static string sanitize(const string& text, const string& pattern=R"([a-zA-Z0-9_/:.@-]+)");
      
      string get_data_source_table_name() const { return f_DS_table_name; }
      string get_cal_source_table_name() const { return f_CS_table_name; }


    private:
      struct uri_parts {
        string connection_uri;
        string table_name;
      };

      KTree f_config;
      string f_calibration_uri;
      string f_data_source_uri;
      string f_DS_table_name;
      string f_CS_table_name;

      uri_parts parse_uri(const string& connection_string, const string& default_table_name) const;

      
  };
}

#endif

