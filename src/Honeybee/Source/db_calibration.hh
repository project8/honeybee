/*
 * db_calibration.hh
 */

#ifndef HONEYBEE_DB_CALIBRATION_HH_
#define HONEYBEE_DB_CALIBRATION_HH_ 1


#include "kebap_calibration.hh"
#include "calibration_accessor.hh"
#include <memory>
#include <string>

namespace kebap {
  class KPParser;
}

namespace honeybee {
  class sensor;
  class sensor_table;

  class db_calibration : public kebap_calibration {
    public:
      db_calibration(const sensor& t_sensor,
                     const sensor_table& t_sensor_table,
                     kebap::KPParser* t_parser,
                     const std::string& t_ktf_path,
                     int t_line_number,
                     std::shared_ptr<calibration_accessor> t_accessor,
                     std::string t_entity_key);
      std::string get_lambda(double query_from, double query_to);

  private:
    std::shared_ptr<calibration_accessor> f_cal_accessor;
    std::string f_entity_key;
  };
} // namespace honeybee

#endif
