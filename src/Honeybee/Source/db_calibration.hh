/*
 * db_calibration.hh
 */

#ifndef HONEYBEE_DB_CALIBRATION_HH_
#define HONEYBEE_DB_CALIBRATION_HH_ 1


#include "kebap_calibration.hh"
#include "calibration_accessor.hh"
#include <memory>
#include <string>

namespace honeybee {
  using namespace std;
  class db_calibration : public kebap_calibration {
    public:
      db_calibration(shared_ptr<calibration_accessor> accessor, string entity_key);
      string get_lambda(double query_from, double query_to);

  private:
    shared_ptr<calibration_accessor> f_accessor;
    string f_entity_key;
  };
} // namespace honeybee

#endif
