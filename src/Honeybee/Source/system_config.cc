/*
 * system_config.cc
 */

#include "system_config.hh"

namespace honeybee {

system_config::system_config(const tabree::KTree& a_config)
    : f_config(a_config)
{
}

std::string system_config::calibration_uri() const
{
    return f_config["calibration_source"]["uri"].Or("");
}

std::string system_config::data_source_uri() const
{
    return f_config["data_source"]["dripline_psql"]["uri"].Or("");
}

std::string system_config::data_source_basename() const
{
    return f_config["data_source"]["dripline_psql"]["basename"].Or("");
}

}
