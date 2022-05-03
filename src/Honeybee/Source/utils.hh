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


#define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/')+1 : __FILE__)
#define hDEBUG(x) ((g_log_level >= e_log_level_debug) && (std::cerr << "##DEBUG: " << __FILENAME__ << ":" << __LINE__ << ": ") && (x))
#define hINFO(x) ((g_log_level >= e_log_level_info) && (std::cerr << "##INFO: ") && (x))
#define hWARN(x) ((g_log_level >= e_log_level_warn) && (std::cerr << "##WARN: ") && (x))
#define hERROR(x) ((g_log_level >= e_log_level_error) && (std::cerr << "##ERROR: ") && (x))
#define hPANIC(x) ((g_log_level >= e_log_level_panic) && (std::cerr << "##PANIC: ") && (x))



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
