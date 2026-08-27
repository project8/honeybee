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

      private:
        tabree::KTree f_config;
    };
}

#endif


// having this class, solely own all config information realted to the setup , including the ktp path and other things 
/// that way its can also be directed to some output, it is a helpful metedata 
// so what dripline, calirbation db was used , and also which sensor table
// which sensor was used for this run, and the calibrations used(the query that was made)for each of the sensors 
    // can be a pairing 

    // so could have a md5 hash that is a fingerprint of the file as a 64bit 


// So first change the creation stage of the factory to be in honeybee, and adjusting the by_ktf.load_with function 
// transfering the the data source uri context to the system config class, and then adding the logic for the class to hold the 
// the table name for the calirbation db, and fro there moving into the how to sort of cache the curernt sytsem setup/config which we used for a run in thsi case


// after this making sure that the calibration objects are acrtaully being applied, and then moving into the diff lecels of time ranging parsing

// for saving the context, where we are storing the setup, 
    // specifying hwere you want it be saved 