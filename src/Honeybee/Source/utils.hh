/*
 * utils.hh
 *
 *  Created on: Oct 22, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_UTILS_HH_
#define HONEYBEE_UTILS_HH_ 1

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cstring>
#include <sstream>

#define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/')+1 : __FILE__)

namespace honeybee {

  enum log_level_t {
    e_log_level_panic = 1,
    e_log_level_error = 2,
    e_log_level_warn = 3,
    e_log_level_info = 4,
    e_log_level_debug = 5,
    e_number_of_log_levels
  };

  extern log_level_t g_log_level;

}

namespace honeybee {
  int error_logger_get_next_static_id();

  // in this case, make it more general, so have put a level inside of(while making it one) and then based on the elvel do specific things 
  void error_logger_log_c(log_level_t a_level, const std::string& a_category, const std::string& a_error_type, const std::string& a_site_id, const std::string& a_message);
  
}

// #define hDEBUG(x) ((g_log_level >= e_log_level_debug) && (std::cerr << "##DEBUG: " << __FILENAME__ << ":" << __LINE__ << ": ") && (x))
// #define hINFO(x) ((g_log_level >= e_log_level_info) && (std::cerr << "##INFO: ") && (x))

// 

// for each level, have if statements that check, so you should only run them when certain level, so for info, if level is 2, then run it
// greater than or less for enum

// look into ndebug 

// Route info/debug through logger metadata flag for unified control. 
// need to show that you are using it at compile time 
#ifndef NDEBUG
#define hDEBUG(x) do { \
    std::ostringstream _hb_oss; _hb_oss << x; \
    honeybee::error_logger_log_c(honeybee::e_log_level_debug, __FILENAME__, "", "", _hb_oss.str()); \
} while(0)
#else
#define hDEBUG(x) {}
#endif

#define hINFO(x) do { \
    std::ostringstream _hb_oss; _hb_oss << x; \
    honeybee::error_logger_log_c(honeybee::e_log_level_info, __FILENAME__, "", "", _hb_oss.str()); \
} while(0)

// Route warnings/errors/.. through the central logger using a stable call-site id.
#define hWARN(x) do { \
  std::ostringstream _hb_oss; _hb_oss << x; \
  static int _hb_warn_id = honeybee::error_logger_get_next_static_id(); \
  honeybee::error_logger_log_c(honeybee::e_log_level_warn, __FILENAME__, "warn", \
                              std::string("warn_") + std::to_string(_hb_warn_id), \
                              _hb_oss.str()); \
} while(0)

#define hERROR(x) do { \
  std::ostringstream _hb_oss; _hb_oss << x; \
  static int _hb_error_id = honeybee::error_logger_get_next_static_id(); \
  honeybee::error_logger_log_c(honeybee::e_log_level_error, __FILENAME__, "error", \
                              std::string("error_") + std::to_string(_hb_error_id), \
                              _hb_oss.str()); \
} while(0)

#define hPANIC(x) do { \
  std::ostringstream _hb_oss; _hb_oss << x; \
  static int _hb_panic_id = honeybee::error_logger_get_next_static_id(); \
  honeybee::error_logger_log_c(honeybee::e_log_level_panic, __FILENAME__, "panic", \
                              std::string("panic_") + std::to_string(_hb_panic_id), \
                              _hb_oss.str()); \
} while(0)



namespace honeybee {

    
    class datetime {
      public:
        explicit datetime(long a_timestamp): f_timestamp(a_timestamp){}
        explicit datetime(const std::string& a_datetime, const std::string& a_format="%Y-%m-%dT%H:%M:%S");
        operator long() { return f_timestamp; }
        std::string as_string(const std::string& a_format="%Y-%m-%dT%H:%M:%S");
        static datetime now(long offset=0) { return datetime(time(NULL)+offset); }
        friend long operator-(const datetime& a_to, const datetime& a_from) {
            return a_to.f_timestamp - a_from.f_timestamp;
        }
      protected:
        long f_timestamp;
    };


    class arange {
        class index {
          public:
            index(unsigned a_current): f_current(a_current) {}
            index& operator++() {
                ++f_current;
                return *this;
            }
            bool operator!=(const index& a_index) const {
                return f_current != a_index.f_current;
            }
            unsigned operator*() const {
                return f_current;
            }
          protected:
            unsigned f_current;
        };
      public:
        template<typename XVector> arange(const XVector& a_vector): f_size(a_vector.size()) {}
        index begin() { return index(0); }
        index end() { return index(f_size); }
      protected:
        unsigned f_size;
    };


    template <class XKeyList, class XValueList>
    class zipped_table: protected XValueList {
      public:
        using XValueList::size;
        using XValueList::empty;
        using XValueList::begin;
        using XValueList::end;
        using XValueList::rbegin;
        using XValueList::rend;
        using XValueList::front;
        using XValueList::back;
        using TKey = typename XKeyList::value_type;
        using TValue = typename XValueList::value_type;
      public:
        class item {
          public:
            item(zipped_table& a_table, unsigned index): f_table(a_table), f_current(index) {}
            item& operator++() {
                ++f_current;
                return *this;
            }
            bool operator!=(const item& a_item) const {
                return f_current != a_item.f_current;
            }
            std::pair<TKey&, TValue&> operator*() const {
                return std::pair<TKey&, TValue&>(f_table.f_keys[f_current], f_table[f_current]);
            }
          protected:
            zipped_table& f_table;
            unsigned f_current;
        };
        class const_item {
          public:
            const_item(const zipped_table& a_table, unsigned index): f_table(a_table), f_current(index) {}
            item& operator++() {
                ++f_current;
                return *this;
            }
            bool operator!=(const item& a_item) const {
                return f_current != a_item.f_current;
            }
            std::pair<const TKey&, const TValue&> operator*() const {
                return std::pair<const TKey&, const TValue&>(f_table.f_keys[f_current], f_table[f_current]);
            }
          protected:
            const zipped_table& f_table;
            unsigned f_current;
        };
        class item_iter {
          public:
            item_iter(zipped_table& a_table): f_table(a_table) {}
            item begin() { return item(f_table, 0); }
            item end() { return item(f_table, f_table.size()); }
          protected:
            zipped_table& f_table;
        };
        class const_item_iter {
          public:
            const_item_iter(const zipped_table& a_table): f_table(a_table) {}
            const_item begin() { return const_item(f_table, 0); }
            const_item end() { return const_item(f_table, f_table.size()); }
          protected:
            const zipped_table& f_table;
        };
      public:
        zipped_table() {}
        zipped_table(const XKeyList& a_keys, const XValueList& a_values): f_keys(a_keys), XValueList(a_values) {
            for (unsigned t_index: honeybee::arange(f_keys)) {
                f_index_table[f_keys[t_index]] = t_index;
            }
        }
        zipped_table(const XKeyList& a_keys, XValueList&& a_values): f_keys(a_keys), XValueList(std::move(a_values)) {
            for (unsigned t_index: honeybee::arange(f_keys)) {
                f_index_table[f_keys[t_index]] = t_index;
            }
        }
        zipped_table(XKeyList&& a_keys, XValueList&& a_values): f_keys(std::move(a_keys)), XValueList(std::move(a_values)) {
            for (unsigned t_index: honeybee::arange(f_keys)) {
                f_index_table[f_keys[t_index]] = t_index;
            }
        }
        const XKeyList& keys() const {
            return f_keys;
        }
        XValueList& values() {
            return *this;
        }
        const XValueList& values() const {
            return *this;
        }
        item_iter items() {
            return item_iter(*this);
        }
        const_item_iter items() const {
            return const_item_iter(*this);
        }
        int find(const TKey& a_key) const {
            auto iter = f_index_table.find(a_key);
            return iter == f_index_table.end() ? -1 : iter->second;
        }
        bool has(const TKey& a_key) const {
            return f_index_table.count(a_key) > 0;
        }
        TValue& operator[](unsigned a_index) {
            return XValueList::at(a_index);
        }
        const TValue& operator[](unsigned a_index) const {
            return XValueList::at(a_index);
        }
        TValue& operator[](std::string a_key) {
            auto a_index = f_index_table.find(a_key);
            return XValueList::operator[](this->find(a_key));
        }
        const TValue& operator[](std::string a_key) const {
            auto a_index = f_index_table.find(a_key);
            return XValueList::operator[](this->find(a_key));
        }
      protected:
        XKeyList f_keys;
        std::map<TKey, unsigned> f_index_table;
    };

    template <class XKeyList, class XValueList>
    inline auto zip(XKeyList&& a_keys, XValueList&& a_values) {
        using TKeyList = typename std::remove_const<typename std::remove_reference<XKeyList>::type>::type;
        using TValueList = typename std::remove_const<typename std::remove_reference<XValueList>::type>::type;
        return zipped_table<TKeyList, TValueList>(std::forward<XKeyList>(a_keys), std::forward<XValueList>(a_values));
    }    
    
}
#endif
