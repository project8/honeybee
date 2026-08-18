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
                                         const string& a_ktf_path)
    : f_parser(a_parser), f_ktf_path(a_ktf_path)
{
}

shared_ptr<calibration> calibration_factory::create_calibration(calibration_context& ctx)
{
    if (!ctx.sensor_ref.get_calibration().empty()) {
        return create_kebap_calibration(ctx);
    }

    if (!ctx.calibration_uri.empty()) {
        return create_db_calibration(ctx);
    }

    return nullptr;
}

shared_ptr<calibration> calibration_factory::create_kebap_calibration(calibration_context& ctx)
{
    if (!f_parser) {
        throw runtime_error(" Parser is not available");
    }

    hINFO("Creating inline calibration for " << ctx.sensor_ref.get_name().join(".")
              << ": \"" << ctx.sensor_ref.get_calibration() << "\"");

    return make_shared<kebap_calibration>(
        ctx.sensor_ref,
        ctx.sensor_table_ref,
        f_parser.get(),
        f_ktf_path,
        ctx.line_number);
}

shared_ptr<calibration> calibration_factory::create_db_calibration(calibration_context& ctx)
{
    if (!ctx.accessor) {
        throw runtime_error("DB calibration requested but no provided accessor available");
    }

    hINFO("Creating DB calibration for " << ctx.entity_key
              << " from " << ctx.calibration_uri);

    return make_shared<db_calibration>(
        ctx.sensor_ref,
        ctx.sensor_table_ref,
        f_parser.get(),
        f_ktf_path,
        ctx.line_number,
        ctx.accessor,
        ctx.entity_key);
}

} // namespace honeybee
