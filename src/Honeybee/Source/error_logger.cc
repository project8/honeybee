/*
* error_logger.cc 
*/

#include <sstream>
#include <atomic>
#include "error_logger.hh"

using namespace honeybee;
using namespace std;

error_logger& error_logger::instance() {
    static error_logger t_logger;
    return t_logger;
}

error_logger::error_logger() : f_message_mode(e_message_first_with_count), f_min_level(e_log_level_warn) {}   

// based on the level specified
bool error_logger::should_print(log_level_t a_level) const {
    return a_level <= f_min_level;
}

// returns true for warn/error/panic level
bool error_logger::is_cache_target(log_level_t a_level) const {
    // caching warn or worst 
    return a_level <= e_log_level_warn;
}  

void error_logger::clear() { // clean up 
    f_records.clear();
    f_order.clear();
}

// builds dedup lookup key from level, category, and error_id
string error_logger::make_key(log_level_t a_level, const string& a_category, const string& a_error_id) const {
    return to_string(a_level) + "|" + a_category + "|" + a_error_id;
}

// queues an error-level message for caching
void error_logger::error(const string& a_category, const string& a_error_id, const string& a_message) {
    log(e_log_level_error, a_category, a_error_id, a_message);
}

// queues a panic-level message for caching
void error_logger::panic(const string& a_category, const string& a_error_id, const string& a_message) {
    log(e_log_level_panic, a_category, a_error_id, a_message);
}

// queues a warning-level message for caching
void error_logger::warn(const string& a_category, const string& a_error_id, const string& a_message) {
    log(e_log_level_warn, a_category, a_error_id, a_message);
}   

// logs an info-level message 
void error_logger::info(const string& a_category, const string& a_error_id, const string& a_message) {
    log(e_log_level_info, a_category, a_error_id, a_message);
}

// logs a debug-level message 
void error_logger::debug(const string& a_category, const string& a_error_id, const string& a_message) {
    log(e_log_level_debug, a_category, a_error_id, a_message);
} 


// check the visiblity, and then do dedup lookup 
void error_logger::log(log_level_t a_level, const string& a_category, const string& a_error_id, const string& a_message)
{
    // visibility rule for every level
    if (! should_print(a_level)) {
        return;
    }

    if (! is_cache_target(a_level) || (f_message_mode == e_message_all)) {
        emit_line(cerr, a_level, a_category, a_error_id, a_message);
        return;
    }

    // key = level + category + error_id, and check for repetition
    string t_key = make_key(a_level, a_category, a_error_id);
    auto t_iter = f_records.find(t_key);
    if (t_iter == f_records.end()) {
        message_record t_record;
        t_record.level = a_level;
        t_record.category = a_category;
        t_record.error_id = a_error_id;
        t_record.first_message = a_message;
        t_record.last_message = a_message;
        t_record.count = 1;
        f_records[t_key] = t_record;
        f_order.emplace_back(t_key);

        // if its first occurence for instance
        if (f_message_mode != e_message_summary) {
            emit_line(cerr, a_level, a_category, a_error_id, a_message);
        }
        return;
    }

    t_iter->second.count++;
    t_iter->second.last_message = a_message;  // track latest version of message

    if (f_message_mode == e_message_all) {
        emit_line(cerr, a_level, a_category, a_error_id, a_message);
    }
    // first-only / first-with-count / summary: suppress repeated runtime prints
}

namespace honeybee {
    // generates next unique call-site id 
    int error_logger_get_next_static_id()
    {
        static std::atomic<int> t_next_id{1};
        return t_next_id.fetch_add(1, std::memory_order_relaxed);
    }

    // combines semantic error_type with id
    static string make_typed_id(const string& a_error_type, const string& a_site_id)
    {
        if (a_error_type.empty()) {
            return a_site_id;
        }
        return a_error_type + ":" + a_site_id;
    }

    
    // wraps warn call from all the log macros
    void error_logger_log_c(log_level_t a_level,const string& a_category, const string& a_error_type, const string& a_site_id, const string& a_message)
    {
        // If level uses typed id (warn/error/panic), combine type and location
        std::string t_typed_id; 
        if (!a_error_type.empty() || !a_site_id.empty()) {
            t_typed_id = make_typed_id(a_error_type, a_site_id);
        }

        // Route into the logger singleton that is based on the level 
        if (a_level == e_log_level_panic) {
            error_logger::instance().panic(a_category, t_typed_id, a_message);
        } else if (a_level == e_log_level_error) {
            error_logger::instance().error(a_category, t_typed_id, a_message);
        } else if (a_level == e_log_level_warn) {
            error_logger::instance().warn(a_category, t_typed_id, a_message);
        } else if (a_level == e_log_level_info) {
            error_logger::instance().info(a_category, t_typed_id.empty() ? "info" : t_typed_id, a_message);
        } else if (a_level == e_log_level_debug) {
            error_logger::instance().debug(a_category, t_typed_id.empty() ? "debug" : t_typed_id, a_message);
        } else{
            error_logger::instance().error(a_category, a_site_id, "Invalid log level");        
        }
    }
}

// logs a stage progress message, %%not sure needed anymore%%
void error_logger::stage(const string& a_stage, const string& a_message)
{
    if (! should_print(e_log_level_info)) {
        return;
    }
    info("stage", a_stage, a_message);
}

// prints grouped repeated-message summary
// $$ needs to add the edge case for when there is only one
void error_logger::create_summary(ostream& a_os) const
{
    if (! should_print(e_log_level_warn)) {
        return;
    }

    if ((f_message_mode != e_message_first_with_count) && (f_message_mode != e_message_summary)) {
        return;
    }

    bool t_header_printed = false;
    for (const auto& t_key: f_order) {
        auto t_iter = f_records.find(t_key);
        if (t_iter == f_records.end()) {
            continue;
        }

        const auto& t_record = t_iter->second;
        if (! should_print(t_record.level)) {
            continue;
        }

        if (! t_header_printed) {
            a_os << "##INFO: [error-summary] repeated message counts" << endl;
            t_header_printed = true;
        }

        a_os << level_prefix(t_record.level);
        if (! t_record.category.empty()) {
            a_os << "[" << t_record.category << "] ";
        }
        if (! t_record.error_id.empty()) {
            a_os << "(" << t_record.error_id << ") ";
        }
        a_os << "[FIRST] " << t_record.first_message << " ... [LAST] " << t_record.last_message << " [COUNT=" << t_record.count << "]" << endl;
    }
}

// formats and writes a single log line to stderr
void error_logger::emit_line(ostream& a_os, log_level_t a_level, const string& a_category, const string& a_error_id, const string& a_message) const
{
    a_os << level_prefix(a_level);
    if (! a_category.empty()) {
        a_os << "[" << a_category << "] ";
    }
    if (! a_error_id.empty()) {
        a_os << "(" << a_error_id << ") ";
    }
    a_os << a_message << endl;
}

// returns the ##WARN:/##ERROR: prefix string for a level.
const char* error_logger::level_prefix(log_level_t a_level) const
{
    switch (a_level) {
    case e_log_level_panic: 
        return "##PANIC: ";
    case e_log_level_error: 
        return "##ERROR: ";
    case e_log_level_warn:  
        return "##WARN: ";
    case e_log_level_info:  
        return "##INFO: ";
    case e_log_level_debug: 
        return "##DEBUG: ";
    default:                
        return "##LOG: ";
    }
}







