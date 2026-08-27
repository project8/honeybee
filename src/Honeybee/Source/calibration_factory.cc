/*
 * calibration_factory.cc
 */

#include "calibration_factory.hh"
#include "sensor_table.hh"
#include "calibration.hh"
#include "kebap_calibration.hh"
#include "db_calibration.hh"
#include "error_logger.hh"
#include <kebap/Kebap.h>
#include <stdexcept>
#include <memory>

using namespace std;

namespace honeybee {

calibration_factory::calibration_factory(shared_ptr<kebap::KPStandardParser> a_parser,
                                         const string& a_ktf_path,
                                         const shared_ptr<calibration_accessor>& a_accessor)
    : f_parser(a_parser), f_ktf_path(a_ktf_path), f_accessor(a_accessor)
{
}

shared_ptr<calibration> calibration_factory::create_calibration(
    const calibration_config& a_config,
    sensor& a_sensor,
    sensor_table& a_sensor_table,
    int a_line_number)
{
    // Semantic parsing
    if (a_config.type == "default_calibration") {
        auto it = a_config.params.find("value");
        if (it != a_config.params.end()) {
            if (!f_parser) {
                throw runtime_error("Parser is not available");
            }
            
            hINFO("Creating inline calibration for " << a_sensor.get_name().join(".")
                      << ": \"" << it->second << "\"");
            
            return make_shared<kebap_calibration>(
                a_sensor,
                a_sensor_table,
                f_parser.get(),
                f_ktf_path,
                a_line_number);
        }
        return nullptr;
    }

    if (a_config.type == "db_calibration") {
        auto it = a_config.params.find("entity_key");
        if (it != a_config.params.end()) {
            if (!f_accessor) {
                throw runtime_error("DB calibration requested but no accessor provided");
            }
            
            hINFO("Creating DB calibration for " << it->second);
            
            return make_shared<db_calibration>(
                a_sensor,
                a_sensor_table,
                f_parser.get(),
                f_ktf_path,
                a_line_number,
                f_accessor,
                it->second);
        }
        return nullptr;
    }

    return nullptr;
}

} // namespace honeybee
