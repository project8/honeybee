/*
 * calibration_factory.hh
 */

#ifndef HONEYBEE_CALIBRATION_FACTORY_HH_
#define HONEYBEE_CALIBRATION_FACTORY_HH_ 1

#include <memory>
#include <string>

namespace kebap {
    class KPStandardParser;
}

namespace honeybee {
    class calibration;
    class calibration_accessor;
    class sensor;
    class sensor_table;

    /*
     * abstract context with all information from ktf calibration blocks needed to create a calibration
     * Can be extended with new fields without changing factory signature
     */
    struct calibration_context {
        sensor& sensor_ref;
        sensor_table& sensor_table_ref;
        std::string entity_key;           // sensor name for DB lookups
        std::string calibration_uri;      // calibration_source uri in ktf
        std::shared_ptr<calibration_accessor> accessor;  // pre-created DB accessor (or nullptr)
        int line_number;                  // line in ktf file for error
        
    };

    /*
     * Factory for creating calibration objects.
     * Avoid repeated allocation by holding on to reusable context (parser, ktf_path)
     */
    class calibration_factory {
    public:
        calibration_factory(std::shared_ptr<kebap::KPStandardParser> a_parser,
                           const std::string& a_ktf_path);
        ~calibration_factory() = default;

        /*
         * logic:
         * - If sensor has inline calibration -> kebap_calibration
         * - Else if calibration_uri is set -> db_calibration
         * - Otherwise -> nullptr
         */
        std::shared_ptr<calibration> create_calibration(calibration_context& ctx);

    private:
        std::shared_ptr<kebap::KPStandardParser> f_parser;
        std::string f_ktf_path;
        
        std::shared_ptr<calibration> create_kebap_calibration(calibration_context& ctx);
        std::shared_ptr<calibration> create_db_calibration(calibration_context& ctx);
    };

} // namespace honeybee

#endif
