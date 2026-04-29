/*
 * error_logger.hh
 */

#ifndef HONEYBEE_ERROR_LOGGER_HH_
#define HONEYBEE_ERROR_LOGGER_HH_ 1

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "utils.hh"

namespace honeybee {
    using namespace std;

    class error_logger {
      public:
        enum message_mode_t {
            e_message_all = 0,               // print every message
            e_message_first_only = 1,        // print first only, suppress repeats
            e_message_first_with_count = 2,  // print first, summarize counts at end
            e_message_summary = 3            // print only final summary
        };

        static error_logger& instance();

        // way of configuring, based on the user choice of mode via CLI
        void set_message_mode(message_mode_t a_mode) { f_message_mode = a_mode; }
        // message_mode_t get_message_mode() const { return f_message_mode; }

        // this is for the metadata before showing the actual data 
        void set_metadata_enabled(bool a_enabled) { f_metadata_enabled = a_enabled; }
        bool is_metadata_enabled() const { return f_metadata_enabled; }

        void clear();

        void log(log_level_t a_level, const string& a_category, const string& a_error_id, const string& a_message);
        // wrappers for log at various levels 
        void error(const string& a_category, const string& a_error_id, const string& a_message);
        void warn(const string& a_category, const string& a_error_id, const string& a_message);
        void info(const string& a_category, const string& a_error_id, const string& a_message);
        void debug(const string& a_category, const string& a_error_id, const string& a_message);

        // Stage and metadata helpers (shown at info level if enabled)

        // Stage: for showing progress through stages of the program, e.g. "Loading data", "Applying calibrations", "Fetching from database", etc.
        // Metadata: for showing key-value pairs or lists of items that are relevant to the user, e.g. "Sensors found: 10", or the names"
        // so more run configuration context
        void stage(const string& a_stage, const string& a_message);
        void metadata(const string& a_title, const vector<string>& a_items); 
        void metadata_kv(const string& a_key, const string& a_value); // one key-val pair, instead of a list of blocks

        // Call once near program shutdown for count/summary modes
        void create_summary(ostream& a_os=std::cerr) const;

      protected:
        error_logger();
        // one stored entry of a unique error, keyed by level + category + error_id
        struct message_record { // ex: {l: e_log_level_warn, c: data_source, 
            log_level_t level;  //      id: invalid_default_column, last_msg: ..., count: 4}
            string category;
            string error_id;
            string last_message;  // most recent message text for that error_id
            unsigned count;
        };

        // check against the g_log_level // verbose
        bool should_print(log_level_t a_level) const;
        // selecting which level should be deduplicated based on the mode
        // idealy want to model depulication to warn, error but not necesarily info, debug, but also want to give user the option to choose which level to deduplicate based on their needs
        bool is_cache_target(log_level_t a_level) const;
        
        // making cache key based on level, category and stable error_id
        // identical triple (level, category, error_id) identifies the same error type
        string make_key(log_level_t a_level, const string& a_category, const string& a_error_id) const;
        // unified and consistent output formatting writer for all messages 
        // so avoid repeating formatting structure multiple times in log, create_summary, and metadata
        void emit_line(ostream& a_os, log_level_t a_level, const string& a_category, const string& a_error_id, const string& a_message) const;
        //mapping level to prefix text 
        const char* level_prefix(log_level_t a_level) const;

      protected:
        message_mode_t f_message_mode;
        bool f_metadata_enabled;
        map<string, message_record> f_records; // fast looking, updating count, make_key helps 
        vector<string> f_order; // keys in FCFS order for summary 
    };
}

#endif