/*
 * series_frame.hh
 *
 *  Created on: Oct 21, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_SERIES_HH_
#define HONEYBEE_SERIES_HH_ 1

#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <numeric>
#include <functional>
#include <memory>
#include <cmath>
#include "utils.hh"


namespace honeybee {
    namespace hb = honeybee;
    
    //// (time) Series ////
    class series {
        double f_start, f_stop;
        std::vector<double> f_t, f_x;
      public:
        series(double a_start, double a_stop): f_start(a_start), f_stop(a_stop) {}
        const std::vector<double>& t() const { return f_t; }
        const std::vector<double>& x() const { return f_x; }
        std::vector<double>& t() { return f_t; }
        std::vector<double>& x() { return f_x; }
        double get_start() const { return f_start; }
        double get_stop() const { return f_stop; }
        series& emplace_back(double tk, double xk) {
            f_t.emplace_back(tk);
            f_x.emplace_back(xk);
            return *this;
        }
        series& set_span(double a_start, double a_stop) {
            f_start = a_start;
            f_stop = a_stop;
            return *this;
        }
        series& clear() {
            f_t.clear();
            f_x.clear();
            return *this;
        }
        double dt() const {
            unsigned n = f_t.size();
            if (n < 2) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            std::vector<double> delta(n-1);
            for (unsigned i = 0; i < n-1; i++) {
                delta[i] = f_t[i+1] - f_t[i];
            }
            std::nth_element(delta.begin(), delta.begin() + n/2-1, delta.end());
            return delta[n/2-1];
        }
      public:
        // array of {t,x}
        class tx {
            double &f_t, &f_x;
          public:
            tx(double& a_t, double& a_x): f_t(a_t), f_x(a_x) {}
            double& t() { return f_t; }
            double& x() { return f_x; }
        };
        class const_tx {
            const double &f_t, &f_x;
          public:
            const_tx(const double& a_t, const double& a_x): f_t(a_t), f_x(a_x) {}
            const double& t() { return f_t; }
            const double& x() { return f_x; }
        };
        unsigned size() const { return f_x.size(); }
        tx operator[](unsigned a_index) {
            return tx{f_t[a_index], f_x[a_index]};
        }
        const_tx operator[](unsigned a_index) const {
            return const_tx{f_t[a_index], f_x[a_index]};
        }
        class item {
          public:
            item(series& a_series, unsigned a_index): f_series(a_series), f_index(a_index) {}
            item& operator++() {
                ++f_index;
                return *this;
            }
            bool operator!=(const item& a_item) const {
                return f_index != a_item.f_index;
            }
            tx operator*() const {
                return f_series[f_index];
            }
          protected:
            series& f_series;
            unsigned f_index;
        };
        class const_item {
          public:
            const_item(const series& a_series, unsigned a_index): f_series(a_series), f_index(a_index) {}
            const_item& operator++() {
                ++f_index;
                return *this;
            }
            bool operator!=(const const_item& a_item) const {
                return f_index != a_item.f_index;
            }
            const_tx operator*() const {
                return f_series[f_index];
            }
          protected:
            const series& f_series;
            unsigned f_index;
        };
        item begin() { return item(*this, 0); }
        const_item begin() const { return const_item(*this, 0); }
        item end() { return item(*this, this->size()); }
        const_item end() const { return const_item(*this, this->size()); }
      public:
        series apply(std::function<series(const series&)> a_transformer) const {
            return a_transformer(*this);
        }
        series apply(std::function<double(double)> a_mapper) const {
            series t_series(get_start(), get_stop());
            for (unsigned k = 0; k < f_x.size(); k++) {
                t_series.emplace_back(f_t[k], a_mapper(f_x[k]));
            }
            return t_series;
        }
        series& apply_inplace(std::function<series&(series&)> a_transformer) {
            return a_transformer(*this);
        }
        series& apply_inplace(std::function<double(double)> a_mapper) {
            for (auto& xk: f_x) {
                xk = a_mapper(xk);
            }
            return *this;
        }
        series clone() const { // to apply inplace functors to a const: s.clone().apply_inplace(f)
            return series(*this);
        }
        double reduce(std::function<double(const series&)> a_reducer) const {
            return a_reducer ? a_reducer(*this) : std::numeric_limits<double>::quiet_NaN();
        }
        series filter(std::function<bool(double)> a_filter) const {
            series t_series(f_start, f_stop);
            for (unsigned k = 0; k < f_x.size(); k++) {
                if (a_filter(f_x[k])) {
                    t_series.emplace_back(f_t[k], f_x[k]);
                }
            }
            return t_series;
        }
      public:        
        std::string to_json(const std::string& indent="") const;
        std::string to_csv() const;
    };


    //// Series-applicable functors (reduce) ////
    // example usages:
    //   double mean = t_series.apply(reduce_to_mean);
    extern double reduce_to_count(const series& a_series);
    extern double reduce_to_sum(const series& a_series);
    extern double reduce_to_mean(const series& a_series);
    extern double reduce_to_var(const series& a_series);
    extern double reduce_to_median(const series& a_series);
    extern double reduce_to_std(const series& a_series);
    extern double reduce_to_sem(const series& a_series);
    extern double reduce_to_min(const series& a_series);
    extern double reduce_to_max(const series& a_series);
    extern double reduce_to_first(const series& a_series);
    extern double reduce_to_last(const series& a_series);
    extern double reduce_to_middle(const series& a_series);
    
    //// Series-applicable functors (transform) ////
    // example usages:
    //   auto t_series_2 = t_series.apply(dropna);
    //   auto t_series_2 = t_series.apply_inplace(fillna_with_prev).apply_inplace(fillna_with(0));
    //   auto t_series_2 = t_series.apply(slice(10, 20));
    
    extern series dropna(const series& a_series);
    extern series& keepna(series& a_series);
    extern series& fillna_with_prev(series& a_series);
    extern series& fillna_with_next(series& a_series);
    extern series& fillna_with_closest(series& a_series);
    extern series& fillna_by_line(series& a_series);
    class fillna_with {
      public:
        fillna_with(double a_value);
        series operator()(const series& a_series);
        series& operator()(series& a_series);
      protected:
        double f_value;
    };
    class slice {
      public:
        slice(unsigned a_from, unsigned a_to);
        series operator()(const series& a_series);
      protected:
        unsigned f_from, f_to;
    };

    
    //// Resampler (Series-applicable functor) ////
    // example usages:
    //   resampler t_resampler(group_by_time(t_resampling_interval), reduce_to_first, keepna)
    //   resampler t_resampler(group_to_align(t_series0), reduce_to_mean, fillna_with(-1))
    //   auto t_series_2 = t_series.apply(resampler);
    
    using series_bundle = hb::zipped_table<std::vector<std::string>, std::vector<hb::series>>;
    
    class resampler {
      public:
        struct grouper {
            struct group_index {
                double t, dt;
                unsigned begin, end;
            };
            virtual ~grouper() {}
            virtual void begin_group(const series& a_series) = 0;
            virtual group_index next() = 0;
        };
        using reducer = std::function<double(const series&)>;
        using filler = std::function<series&(series&)>;
      public:
        resampler(std::shared_ptr<grouper> a_grouper, reducer a_reducer, filler a_filler=keepna): f_grouper(a_grouper), f_reducer(a_reducer), f_filler(a_filler) {}
        resampler(const resampler& a_resampler): f_grouper(a_resampler.f_grouper), f_reducer(a_resampler.f_reducer), f_filler(a_resampler.f_filler) {}
        series operator()(const series& a_series);
      protected:
        std::shared_ptr<grouper> f_grouper;
        reducer f_reducer;
        filler f_filler;
    };
    
    class time_grouper: public resampler::grouper {
      public:
        time_grouper(double a_step);
        time_grouper& with_offset(double a_offset);
        time_grouper& with_bounds(double a_start, double a_stop);
      public:
        virtual void begin_group(const series& a_series);
        virtual group_index next();
      protected:
        double f_step, f_offset, f_start, f_stop;
      private:
        std::vector<double> f_time_list;
        unsigned f_current_segment, f_current_point;
    };
    
    class adaptive_time_grouper: public time_grouper {
      public:
        adaptive_time_grouper(double a_step=0);
        time_grouper& group_to_align(const std::vector<series>& a_series);
        time_grouper& group_by_min_intervals_of(const std::vector<series>& a_series);
        time_grouper& group_by_max_intervals_of(const std::vector<series>& a_series);
    };
    
    extern std::shared_ptr<time_grouper> group_by_time(double a_step);
    extern std::shared_ptr<time_grouper> group_to_align(const std::vector<series>& a_series_list);
    extern std::shared_ptr<time_grouper> group_to_align(const series_bundle& a_series_bundle);
    extern std::shared_ptr<time_grouper> group_by_min_intervals_of(const std::vector<series>& a_series_list);
    extern std::shared_ptr<time_grouper> group_by_min_intervals_of(const series_bundle& a_series_bundle);
    extern std::shared_ptr<time_grouper> group_by_max_intervals_of(const std::vector<series>& a_series_list);
    extern std::shared_ptr<time_grouper> group_by_max_intervals_of(const series_bundle& a_series_bundle);


    //// DataFrame: set of resampled (aligned) Series ////
    // DataFame can be viewed:
    // - as an array of columns (array of Series) by columns(), or
    // - as an array of rows (array of Records) by rows(), or
    // - as an matrix by at(irow, icol).
        
    class data_frame {
      public:
        class row_field {
          public:
            row_field(data_frame& a_data_frame, unsigned a_irow, unsigned a_icol): f_data_frame(a_data_frame), f_irow(a_irow), f_icol(a_icol) {}
            row_field& operator++() {
                ++f_icol;
                return *this;
            }
            bool operator!=(const row_field& a_row_field) const {
                return f_icol != a_row_field.f_icol;
            }
            double& operator*() {
                return f_data_frame.at(f_irow, f_icol);
            }
          protected:
            data_frame& f_data_frame;
            unsigned f_irow, f_icol;
        };
        class const_row_field {
          public:
            const_row_field(const data_frame& a_data_frame, unsigned a_irow, unsigned a_icol): f_data_frame(a_data_frame), f_irow(a_irow), f_icol(a_icol) {}
            const_row_field& operator++() {
                ++f_icol;
                return *this;
            }
            bool operator!=(const const_row_field& a_row_field) const {
                return f_icol != a_row_field.f_icol;
            }
            double operator*() const {
                return f_data_frame.at(f_irow, f_icol);
            }
          protected:
            const data_frame& f_data_frame;
            unsigned f_irow, f_icol;
        };
        
        class row_record {
          public:
            row_record(data_frame& a_data_frame, unsigned a_row_index): f_data_frame(a_data_frame), f_row_index(a_row_index) {}
            row_field begin() { return row_field(f_data_frame, f_row_index, 0); }
            row_field end() { return row_field(f_data_frame, f_row_index, f_data_frame.f_columns.size()); }
          public:
            size_t size() const { return f_data_frame.number_of_columns(); }
            double& t() { return f_data_frame.t()[f_row_index]; }
            double& operator[](unsigned a_column_index) { return f_data_frame.at(f_row_index, a_column_index); }
            double& operator[](const std::string& a_column_name) { return f_data_frame[a_column_name].x().at(f_row_index); }
            std::string to_json(const std::string& indent) const;
          protected:
            data_frame& f_data_frame;
            unsigned f_row_index;
        };
        class const_row_record {
          public:
            const_row_record(const data_frame& a_data_frame, unsigned a_row_index): f_data_frame(a_data_frame), f_row_index(a_row_index) {}
            const_row_field begin() const { return const_row_field(f_data_frame, f_row_index, 0); }
            const_row_field end() const { return const_row_field(f_data_frame, f_row_index, f_data_frame.f_columns.size()); }
          public:
            size_t size() const { return f_data_frame.number_of_columns(); }
            double t() const { return f_data_frame.t()[f_row_index]; }
            double operator[](unsigned a_column_index) const { return f_data_frame.at(f_row_index, a_column_index); }
            std::string to_json(const std::string& indent) const;
          protected:
            const data_frame& f_data_frame;
            unsigned f_row_index;
        };
        
      public:
        data_frame();
        data_frame(const series_bundle& a_series_bundle, resampler a_resampler);
        data_frame(const std::vector<series>& a_series_list, resampler a_resampler);
        
        unsigned number_of_rows() const {
            return f_columns.empty() ? 0 : f_columns.front().size();
        }
        unsigned number_of_columns() const {
            return f_columns.size();
        }
        const std::vector<std::string>& column_names() const {
            return f_columns.keys();
        }
        bool has_column(const std::string a_name) const {
            return f_columns.has(a_name);
        }
        const std::vector<double>& t() const {
            return f_columns.front().t();
        }
        std::vector<double>& t() {
            return f_columns.front().t();
        }

        // Data Frame as an array of rows (array of Records) //
        std::vector<row_record>& rows() {
            unsigned n = this->number_of_rows();
            if (f_row_records.size() < n) {
                f_row_records.reserve(n);
                for (unsigned i = 0; i < n; i++) {
                    f_row_records.emplace_back(*this, i);
                }
            }
            return f_row_records;
        }
        std::vector<const_row_record>& rows() const {
            unsigned n = this->number_of_rows();
            if (f_const_row_records.size() < n) {
                f_const_row_records.reserve(n);
                for (unsigned i = 0; i < n; i++) {
                    f_const_row_records.emplace_back(*this, i);
                }
            }
            return f_const_row_records;
        }
        row_record& operator[](unsigned a_row_index) {
            return this->rows()[a_row_index];
        }
        const_row_record& operator[](unsigned a_row_index) const {
            return this->rows()[a_row_index];
        }

        // Data Frame as an array of columns (array of Series) //
        series_bundle& columns() {
            return f_columns;
        }
        const series_bundle& columns() const {
            return f_columns;
        }
        series& operator[](const std::string& a_column_name) {
            return f_columns[f_columns.find(a_column_name)];
        }
        const series& operator[](const std::string& a_column_name) const {
            return f_columns[f_columns.find(a_column_name)];
        }

        // Data Frame as a 2-dim Matrix //
        double& at(unsigned a_row, unsigned a_col) {
            return f_columns[a_col].x().at(a_row);
        }
        double at(unsigned a_row, unsigned a_col) const {
            return f_columns[a_col].x().at(a_row);
        }
        
        std::string to_json(const std::string& indent="") const;
        std::string to_csv() const;

      protected:
        series_bundle f_columns;
        std::vector<row_record> f_row_records;
        mutable std::vector<const_row_record> f_const_row_records;
    };
}
#endif
