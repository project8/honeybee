
#include <memory>
#include <string>
#include <stdexcept>
#include "db_calibration.hh"

namespace honeybee {

db_calibration::db_calibration(const sensor& t_sensor,
                               const sensor_table& t_sensor_table,
                               kebap::KPParser* t_parser,
                               const std::string& t_ktf_path,
                               int t_line_number,
                               std::shared_ptr<calibration_accessor> t_accessor,
                               std::string t_entity_key)
    : kebap_calibration(t_sensor, t_sensor_table, t_parser, t_ktf_path, t_line_number),
      f_cal_accessor(t_accessor),
      f_entity_key(std::move(t_entity_key))
{
}

std::string db_calibration::get_lambda(double query_from, double query_to)
{
    if (!f_cal_accessor) {
        throw std::runtime_error("db_calibration: calibration accessor is null");
    }

    return f_cal_accessor->get_lambda(f_entity_key, query_from, query_to);
}

} // namespace honeybee