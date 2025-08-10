// demo-honeybee.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <vector>
#include <iostream>
#include "honeybee/honeybee.hh"

namespace hb = honeybee;


int main(int argc, char** argv)
{
    std::vector<std::string> t_sensors = { "sccm.Alicat.Inj", "K.ThrmCpl.Diss", "mbar.IG.MS" };
    hb::datetime t_from("2022-01-26T08:00:00"), t_to("2022-01-26T08:10:00");

    hb::honeybee_app t_honeybee;
    //t_honeybee.add_dripline_db("p8_db_user:dripline@localhost:5432/p8_sc_db");
    
    auto t_series_bundle = t_honeybee.read(t_sensors, t_from, t_to);

    
    //// Raw Data, element access by index ////
    std::cout << "### Raw Data ####" << std::endl;
    for (const auto& t_name: t_series_bundle.keys()) {
        auto& t_series = t_series_bundle[t_name];
        const std::vector<double>& t = t_series.t();
        const std::vector<double>& x = t_series.x();
        std::cout << t_name << ": ";
        for (unsigned k = 0; k < t_series.size(); k++) {
            std::cout << "(" << long(t[k]) << "," << x[k] << "),";
        }
        std::cout << std::endl;
    }

    //// Reduction / Summarizing ////
    std::cout << "### Reduced to N, Mean and Std ####" << std::endl;
    for (const auto& t_item: t_series_bundle.items()) { // item: pair of name and series
        std::cout << t_item.first << ": ";
        std::cout << t_item.second.reduce(hb::reduce_to_count) << ", ";
        std::cout << t_item.second.reduce(hb::reduce_to_mean) << ", ";
        std::cout << t_item.second.reduce(hb::reduce_to_std) << std::endl;
    }

    //// Resampling (down-sampling with a reducer and up-sampling with a filler) ////
    hb::resampler t_resampler(hb::group_by_time(60), hb::reduce_to_mean, hb::fillna_with_closest);
    std::cout << "### Resampled ####" << std::endl;
    for (const auto& t_series: t_series_bundle) {
        hb::series t_series_resampled = t_series.apply(t_resampler);
        std::cout << t_series_resampled.to_json() << std::endl;
    }
    
    //// Data Frame ////
    std::cout << "### Data Frame ####" << std::endl;
    hb::data_frame t_data_frame(t_series_bundle, t_resampler);
    for (auto row: t_data_frame.rows()) {
        std::cout << long(row.t()) << "   ";
        for (auto col: row) {
            std::cout << col << "  ";
        }
        std::cout << std::endl;
    }

    return 0;
}
