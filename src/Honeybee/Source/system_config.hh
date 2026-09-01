/*
 * system_config.hh
 */

#ifndef HONEYBEE_SYSTEM_CONFIG_HH_
#define HONEYBEE_SYSTEM_CONFIG_HH_ 1

#include <string>
#include <tabree/KTree.h>

namespace honeybee {
    class system_config {
      public:
        explicit system_config(const tabree::KTree& a_config);

        std::string calibration_uri() const;
        std::string data_source_uri() const;
        std::string data_source_basename() const;

      private:
        tabree::KTree f_config;
    };
}

#endif

