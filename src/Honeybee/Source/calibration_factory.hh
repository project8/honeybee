/*
 * calibration_factory.hh
 */

#ifndef HONEYBEE_CALIBRATION_FACTORY_HH_
#define HONEYBEE_CALIBRATION_FACTORY_HH_ 1

#include <memory>
#include <string>
#include <map>

namespace kebap {
    class KPStandardParser;
}

namespace honeybee {
    class calibration;
    class calibration_accessor;
    class sensor;
    class sensor_table;

    struct calibration_config {
        std::string type;                          // calibration specifier
        std::map<std::string, std::string> params; // All block parameters as key-value pairs
    };

    /*
     * Factory for creating calibration objects.
     * Avoid repeated allocation by holding on to reusable context (parser, ktf_path)
     */
    class calibration_factory {
    public:
        calibration_factory(std::shared_ptr<kebap::KPStandardParser> a_parser,
                           const std::string& a_ktf_path,
                           const std::shared_ptr<calibration_accessor>& a_accessor);
        ~calibration_factory() = default;

        std::shared_ptr<calibration> create_calibration(
            const calibration_config& a_config,
            sensor& a_sensor,
            sensor_table& a_sensor_table,
            int a_line_number = 0);

    private:
        std::shared_ptr<kebap::KPStandardParser> f_parser;
        std::string f_ktf_path;
        std::shared_ptr<calibration_accessor> f_accessor;
    };

} // namespace honeybee

#endif
