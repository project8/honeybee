// demo-series.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <honeybee/honeybee.hh>

namespace hb = honeybee;


int main(int argc, char** argv)
{
    double t0 = hb::datetime::now()-100;
    hb::series t_series(t0, t0+100), t_series2(t0, t0+100);
    for (int i = 0; i < 100; i++) {
        t_series.emplace_back(t0 + i, 10*i);
        t_series2.emplace_back(t0 + i, 5*i);
    }

    //// Several ways to access Series ////
    {
        // as {t[],x[]}
        for (unsigned k = 0; k < t_series.size(); k++) {
            std::cout << "(" << long(t_series.t()[k]) << "," << t_series.x()[k] << "),";
        }
        std::cout << std::endl;
    }
    {
        // as {t[],x[]}, index range-based
        for (unsigned k: hb::arange(t_series)) {
            std::cout << "(" << long(t_series.t()[k]) << "," << t_series.x()[k] << "),";
        }
        std::cout << std::endl;
    }
    {
        // as {t,x}[], index range-based
        for (unsigned k: hb::arange(t_series)) {
            std::cout << "(" << long(t_series[k].t()) << "," << t_series[k].x() << "),";
        }
        std::cout << std::endl;
    }
    {
        // as {t,x}[], range-based
        for (auto t_tx: t_series) {
            std::cout << "(" << long(t_tx.t()) << "," << t_tx.x() << "),";
        }
        std::cout << std::endl;
    }


    //// Series Bundle ////
    std::vector<hb::series> t_series_list{t_series, t_series2};
    std::vector<std::string> t_name_list{"Series0", "Series1"};
    auto t_series_bundle = hb::zip(t_name_list, t_series_list);

    //// Several ways to access Series Bundle ////
    {
        // keys() to get a list of keys, [key] to access series
        for (const std::string& a_name: t_series_bundle.keys()) {
            std::cout << a_name << ": " << t_series_bundle[a_name].to_json() << std::endl;
        }
    }
    {
        // as an array of series
        for (hb::series& t_series: t_series_bundle) {
            std::cout << t_series.to_json() << std::endl;
        }
    }
    {
        // iterating over (key, series) pairs
        for (auto t_item: t_series_bundle.items()) {
            std::cout << t_item.first << ": " << t_item.second.to_json() << std::endl;
        }
    }

    
    //// Functors (Reducers and Resampler) ////
    std::cout << "Mean: " << t_series.reduce(hb::reduce_to_mean) << std::endl;
    
    hb::resampler t_resampler(hb::group_by_time(10), hb::reduce_to_mean, hb::fillna_by_line);
    //hb::resampler t_resampler(hb::group_by_time(10), hb::reduce_to_middle, hb::fillna_with_closest);
    t_series = t_series.apply(t_resampler);
    std::cout << "Resampled: " << t_series.to_json() << std::endl;

    
    //// Data Frame ////
    hb::data_frame t_data_frame(t_series_bundle, t_resampler);
    std::cout << t_data_frame.to_csv();

    
    //// Several ways to access Data Frame ////
    {
        // matrix of doubles
        for (unsigned i = 0; i < t_data_frame.number_of_rows(); i++) {
            std::cout << long(t_data_frame.t()[i]) << "    ";
            for (unsigned j = 0; j < t_data_frame.number_of_columns(); j++) {
                std::cout << t_data_frame[i][j] << "  ";
            }
            std::cout << std::endl;
        }
    }
    {
        // row-oriented (array of records)
        for (auto t_record: t_data_frame.rows()) {
            std::cout << long(t_record.t()) << "   ";
            for (double xk: t_record) {
                std::cout << xk << "  ";
            }
            std::cout << std::endl;
        }
    }
    {
        // column-oriented (bundle of series)
        for (hb::series& t_col: t_data_frame.columns()) {
            for (double xk: t_col.x()) {
                std::cout << xk << "  ";
            }
            std::cout << std::endl;
        }
    }
    {
        // column-oriented, by names (bundle of series)
        for (std::string t_col_name: t_data_frame.column_names()) {
            std::cout << t_col_name << ": ";
            for (auto pk: t_data_frame[t_col_name]) {
                std::cout << pk.x() << "  ";
            }
            std::cout << std::endl;
        }
    }
        
    return 0;
}
