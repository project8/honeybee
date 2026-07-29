/*
 * calibration_accessor.hh
 */

#ifndef HONEYBEE_CALIBRATION_ACCESSOR_HH_
#define HONEYBEE_CALIBRATION_ACCESSOR_HH_ 1

#include <memory>
#include <string>

namespace honeybee {
  class calibration_accessor {
    public:
      virtual ~calibration_accessor() = default;

      virtual std::string get_lambda( const std::string& entity_key, double query_from,
                                double query_to) const = 0;
  };
} // namespace honeybee

#endif

