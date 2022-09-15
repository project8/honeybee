/*
 * series.cc
 *
 *  Created on: Oct 21, 2020
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "utils.hh"
#include "series.hh"

namespace honeybee {
using namespace std;


static const double NaN = numeric_limits<double>::quiet_NaN();


double reduce_to_count(const series& a_series)
{
    auto acc = [](int y0, double x) { return std::isnan(x) ? y0 : y0+1; };
    return std::accumulate(a_series.x().begin(), a_series.x().end(), 0, acc);
}

double reduce_to_sum(const series& a_series)
{
    double n = reduce_to_count(a_series);
    if (n <= 0) {
        return NaN;
    }
    auto acc = [](double y0, double x) { return std::isnan(x) ? y0 : y0+x; };
    return std::accumulate(a_series.x().begin(), a_series.x().end(), 0.0, acc);
}

double reduce_to_mean(const series& a_series)
{
    double n = reduce_to_count(a_series);
    if (n <= 0) {
        return NaN;
    }
    return reduce_to_sum(a_series)/n;
}

double reduce_to_var(const series& a_series)
{
    const int ddof = 1;
    double n = reduce_to_count(a_series);
    if (n <= ddof) {
        return NaN;
    }
    double m = reduce_to_mean(a_series);
    auto acc = [&](double y0, double x) { return std::isnan(x) ? y0 : y0+(x-m)*(x-m); };
    return std::accumulate(a_series.x().begin(), a_series.x().end(), 0.0, acc) / (n-ddof);
}

double reduce_to_median(const series& a_series)
{    
    std::vector<double> x;
    std::copy_if(
        a_series.x().begin(), a_series.x().end(), std::back_inserter(x),
        [](double x){return !std::isnan(x);}
    );
    if (x.empty()) {
        return NaN;
    }
    std::nth_element(x.begin(), x.begin() + x.size()/2, x.end());
    return x[x.size()/2];
}

double reduce_to_std(const series& a_series)
{
    return sqrt(reduce_to_var(a_series));
}

double reduce_to_sem(const series& a_series)
{
    return sqrt(reduce_to_var(a_series)/reduce_to_count(a_series));
}

double reduce_to_min(const series& a_series)
{        
    auto iter = std::min_element(a_series.x().begin(), a_series.x().end());
    return (iter != a_series.x().end()) ? *iter : NaN;
}

double reduce_to_max(const series& a_series)
{        
    auto iter = std::max_element(a_series.x().begin(), a_series.x().end());
    return (iter != a_series.x().end()) ? *iter : NaN;
}

double reduce_to_first(const series& a_series)
{        
    for (auto iter = a_series.x().begin(); iter != a_series.x().end(); iter++) {
        if (! std::isnan(*iter)) {
            return *iter;
        }
    }
    return NaN;
}

double reduce_to_last(const series& a_series)
{        
    for (auto iter = a_series.x().rbegin(); iter != a_series.x().rend(); iter++) {
        if (! std::isnan(*iter)) {
            return *iter;
        }
    }
    return NaN;
}

double reduce_to_middle(const series& a_series)
{
    auto& t = a_series.t();
    auto& x = a_series.x();
    
    double middle_time = (a_series.get_start() + a_series.get_stop()) / 2;
    double dt0 = std::numeric_limits<double>::max();
    double x0 = NaN;
    if (t.empty()) {
        return x0;
    }
    
    int index = t.size() / 2;
    for (; index >= 0; index--) {
        if (std::isnan(x[index])) {
            continue;
        }
        double dt = fabs(t[index] - middle_time);
        if (dt > dt0) {
            break;
        }
        dt0 = dt;
        x0 = x[index];
    }
    for (index += 1; index < t.size(); index++) {
        if (std::isnan(x[index])) {
            continue;
        }
        double dt = fabs(t[index] - middle_time);
        if (dt > dt0) {
            break;
        }
        dt0 = dt;
        x0 = x[index];
    }
    return x0;
}

series dropna(const series& a_series)
{
    return a_series.filter([](double xk)->bool{ return ! std::isnan(xk); });
}

series& keepna(series& a_series)
{
    return a_series;
}

fillna_with::fillna_with(double a_value)
: f_value(a_value)
{
}
    
series fillna_with::operator()(const series& a_series)
{
    series t_series(a_series);
    for (auto& xk: t_series.x()) {
        xk = (std::isnan(xk) ? f_value : xk);
    }
    return t_series;
}
    
series& fillna_with::operator()(series& a_series)
{
    for (auto& xk: a_series.x()) {
        xk = (std::isnan(xk) ? f_value : xk);
    }
    return a_series;
}

series& fillna_with_prev(series& a_series)
{
    double t_value = NaN;
    for (auto& xk: a_series.x()) {
        if (std::isnan(xk)) {
            xk = t_value;
        }
        else {
            t_value = xk;
        }
    }
    return a_series;
}

series& fillna_with_next(series& a_series)
{
    double t_value = NaN;
    for (auto iter = a_series.x().rbegin(); iter != a_series.x().rend(); ++iter) {
        if (std::isnan(*iter)) {
            *iter = t_value;
        }
        else {
            t_value = *iter;
        }
    }
    return a_series;
}

struct tx {
    double t, x;
    tx(double a_t, double a_x): t(a_t), x(a_x) {}
    tx(series::tx a_tx): t(a_tx.t()), x(a_tx.x()) {}
};

static double interpolate_with_closest(double t, const tx& tx0, const tx& tx1)
{    
    if (std::isnan(tx0.x) && std::isnan(tx1.x)) {
        return NaN;
    }
    else if (std::isnan(tx0.x)) {
        return tx1.x;
    }
    else if (std::isnan(tx1.x)) {
        return tx0.x;
    }
    if (! (tx1.t > tx0.t)) {
        return NaN;
    }
    
    return (fabs(tx0.t - t) < fabs(tx1.t - t)) ? tx0.x : tx1.x;
}
    
static double interpolate_by_line(double t, const tx& tx0, const tx& tx1)
{    
    if (std::isnan(tx0.x) || std::isnan(tx1.x)) {
        return NaN;
    }
    if (! (tx1.t > tx0.t)) {
        return NaN;
    }
    
    return (t - tx0.t) / (tx1.t - tx0.t) * (tx1.x - tx0.x) + tx0.x;
}
    
static series& fillna_by_interpolation(series& a_series, std::function<double(double, const tx&, const tx&)> a_interpolator)
{
    tx t_prev_tx(0, NaN), t_next_tx(0, NaN);
    unsigned t_prev_index = 0, t_next_index = 0, t_size = a_series.size();

    auto& x = a_series.x();
    for (unsigned k = 0; k < t_size; k++) {
        if (! std::isnan(x[k])) {
            t_prev_index = k;
            t_prev_tx = a_series[k];
            continue;
        }
        if (t_next_index <= k) {
            t_next_index = k + 1;
            while ((t_next_index < t_size) && std::isnan(x[t_next_index])) {
                t_next_index++;
            }
            if (t_next_index < t_size) {
                t_next_tx = a_series[t_next_index];
            }
            else {
                t_next_tx.x = NaN;
            }
        }
        x[k] = a_interpolator(a_series[k].t(), t_prev_tx, t_next_tx);
    }

    return a_series;
}
    
series& fillna_with_closest(series& a_series)
{
    return fillna_by_interpolation(a_series, interpolate_with_closest);
}

series& fillna_by_line(series& a_series)
{
    return fillna_by_interpolation(a_series, interpolate_by_line);
}


slice::slice(unsigned a_from, unsigned a_to)
: f_from(a_from), f_to(a_to)
{
}

series slice::operator()(const series& a_series)
{
    series t_series(a_series.get_start(), a_series.get_stop());
    unsigned t_from = std::min<unsigned>(f_from, a_series.size());
    unsigned t_to = std::min<unsigned>(f_to, a_series.size());
    for (unsigned i = t_from; i < t_to; i++) {
        t_series.emplace_back(a_series.t()[i], a_series.x()[i]);
    }
    return t_series;
}


time_grouper::time_grouper(double a_step)
: f_step(a_step), f_offset(0), f_start(NaN), f_stop(NaN)
{
}

time_grouper& time_grouper::with_offset(double a_offset)
{
    f_offset = a_offset;
    return *this;
}

time_grouper& time_grouper::with_bounds(double a_start, double a_stop)
{
    f_start = a_start;
    f_stop = a_stop;
    return *this;
}

void time_grouper::begin_group(const series& a_series)
{
    f_time_list = a_series.t();
    
    if (std::isnan(f_start) || (f_start <= 0)) {
        f_start = a_series.get_start();
    }
    if (std::isnan(f_stop) || (f_stop <= f_start)) {
        f_stop = a_series.get_stop();
    }
    
    f_current_segment = 0;
    f_current_point = 0;
}


time_grouper::group_index time_grouper::next()
{
    if (
        std::isnan(f_step) || (f_step < 0) ||
        std::isnan(f_start) || std::isnan(f_stop) || (f_start >= f_stop)
    ){
        return group_index{NaN, f_step, 0, 0};
    }
    
    while (true) {
        double t0 = f_start + f_offset + f_current_segment * f_step;
        double t1 = t0 + f_step;
        double tk = t0 + f_step/2;
        if (std::isnan(tk) || (tk >= f_stop)) {
            return group_index{tk, f_step, f_current_point, f_current_point};
        }
        f_current_segment++;

        while (f_current_point < f_time_list.size() && f_time_list[f_current_point] < t0) {
            f_current_point++;
        }
        unsigned begin = f_current_point;
        while (f_current_point < f_time_list.size() && f_time_list[f_current_point] < t1) {
            f_current_point++;
        }
        unsigned end = f_current_point;
        return group_index{tk, f_step, begin, end};
    }
}


adaptive_time_grouper::adaptive_time_grouper(double a_step)
: time_grouper(a_step)
{
}

time_grouper& adaptive_time_grouper::group_to_align(const vector<series>& a_series_list)
{
    if (a_series_list.empty()) {
        return *this;
    }

    vector<double> t_steps, t_starts, t_stops;
    for (const auto& t_series: a_series_list) {
        t_starts.push_back(t_series.get_start());
        t_stops.push_back(t_series.get_stop());
        if (t_series.t().size() > 1) {
            t_steps.push_back(t_series.dt());
        }
    }

    auto median_of = [](vector<double>& x)->double {
        if (x.empty()) {
            return NaN;
        }
        std::sort(x.begin(), x.end());
        return x[x.size()/2];
    };
    
    f_step = median_of(t_steps);
    f_start = median_of(t_starts);
    f_stop = median_of(t_stops);

    if (std::isnan(f_step)) {
        f_step = 10;
    }
    
    return *this;
}

time_grouper& adaptive_time_grouper::group_by_min_intervals_of(const vector<series>& a_series_list)
{
    if (a_series_list.empty()) {
        return *this;
    }

    f_step = a_series_list[0].dt();
    f_start = a_series_list[0].get_start();
    f_stop = a_series_list[0].get_stop();
    for (const auto& t_series: a_series_list) {
        f_step = std::isnan(f_step) ? t_series.dt() : std::min(f_step, t_series.dt());
        f_start = std::min(f_start, t_series.get_start());        
        f_stop = std::max(f_stop, t_series.get_stop());        
    }

    if (std::isnan(f_step)) {
        f_step = 10;
    }
    
    return *this;
}

time_grouper& adaptive_time_grouper::group_by_max_intervals_of(const vector<series>& a_series_list)
{
    if (a_series_list.empty()) {
        return *this;
    }

    f_step = a_series_list[0].dt();
    f_start = a_series_list[0].get_start();
    f_stop = a_series_list[0].get_stop();
    for (const auto& t_series: a_series_list) {
        f_step = std::isnan(f_step) ? t_series.dt() : std::max(f_step, t_series.dt());
        f_start = std::max(f_start, t_series.get_start());        
        f_stop = std::min(f_stop, t_series.get_stop());        
    }

    if (std::isnan(f_step)) {
        f_step = 10;
    }
    
    return *this;
}

std::shared_ptr<time_grouper> group_by_time(double a_step)
{
    return std::make_shared<time_grouper>(a_step);
}

std::shared_ptr<time_grouper> group_to_align(const std::vector<series>& a_series_list)
{
    auto grouper = std::make_shared<adaptive_time_grouper>();
    grouper->group_to_align(a_series_list);
    return grouper;
}

std::shared_ptr<time_grouper> group_to_align(const series_bundle& a_series_bundle)
{
    auto grouper = std::make_shared<adaptive_time_grouper>();
    grouper->group_to_align(a_series_bundle.values());
    return grouper;
}

std::shared_ptr<time_grouper> group_by_min_intervals_of(const std::vector<series>& a_series_list)
{
    auto grouper = std::make_shared<adaptive_time_grouper>();
    grouper->group_by_min_intervals_of(a_series_list);
    return grouper;
}

std::shared_ptr<time_grouper> group_by_min_intervals_of(const series_bundle& a_series_bundle)
{
    auto grouper = std::make_shared<adaptive_time_grouper>();
    grouper->group_by_min_intervals_of(a_series_bundle.values());
    return grouper;
}

std::shared_ptr<time_grouper> group_by_max_intervals_of(const std::vector<series>& a_series_list)
{
    auto grouper = std::make_shared<adaptive_time_grouper>();
    grouper->group_by_max_intervals_of(a_series_list);
    return grouper;
}

std::shared_ptr<time_grouper> group_by_max_intervals_of(const series_bundle& a_series_bundle)
{
    auto grouper = std::make_shared<adaptive_time_grouper>();
    grouper->group_by_max_intervals_of(a_series_bundle.values());
    return grouper;
}



series resampler::operator()(const series& a_series)
{
    f_grouper->begin_group(a_series);
    
    series t_series(a_series.get_start(), a_series.get_stop());
    while (true) {
        auto t_range = f_grouper->next();
        if (std::isnan(t_range.t) || (t_range.t >= a_series.get_stop())) {
            break;
        }
        series t_slice = a_series.apply(slice(t_range.begin, t_range.end));
        t_slice.set_span(t_range.t-t_range.dt/2, t_range.t+t_range.dt/2);
        t_series.emplace_back(t_range.t, f_reducer(std::move(t_slice)));
    }

    t_series.apply_inplace(f_filler);
    
    return t_series;
}



std::string series::to_json(const std::string& indent) const
{
    ostringstream os;
    
    auto writeFloat = [](ostream& os, double x) -> void {
        if (std::isnan(x)) os << "null"; else os << x;
    };
    
    string delim="";
    os << "{" << endl;
    os << std::setprecision(12);
    os << indent << "    \"start\": " << f_start << "," << endl;
    os << indent << "    \"length\": " << (f_stop-f_start) << "," << endl;
    os << indent << "    \"timeseries\": {" << endl;
    os << std::setprecision(6);
    os << indent << "        \"t\": ["; delim = "";
    for (auto& t: this->t()) {
        os << delim; delim = ",";
        os << std::round(10*(t-f_start))/10.0;
    }
    os << "]," << endl; 
    os << indent << "        \"x\": ["; delim="";
    for (auto& x: this->x()) {
        os << delim; delim = ",";
        writeFloat(os, x);
    }
    os << "]" << endl;
    os << indent << "    }" << endl;
    os << indent << "}";

    return os.str();
}

std::string series::to_csv(const std::string& label) const
{
    ostringstream os;
    
    os << "DateTime,TimeStamp," << label << endl;
    for (unsigned irow: arange(this->t())) {
        const double& time = f_t[irow];
        os << datetime(time).as_string() << ",";
        os << std::setprecision(12) << std::round(10*time)/10.0 << ",";
        os << std::setprecision(6) << f_x[irow] << endl;
    }

    return os.str();
}




data_frame::data_frame()
{
}

data_frame::data_frame(const series_bundle& a_series_bundle, resampler a_resampler)
{
    std::vector<series> t_resampled_series;
    for (const auto& t_series: a_series_bundle) {
        t_resampled_series.emplace_back(t_series.apply(a_resampler));
    }
    
    f_columns = zip(a_series_bundle.keys(), std::move(t_resampled_series));
}

data_frame::data_frame(const std::vector<series>& a_series_list, resampler a_resampler)
{
    std::vector<std::string> t_column_names;
    std::vector<series> t_resampled_series;
    for (unsigned i: honeybee::arange(a_series_list)) {
        t_resampled_series.emplace_back(a_series_list[i].apply(a_resampler));
        ostringstream os;
        os << "Column" << setw(3) << setfill('0') << i;
        t_column_names.push_back(os.str());
    }
    
    f_columns = zip(std::move(t_column_names), std::move(t_resampled_series));
}



std::string data_frame::row_record::to_json(const std::string& indent) const
{
    return const_row_record(f_data_frame, f_row_index).to_json(indent);
}

std::string data_frame::const_row_record::to_json(const std::string& indent) const
{
    ostringstream os;

    auto writeFloat = [](ostream& os, double x) -> void {
        if (std::isnan(x)) os << "null"; else os << x;
    };
    
    double time = this->t();
    os << indent << "[ \"" << datetime(time).as_string() << "\", ";
    os << std::setprecision(12) << std::round(10*time)/10.0 << std::setprecision(6);
    for (const auto& fields: *this) {
        os << ", "; writeFloat(os, fields);
    }
    os << " ]";

    return os.str();
}

std::string data_frame::to_json(const std::string& indent) const
{
    ostringstream os;
    
    os << indent << "{";
    
    os << endl << indent << "  \"columns\": { \"DateTime\", \"TimeStamp\"";
    for (const string& t_name: f_columns.keys()) {
        os << ", \"" << t_name << "\"";
    }
    os << " }," << endl;
    
    string delim;
    os << indent << "  \"data\": [";
    for (const auto& row: this->rows()) {
        os << delim << endl << row.to_json(indent + "    ");
        delim = ",";
    }
    os << endl << indent << "  ]" << endl;
    
    os << indent << "}" << endl;

    return os.str();
}

std::string data_frame::to_csv() const
{
    ostringstream os;

    os << "DateTime,TimeStamp";
    for (const string& col: f_columns.keys()) {
        os << "," << col;
    }
    os << endl;
    
    for (const auto& row: this->rows()) {
        double time = row.t();
        os << datetime(time).as_string() << ",";
        os << std::setprecision(12) << std::round(10*time)/10.0 << std::setprecision(6);
        for (const auto& col: row) {
            os << "," << col;
        }
        os << endl;
    }

    return os.str();
}

}
