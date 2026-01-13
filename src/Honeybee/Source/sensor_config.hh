/*
 * sensor_config.hh
 */

#ifndef HONEYBEE_SENSOR_CONFIG_HH_
#define HONEYBEE_SENSOR_CONFIG_HH_ 1

#include <string>

namespace honeybee {
    using namespace std;
    
    class sensor_table;
    
    class sensor_config {
      public:
        virtual ~sensor_config() {}
        
        // Load sensors from file, populate sensor_table
        virtual void load(sensor_table& a_table, const string& a_filename) = 0;
        
      protected:
        sensor_config() {}
    };
}

#endif
