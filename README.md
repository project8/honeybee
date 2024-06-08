Honeybee: Slow-Controls Data Access Library in C++
==================================================

Purposes:
- Pandas-like interface to Dripline SQL data
- Managed sensor names in multiple data sources
- Managed channel mappings that can evolve
- Managed retrospective calibrations
- Zero startup overhead (convention over configuration rule)

Installation
============

## Prerequisite
#### PostgreSQL System-wide
- postgresql-server-dev-all
- libpq-dev


## Compiling
This library consists of two sub-modules (external libraries, written by Sanshiro and publicly available) and one main module:
- Honeybee: main module
- Kebap: formula evaluator
- Tabree: table and tree of variants, JSON I/O, SQL result structure

The usual CMake procedure can be used to install these all (Method 1). Alternatively, for the default installation configuration, a shell script can be used for convenience (Method 2, recommended).

### Method 1 (standard CMake procedure)
1. go to `honeybee/src/ExternalLibraries`
2. do the usual CMake procedure: `cmake`, `make` and `make install`
3. add the `INSTALL_DIR/lib/cmake` to the CMake package path (`CMAKE_PREFIX_PATH` etc)
4. go to `honeybee/src/Honeybee`
5. do the usual CMake procedure

### Method 2 (installation shell script for default configuration)
A shell script is provided to do the Method 1 procedure with the default configuration: 
- the `build` directories are `honeybee/src/build-extern` and `honeybee/src/build-honeybee`
- the `install` directory is `honeybee/install`

Nothing will be placed outside the honeybee tree and no environmental variable is automatically set.
```
$ cd honeybee/src
$ ./setup-honeybee.sh 
Honeybee will be installed at  /home/sanshiro/workspace/Project8/honeybee/install
Are you sure? [y/N] y
-- The CXX compiler identification is Clang 6.0.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
```
- The executable files are located at `honeybee/install/bin`
- The CMake files (necessary to use honeybee) are located at `honeybee/install/lib/cmake`

Usage
=====

## Database Connection
PostgreSQL for ATD at UW:
- IP: currently `10.0.0.31`, will be changed to `10.0.4.x`
- Port: `5432`
- Database: `p8_sc_db`
- User: `p8_db_user`
- Password: **** (should be the same as phase-II)

Setup a ssh tunnel using your account on the p8-potal (`128.208.127.109`):
```
$ ssh -f -N -L 5432:10.0.0.31:5432 128.208.127.109
```

Test the connection:
```
$ psql -U p8_db_user -d p8_sc_db -h localhost
Password for user p8_db_user: ****
Type "help" for help.

p8_sc_db=> select * from numeric_data order by timestamp desc limit 10;
   endpoint_name    |           timestamp           |  value_raw   | value_cal | memo 
--------------------+-------------------------------+--------------+-----------+------
 degC_RTD7_Acc_AS   | 2022-04-19 08:04:56.761709+00 |      -242.02 |           | 
 mbar_CC10_Inj_Gas  | 2022-04-19 08:04:56.735714+00 |         1095 |           | 
 V_CC10_Inj_Gas     | 2022-04-19 08:04:56.729207+00 |       4.8734 |           | 
 degC_RTD6_Acc_AS   | 2022-04-19 08:04:56.631642+00 |      -242.02 |           | 
 degC_RTD5_Acc_AS   | 2022-04-19 08:04:56.487037+00 |      -242.02 |           | 
 degC_RTD4_Acc_AS   | 2022-04-19 08:04:56.348033+00 |      -242.02 |           | 
 degC_RTD3_Acc_AS   | 2022-04-19 08:04:56.216482+00 |      -242.02 |           | 
 V_ThermoCo_Diss_AS | 2022-04-19 08:04:56.086349+00 | 0.0001086677 |           | 
 degC_RTD2_Acc_AS   | 2022-04-19 08:04:56.082338+00 |      -242.02 |           | 
 degC_RTD1_Acc_AS   | 2022-04-19 08:04:55.948071+00 |      -242.02 |           | 
(10 rows)
```

## Quick Tour
### Sensor Table and Management of Sensor Channels
Sensors are organized in a tree structure, and the configuration is typically described in a config file. For the ATD at UW (ATDS) setup, the file is `honeybee/SensorTable/SensorTable_ATDS.ktf`:
```
# data_sources:
#  dripline_psql:
#    uri: p8_db_user:****@localhost:5432/p8_sc_db
#    basename: ATDS

# sensor_table:
#   setup:
#     id: { name: ATDS, label: Atomic Tritium Demonstrator in Seattle }
#     valid_if: date > 20201001

#     section:
#       id: { name: Gas, label: Gas Handling }
#       subsection:
#         id: { name: Inj, label: Injection }
#         module:
#           id: { name: Alicat, label: Alicat Flowmeter }
#           channel:
#             id: { name: sccm, label: Flow }
#           channel:
#             id: { name: degC, label: Temperature }
#           channel:
#             id: { name: psia, label: Pressure }
#         module:
#           id: { name: CC10, label: CC10 Pressure Gauge }
#           channel:
#             id: { name: V, label: LabJunk Voltage }
#           channel:
#             id: { name: mbar, label: Pressure }

#     section:
#       id: { name: AS, label: Atomic Source }
#       subsection:
#         id: { name: Diss, label: Dissociator }
#         module:
#           id: { name: Sys, label: Dissociator System }
..... (continues)
```
(here the format is very similar to YAML, but not exactly the same: multiple child nodes of the same tag name construct an array. This format will be / can be replaced with YAML.)

The sensors defined in the config file can be viewed by the `install/bin/hb-list-sensors` command:
```
$ ./install/bin/hb-list-sensors --config=SensorTable/SensorTable_ATDS.ktf 
[
    { "number": 268435456, "name": "sccm.Alicat.Inj.Gas.ATDS", "options": { "dripline_endpoint": "sccm_Alicat_Inj_Gas" } },
    { "number": 268435457, "name": "degC.Alicat.Inj.Gas.ATDS", "options": { "dripline_endpoint": "degC_Alicat_Inj_Gas" } },
    { "number": 268435458, "name": "psia.Alicat.Inj.Gas.ATDS", "options": { "dripline_endpoint": "psia_Alicat_Inj_Gas" } },
    { "number": 268435459, "name": "V.CC10.Inj.Gas.ATDS", "options": { "dripline_endpoint": "V_CC10_Inj_Gas" } },
    { "number": 268435460, "name": "mbar.CC10.Inj.Gas.ATDS", "options": { "dripline_endpoint": "mbar_CC10_Inj_Gas" } },
    { "number": 268435461, "name": "Type.Sys.Diss.AS.ATDS" },
    { "number": 268435462, "name": "V.PS.Diss.AS.ATDS", "options": { "dripline_endpoint": "V_PS_Diss_AS" } },
    { "number": 268435463, "name": "A.PS.Diss.AS.ATDS", "options": { "dripline_endpoint": "A_PS_Diss_AS" } },
    { "number": 268435464, "name": "V.ThrmCpl.Diss.AS.ATDS", "options": { "dripline_endpoint": "V_ThermoCo_Diss_AS" } },
    { "number": 268435465, "name": "degC.ThrmCpl.Diss.AS.ATDS", "default_calibration": "V:5.1865e-01+V*(-7.0934e+04+V*(-2.4686e+06+V*(-1.3643e+08+V*(-3.3825e+09+V*(-3.5836e+10)))))" },
    { "number": 268435466, "name": "K.ThrmCpl.Diss.AS.ATDS", "default_calibration": "degC:degC+273.15" },
    { "number": 268435467, "name": "degC.RTD0.Acc.AS.ATDS" },
    { "number": 268435468, "name": "K.RTD0.Acc.AS.ATDS", "default_calibration": "degC:degC+273.15" },
    { "number": 268435469, "name": "degC.RTD1.Acc.AS.ATDS", "options": { "dripline_endpoint": "degC_RTD1_Acc_AS" } },
    { "number": 268435470, "name": "K.RTD1.Acc.AS.ATDS", "default_calibration": "degC:degC+273.15" },
    { "number": 268435471, "name": "degC.RTD2.Acc.AS.ATDS", "options": { "dripline_endpoint": "degC_RTD2_Acc_AS" } },
    { "number": 268435472, "name": "K.RTD2.Acc.AS.ATDS", "default_calibration": "degC:degC+273.15" },
..... (continues)
```

The sensor list can be filtered by providing (partial) name matches:
```
### all sensors in the "Diss.AS" section ###
$ ./install/bin/hb-list-sensors  --config=SensorTable/SensorTable_ATDS.ktf   Diss.AS
[
    { "number": 268435461, "name": "Type.Sys.Diss.AS.ATDS" },
    { "number": 268435462, "name": "V.PS.Diss.AS.ATDS", "options": { "dripline_endpoint": "V_PS_Diss_AS" } },
    { "number": 268435463, "name": "A.PS.Diss.AS.ATDS", "options": { "dripline_endpoint": "A_PS_Diss_AS" } },
    { "number": 268435464, "name": "V.ThrmCpl.Diss.AS.ATDS", "options": { "dripline_endpoint": "V_ThermoCo_Diss_AS" } },
    { "number": 268435465, "name": "degC.ThrmCpl.Diss.AS.ATDS", "default_calibration": "V:5.1865e-01+V*(-7.0934e+04+V*(-2.4686e+06+V*(-1.3643e+08+V*(-3.3825e+09+V*(-3.5836e+10)))))" },
    { "number": 268435466, "name": "K.ThrmCpl.Diss.AS.ATDS", "default_calibration": "degC:degC+273.15" }
]
```
```
### all sensors that have "mbar" reading ###
$ ./install/bin/hb-list-sensors  --config=SensorTable/SensorTable_ATDS.ktf   mbar    
[
    { "number": 268435460, "name": "mbar.CC10.Inj.Gas.ATDS", "options": { "dripline_endpoint": "mbar_CC10_Inj_Gas" } },
    { "number": 268435491, "name": "mbar.IG.Vac.AS.ATDS", "options": { "dripline_endpoint": "mbar_IG_Vac_AS" } },
    { "number": 268435503, "name": "mbar.IG.Vac.VSS.ATDS", "options": { "dripline_endpoint": "mbar_IG_Vac_VSS" } },
    { "number": 268435606, "name": "mbar.IG.Vac.MS.ATDS", "options": { "dripline_endpoint": "mbar_IG_Vac_MS" } }
]
```

The sensor table can be constructed from Dripline data, instead of or in addition to configuration files:
```
$ ./install/bin/hb-list-sensors  --dripline-db=p8_db_user:****@localhost:5432/p8_sc_db
[
    { "number": 268435456, "name": "degC.RTD2.Acc.AS", "options": { "dripline_endpoint": "degC_RTD2_Acc_AS" } },
    { "number": 268435457, "name": "mbar.CC10.Inj.Gas", "options": { "dripline_endpoint": "mbar_CC10_Inj_Gas" } },
    { "number": 268435458, "name": "A.PS.Diss.Coax", "options": { "dripline_endpoint": "A_PS_Diss_Coax" } },
    { "number": 268435459, "name": "mbar.IG.Vac.AS", "options": { "dripline_endpoint": "mbar_IG_Vac_AS" } },
    { "number": 268435460, "name": "V.CC10.Inj.Gas", "options": { "dripline_endpoint": "V_CC10_Inj_Gas" } },
    { "number": 268435461, "name": "A.PS.Diss.AS", "options": { "dripline_endpoint": "A_PS_Diss_AS" } },
    { "number": 268435462, "name": "mbar.IG.Vac.MS", "options": { "dripline_endpoint": "mbar_IG_Vac_MS" } },
    { "number": 268435463, "name": "degC.RTD5.Acc.AS", "options": { "dripline_endpoint": "degC_RTD5_Acc_AS" } },
    { "number": 268435464, "name": "degC.RTD1.Acc.AS", "options": { "dripline_endpoint": "degC_RTD1_Acc_AS" } },
..... (continues)
]
```

The location of the configuration file can be specified with an environmental variable, instead of using the program option:
```
$ export HONEYBEE_CONFIG_PATH=/PATH/TO/YOUR/DIR/SensorTable
```
With this setting, all the files with the `.ktf` extension at the indicated directory will be parsed. Hereafter in this documentation, this setting is assumed.

Majority of channels have an automatically-inferred binding to a Dripline end-point:
```
$ ./install/bin/hb-list-sensors  mbar.IG
[
    { "number": 268435491, "name": "mbar.IG.Vac.AS.ATDS", "options": { "dripline_endpoint": "mbar_IG_Vac_AS" } },
    { "number": 268435503, "name": "mbar.IG.Vac.VSS.ATDS", "options": { "dripline_endpoint": "mbar_IG_Vac_VSS" } },
    { "number": 268435606, "name": "mbar.IG.Vac.MS.ATDS", "options": { "dripline_endpoint": "mbar_IG_Vac_MS" } }
]
```

Some channels have an explicit binding described in the config file:
```
$ ./install/bin/hb-list-sensors  V.ThrmCpl.Diss.AS
[
    { "number": 268435464, "name": "V.ThrmCpl.Diss.AS.ATDS", "options": { "dripline_endpoint": "V_ThermoCo_Diss_AS" } }
]
```
(note that namings are not consistent here: `ThrmCpl` vs `ThermoCo`; the config file was used to absorb the error.)

Instead of binding to a Dripline endpoint, some channels have a _calibration_ associated to another input channel:
```
$ ./install/bin/hb-list-sensors  ThrmCpl.Diss.AS
[
    { "number": 268435464, "name": "V.ThrmCpl.Diss.AS.ATDS", "options": { "dripline_endpoint": "V_ThermoCo_Diss_AS" } },
    { "number": 268435465, "name": "degC.ThrmCpl.Diss.AS.ATDS", "default_calibration": "V:5.1865e-01+V*(-7.0934e+04+V*(-2.4686e+06+V*(-1.3643e+08+V*(-3.3825e+09+V*(-3.5836e+10)))))" },
    { "number": 268435466, "name": "K.ThrmCpl.Diss.AS.ATDS", "default_calibration": "degC:degC+273.15" }
]
```
Multiple calibrations can be chained (as in this example, `V` -> `degC` -> `K`).

These bindings and calibrations can be displayed with the `--verbose` option:
```
$ ./install/bin/hb-list-sensors  --verbose
##INFO: honeybee.cc:98: Dripline Datasource: p8_db_user:****@localhost:5432/p8_sc_db
##INFO: pgsql.cc:36: connecting to DB (postgresql://p8_db_user:****@localhost:5432/p8_sc_db)...
##INFO: pgsql.cc:42:     DB connected.
##INFO: data_source.cc:126: Found Dripline ID-Map
##INFO: data_source.cc:145: Dripline Sensor-Name Column: endpoint_name
##INFO: data_source.cc:41: Calibration Chain:
##INFO: data_source.cc:52:     degC.ThrmCpl.Diss.AS.ATDS <= V.ThrmCpl.Diss.AS.ATDS : V:5.1865e-01+V*(-7.0934e+04+V*(-2.4686e+06+V*(-1.3643e+08+V*(-3.3825e+09+V*(-3.5836e+10)))))
##INFO: data_source.cc:52:     K.ThrmCpl.Diss.AS.ATDS <= degC.ThrmCpl.Diss.AS.ATDS : degC:degC+273.15
##INFO: data_source.cc:52:     K.RTD0.Acc.AS.ATDS <= degC.RTD0.Acc.AS.ATDS : degC:degC+273.15
##INFO: data_source.cc:52:     K.RTD1.Acc.AS.ATDS <= degC.RTD1.Acc.AS.ATDS : degC:degC+273.15
(... lines skipped)
##INFO: data_source.cc:152: getting Dripline end-point names...
##INFO: data_source.cc:159:     20 end-points found.
##INFO: data_source.cc:168: Dripline Endpoints: 
##INFO: data_source.cc:170:     degC_RTD2_Acc_AS
##INFO: data_source.cc:170:     mbar_CC10_Inj_Gas
##INFO: data_source.cc:170:     A_PS_Diss_Coax
(... lines skipped)
##INFO: sensor_table.cc:224: Sensor ID matching or creation
##INFO: sensor_table.cc:226:     Namespace: dripline_endpoint
##INFO: sensor_table.cc:227:     Basename: ATDS
##INFO: sensor_table.cc:257:     Inferred: degC_RTD2_Acc_AS => degC.RTD2.Acc.AS.ATDS
##INFO: sensor_table.cc:257:     Inferred: mbar_CC10_Inj_Gas => mbar.CC10.Inj.Gas.ATDS
##INFO: sensor_table.cc:274:     Created: A_PS_Diss_Coax => A.PS.Diss.Coax.ATDS
##INFO: sensor_table.cc:257:     Inferred: mbar_IG_Vac_AS => mbar.IG.Vac.AS.ATDS
##INFO: sensor_table.cc:245:     Explicit: V_ThermoCo_Diss_AS => V.ThrmCpl.Diss.AS.ATDS
(... lines skipped)
##INFO: data_source.cc:178: Dripline Endpoint Binding: 
##INFO: data_source.cc:185:     sccm_Alicat_Inj_Gas => sccm.Alicat.Inj.Gas.ATDS
##INFO: data_source.cc:185:     degC_Alicat_Inj_Gas => degC.Alicat.Inj.Gas.ATDS
##INFO: data_source.cc:185:     psia_Alicat_Inj_Gas => psia.Alicat.Inj.Gas.ATDS
##INFO: data_source.cc:185:     V_CC10_Inj_Gas => V.CC10.Inj.Gas.ATDS
##INFO: data_source.cc:185:     mbar_CC10_Inj_Gas => mbar.CC10.Inj.Gas.ATDS
(... lines skipped)
```


Note:
- Calibrations are applied at run-time, enabling retrospective updates.
- Identity calibration can be used for channel mapping.
- Calibration can be also used for decoding (unpacking of packed bits etc).

All the entries in the config file can be placed under a conditional block (`valid_if`), enabling time-dependent calibration and mapping.
```
#         module:
#           id: { name: ThrmCpl, label: Thermocouple }
#           channel:
#             valid_if: date < 20220201
#             id: { name: degC, label: Temperature in Celsius }
#             default_calibration: V:5.1865e-01-7.0934e+04*V + 23.0
#           channel:
#             valid_if: date >= 20220201
#             id: { name: degC, label: Temperature in Celsius }
#             default_calibration: V:5.1865e-01-7.0934e+04*V
```
This particular example assumes the thermo-couple cold junction compensation was added on 1 Feb 2022.

Theoretically, for more complicated cases, the sensor table contents can be overwritten at run time based on external data, something like below stored in a SQL DB or CSV file:
```
Sensor,ValidityStart,ValidityEnd,Calibration
degC.ThrmCpl.Diss,2021-01-01T00:00:00,9999-01-01T00:00:00,V:0.519*V+28.0
degC.ThrmCpl.Diss,2022-02-01T08:00:00,9999-01-01T00:00:00,V:0.519*V
```

### Data Access
[As of Apr 20 2022, the UW ATD setup has been down since Mar 4. Only `CC10.Inj.Gas` and `ThrmCpl.Diss` produce varying values for this period.]

Using the sensor names, data can be obtained with the `install/bin/hb-get-data` command:
```
$ ./install/bin/hb-get-data  --from=2022-03-04T22:00:00  --length=60   sccm.Inj  K.ThrmCpl  mbar.IG.MS
DateTime,TimeStamp,sccm.Inj,K.ThrmCpl,mbar.IG.MS
2022-03-04T22:00:05,1646431205,4.999,502.444,6e-09
2022-03-04T22:00:15,1646431215,5,512.539,6e-09
2022-03-04T22:00:25,1646431225,5.001,521.774,6e-09
2022-03-04T22:00:35,1646431235,4.998,530.772,6e-09
2022-03-04T22:00:45,1646431245,5.002,539.792,6e-09
2022-03-04T22:00:55,1646431255,4.999,548.503,6e-09
```
If time range is not specified with `--from` and/or `--length`, the last 1 minute data will be shown by default. (The default `length` is 1 minute, and the default `from` is _length_ seconds ago. Also `--to` can be used instead of `--from` or `--length`.)

`--summary` option calculates digested values:
```
$ ./install/bin/hb-get-data   sccm.Inj  K.ThrmCpl  --summary=n,mean,std
{
    "sccm.Inj": { "n": 6, "mean": 4.99983, "std": 0.00147196 },
    "K.ThrmCpl": { "n": 6, "mean": 525.971, "std": 17.1657 },
    "mbar.IG.MS": { "n": 6, "mean": 6e-09, "std": 0 }
}
```

Each sensor is read out at its own timing, therefore data points are not (necessarily) aligned. In order to display the data in the table format (CSV), resampling is internally applied. Parameters to the resampler can be specified with the `--resample=[INTERVAL[,METHOD]]` option:
```
$ ./install/bin/hb-get-data  --from=2022-03-04T22:00:00  --length=60  --resample=20,mean   sccm.Inj  K.ThrmCpl  mbar.IG.MS
DateTime,TimeStamp,sccm.Inj,K.ThrmCpl,mbar.IG.MS
2022-03-04T22:00:10,1646431210,4.9995,507.491,6e-09
2022-03-04T22:00:30,1646431230,4.9995,526.273,6e-09
2022-03-04T22:00:50,1646431250,5.0005,544.147,6e-09
```
Both the resampler parameters are optional. If the interval parameter is not specified (or zero is given), the time buckets are determined automatically based on the majority of the input data intervals. The default reduction method is `middle`, which takes one data point closest to the center of each time bucket. By taking one data point (at most) from each time bucket, the fluctuations in the data is preserved. If this is not necessary, `mean` would be a better reduction method.

If resampling is not necessary, each time-series data can be displayed individually with the `--series` option:
```
$ ./install/bin/hb-get-data  --from=2022-03-04T22:00:00  --length=120   --series   sccm.Inj  K.ThrmCpl  mbar.IG.MS
{
    "sccm.Inj": {
      "start": 1646431200, "length": 120,
      "t": [1.3,11.3,21.3,31.3,41.3,51.3,61.3,71.3,81.3,91.3,101.3,111.3],
      "x": [4.999,5,5.001,4.998,5.002,4.999,5.003,4.997,5.001,5.001,4.999,5]
    },
    "K.ThrmCpl": {
      "start": 1646431200, "length": 120,
      "t": [1.3,11.3,21.3,31.3,41.3,51.3,61.3,71.3,81.3,91.3,101.3,111.3],
      "x": [502.444,512.539,521.774,530.772,539.792,548.503,557.22,565.721,573.955,582.233,590.24,598.056]
    },
    "mbar.IG.MS": {
      "start": 1646431200, "length": 120,
      "t": [1.3,11.3,21.3,31.3,41.3,51.3,61.3,71.3,81.3,91.3,101.3,111.3],
      "x": [6e-09,6e-09,6e-09,6e-09,6e-09,6e-09,6e-09,6e-09,6e-09,6e-09,6e-09,6e-09]
    }
}
```

## C++ API
### Example Program
This example can be found in `install/Examples/Honeybee/demo-honeybee.cxx`.
```
// demo-honeybee.cxx //

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
    //t_honeybee.add_dripline_db("p8_db_user:****@localhost:5432/p8_sc_db");
    
    auto t_series_bundle = t_honeybee.read(t_sensors, t_from, t_to);

    
    //// Raw Data, element access by index ////
    std::cout << "### Raw Data ####" << std::endl;
    for (const auto& t_name: t_series_bundle.keys()) {
        auto& t_series = t_series_bundle[t_name];
        const std::vector<double>& t = t_series.t();
        const std::vector<double>& x = t_series.x();
        std::cout << t_name << ": ";
        for (unsigned k = 0; k < t_series.size(); k++) {
            std::cout << "(" << t[k] << "," << x[k] << "),";
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
    hb::resampler t_resampler(hb::group_by_time(60), hb::reduce_to_mean, hb::fillna_with_prev);
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
```

The `CMaleLists.txt` file in the example directory searches for all the files with the `.cxx` extension.
```
$ cd install/Examples/Honeybee
$ cmake .
$ make
```

Before running, make sure that the ssh-tunneling to Dripline DB has been established.
```
$ ./demo-honeybee
### Raw Data ####
sccm.Alicat.Inj: (1643184001,0.002),(1643184011,0.002),(1643184021,0.002), (...line trimmed)
K.ThrmCpl.Diss: (1643184001,265.949),(1643184011,265.982),(1643184021,265.917), (...line trimmed)
mbar.IG.MS: (1643184009,8.8e-08),(1643184019,8.8e-08),(1643184029,8.8e-08), (...line trimmed)
### Reduced to N, Mean and Std ####
sccm.Alicat.Inj: 60, 0.002, 1.31202e-18
K.ThrmCpl.Diss: 60, 265.881, 0.0570167
mbar.IG.MS: 49, 8.75102e-08, 5.05076e-10
### Resampled ####
{
  "start": 1643184000, "length": 600,
  "t": [30,90,150,210,270,330,390,450,510,570],
  "x": [0.002,0.002,0.002,0.002,0.002,0.002,0.002,0.002,0.002,0.002]
}
{
  "start": 1643184000, "length": 600,
  "t": [30,90,150,210,270,330,390,450,510,570],
  "x": [265.926,265.886,265.859,265.898,265.918,265.909,265.849,265.849,265.878,265.842]
}
{
  "start": 1643184000, "length": 600,
  "t": [30,90,150,210,270,330,390,450,510,570],
  "x": [8.8e-08,8.8e-08,8.8e-08,8.78333e-08,8.76667e-08,8.71667e-08,8.7e-08,8.7e-08,8.7e-08,8.7e-08]
}
### Data Frame ####
1643184030   0.002  265.926  8.8e-08  
1643184090   0.002  265.886  8.8e-08  
1643184150   0.002  265.859  8.8e-08  
1643184210   0.002  265.898  8.78333e-08  
1643184270   0.002  265.918  8.76667e-08  
1643184330   0.002  265.909  8.71667e-08  
1643184390   0.002  265.849  8.7e-08  
1643184450   0.002  265.849  8.7e-08  
1643184510   0.002  265.878  8.7e-08  
1643184570   0.002  265.842  8.7e-08  
```

### Data Objects
Example here can be found in `install/bin/Examples/Honeybee/demo-series.cxx`.

#### Series
The `series` class represents one chain of time-series data.
```
    class series {
        double f_start, f_stop;
        std::vector<double> f_t, f_x;
      public:
        std::vector<double>& t();
        std::vector<double>& x();
        double get_start() const;
        double get_stop() const;
```

Although the implementation is _a struct of arrays_, `{t[],x[]}`, the class provides interface for _an array of structs_, `{t,x}[]`:
```
        // as {t[],x[]}
        for (unsigned k = 0; k < t_series.size(); k++) {
            cout << long(t_series.t()[k]) << "," << t_series.x()[k] << endl;
        }

        // as {t,x}[]
        for (unsigned k = 0; k < t_series.size(); k++) {
            cout << long(t_series[k].t()) << "," << t_series[k].x() << endl;
        }

        // as {t,x}[], range-based
        for (auto t_tx: t_series) {
            cout << long(t_tx.t()) << "," << t_tx.x() << endl;
        }
```

Algorithms for `series` are generally implemented as a functor. For example,
```
    t_series.apply_inplace(fillna_with(0));
    double mean = t_series.reduce(reduce_to_mean);
```
Here are the methods to apply a functor:
```
   public:
      series apply(function<series(const series&)> a_transformer) const;
      series apply(function<double(double)> a_mapper) const;
      series& apply_inplace(function<series&(series&)> a_transformer);
      series& apply_inplace(function<double(double)> a_mapper);
      double reduce(function<double(const series&)> a_reducer) const:
      series filter(function<bool(double)> a_filter) const;
```
Currently the following functors are available:
- to apply: `dropna`, `resample(...)`
- to apply_inplace: `fillna_with(val)`, `fillna_with_prev`, `fillna_with_next`, `fillna_with_closest`, `fillna_by_line`, `keepna`
- to reduce: `reduce_to_XXX` where XXX is `mean`, `median`, `min`, `max`, `count`, `sum`, `std`, `var`, `first`, `last`, `middle`

To apply an mutating functor to a const object, use `clone()`:
```
    series t_series_2 = t_series_1.clone().apply_inplace(fillna_by_line);
```

For the other way, not crating an additional variable even for non-mutating functors, 
```
    (t_series = t_series.apply(dropna)).apply_inplace(...
```

#### Resampling
Resampler is implemented as a functor to `series`. It performs:
1. grouping: break time-series into time buckets
2. reducing: for each bucket, reduce data points to a scalar value (or NaN if no data point exists)
3. filling: for each bucket, replace NaN (called `na` following Pandas) with a value

If a bucket contains multiple data points, the step 2 performs down sampling. If data points are sparse compared to the buckets, the step 3 performs up sampling.

Algorithms for grouping, reducing and filling are provided to the resampler as functors. If smaller fluctuations by averaging is preferable or acceptable, it should typically look like:
```
    resampler t_resampler(group_by_time(10), reduce_to_mean, fillna_by_line);
    series t_resampled_series = t_series.apply(t_resampler);
```
where linear interpolation is used by `fillna_by_line`. If it is only for down sampling with keeping the fluctuations (i.e., to emulate sampling at a lower rate),
```
    resampler t_resampler(group_by_time(10), reduce_to_middle, keepna);
    series t_resampled_series = t_series.apply(t_resampler);
```
where `reduce_to_middle`, as well as `reduce_to_first` and `reduce_to_last`, takes one sample per bucket, and `keepna` does not perform any kind of interpolation; for data from sparse or lazy recording, `fillna_with_closest` or `fillna_with_prev` might be used instead.

#### Series Bundle
The `honeybee::zip(key_vector, value_vector)` combines two input vectors and makes an ordered map of key-value, something similar to Python's _OrderedDict_. It is often convenient to _zip_ a set of series data with their names:
```
    vector<string> t_name_list{ "sccm.Alicat", "K.ThrmCpl.Diss", "mbar.IG.AS" };
    vector<hb::series> t_series_list = read data...;
    auto t_series_bundle = hb::zip(t_name_list, t_series_list);

    for (const string& a_name: t_series_bundle.keys()) {
        cout << a_name << ": " << t_series_bundle[a_name].to_json() << endl;
    }
```
In addition to the key-based access, index-based access is still available, just like `vector<series>`:
```
        for (hb::series& t_series: t_series_bundle) {
            cout << t_series.to_json() << endl;
        }
```
It also can be _iterated_ over the (key, series) pairs:
``` 
        for (auto t_item: t_series_bundle.items()) {
            cout << t_item.first << ": " << t_item.second.to_json() << endl;
        }
```
The zip of `vector<string>` and `vector<series>` is `typedef`-ed as `series_bundle`. 

#### Data Frame
Data Frame is a bundle of aligned (resampled) series. 
```
    hb::data_frame t_data_frame(t_series_bundle, hb:resampler(hb:group_by_time(10), hb::reduce_to_mean, hb::fillna_with_last);
    cout << t_data_frame.to_csv();
```

Data Frame provides three _shapes_ to access data: 1) 2-dim matrix, 2) row-oriented tables, and 3) column-oriented bundle of series.
```
   // matrix of doubles
   for (unsigned i = 0; i < t_data_frame.number_of_rows(); i++) {
      cout << long(t_data_frame.t()[i]) << "    ";
      for (unsigned j = 0; j < t_data_frame.number_of_columns(); j++) {
         cout << t_data_frame[i][j] << "  ";
      }
      cout << endl;
   }
```
```
   // row-oriented (array of records)
   for (auto t_record: t_data_frame.rows()) {
      cout << long(t_record.t()) << "   ";
      for (double xk: t_record) {
         cout << xk << "  ";
      }
      // xk can also be accessed like t_record[t_column_index]
      // xk can also be accessed like t_record[t_column_name]
      cout << endl;
   }
```
```
   // column-oriented (bundle of series)
   for (series& t_col: t_data_frame.columns()) {  // t_col is just a series
       for (double xk: t_col.x()) {
           cout << xk << "  ";
       }
       cout << std::endl;
   }
```
```
   // column-oriented, by names (bundle of series)
   for (string t_col_name: t_data_frame.column_names()) {
      cout << t_col_name << ": ";
      for (auto pk: t_data_frame[t_col_name]) {  // t_data_frame[t_col_name] is just a series
         cout << pk.x() << "  ";
      }
      cout << endl;
   }
```

### Sensors and Sensor Table
#### Sensor Name
Sensors are organized in a tree structure. Sensor names are a chain of mnemonic, such as `sccm.Alicat.Inj.Gas.AS.ATDS` and `K.ThrmCpl.Diss.AS.ATDS`, where each mnemonic corresponds to a node in the sensor tree, and the separator (`.` here) is arbitrary. The corresponding class is:
```
    class name_chain {
        vector<string> f_chain;
      public:
        name_chain(const vector<string>& a_chain);
        name_chain(string a_joined, string a_sep);
        string& operator[](int index);
        string join(const string& a_sep=".") const;
    };
```

For performance reasons, sensors are internally handled with an integer `sensor_number`. Also each sensor name node has a descriptive label. The `sensor` class is composed of these:
```
    class sensor {
        int f_number;
        name_chain f_name;
        name_chain f_label;
        string f_calibration;
      public:
        int get_number() const;
        const name_chain& get_name() const;
        const name_chain& get_label() const;
        const string& get_calibration() const;
        operator int() const { return f_number; }
      };
```
The type-cast operator to `int` enables us to handle a sensor object as an integer value.

While majority of sensors are bound to entries in data sources (such as Dripline SQL DB), some sensor values are derived from values of other sensors. The optional `calibration` field describes this.

#### Sensor Table
Sensor table organizes all the sensors in the system.
```
    class sensor_table {
      public:
        const sensor& operator[](int a_number) const;
        const sensor& operator[](const name_chain& a_name) const;
        vector<int> find_like(const name_chain& a_chain) const;
```
The array operator `[]` converts a sensor number or name to a corresponding sensor object, and `find_like` method searches for sensors that have a (partly) matching name.

The sensor table can be configured by multiple ways. For the simplest system, it can be configured based on entries in a data source such as the Dripline SQL DB:
```
    hb::sensor_table t_table;
    
    hb::sensor_config_by_names t_config;
    hb::dripline_pgsql t_dripline_db("p8_db_user:****@localhost:5432/p8_sc_db");
    t_config.load(t_table, t_dripline_db.get_data_names(), {{"ATDS"}});
```

For a managed system, sensor entries are typically described in a config file:
```
    hb::sensor_table t_table;
    
    hb::sensor_config_by_file t_config;
    t_config.set_variables({{"version", 3}, {"date", 20201010}});
    t_config.load(t_table, "PATH/TO/CONF/SensorTable_ATDS.ktf");
```
Here the configuration file does not have to list all the sensors. If entries constructed from a data source is sufficient, they can be used in that way.

Sensor configuration file can have variables (`version` and `date` in this example) for conditional contents.

#### Example
Example here can be found in `install/bin/Examples/Honeybee/demo-sensor-table.cxx`.
```
// demo-sensor-table.cxx //

#include <iostream>
#include <honeybee/honeybee.hh>
namespace hb = honeybee;

int main(int argc, char** argv)
{
    hb::sensor_table t_table;
    
    hb::sensor_config_by_file t_config;
    t_config.set_variables({{"version", 3}, {"date", 20201010}});
    t_config.load(t_table, "../../../SensorTable/SensorTable_ATDS.ktf");
    
    hb::sensor t_sensor = t_table[{{"mbar","IG","Vac","MS","ATDS"}}];
    std::cout << "number: " << t_sensor.get_number() << std::endl;
    std::cout << "name: " << t_sensor.get_name().join(".") << std::endl;
    std::cout << "label: " << t_sensor.get_label().join(", ") << std::endl << std::endl;
    
    for (auto& t_number: t_table.find_like({{"mbar","IG"}})) {
        std::cout << t_table[t_number].get_name().join(".") << std::endl;
    }

    return 0;
}
```
```
$ ./demo-sensor-table 
number: 268435606
name: mbar.IG.Vac.MS.ATDS
label: Pressure, Ion Gauge, Vacuum, Mass Spectrometer, Atomic Tritium Demonstrator in Seattle

mbar.IG.Vac.AS.ATDS
mbar.IG.Vac.VSS.ATDS
mbar.IG.Vac.MS.ATDS
```

To loop over all the defined channels, use `find_like()` with an empty match pattern:
```    
    // this will display all the channels
    for (auto& t_number: t_table.find_like({{}})) {
        std::cout << t_table[t_number].get_name().join(".") << std::endl;
    }
```

### Data Source
The `data_source` class is an abstraction to various types of data source implementations, such as Dripline DB, other time-series DB, and CSV files. It defines interfaces 1) to query sensor names defined in the data source, and 2) to fetch data from the data source.
```
    class data_source {
      public:
        data_source() {}
        virtual ~data_source() {}
        virtual vector<string> get_data_names() = 0;
        virtual void bind(sensor_table& a_sensor_table);
        virtual vector<series> read(const vector<int>& a_sensor_list, double a_from, double a_to);
```
The `bind()` method constructs bindings between sensor entries defined in the sensor table and entries in the data source.

Currently only Dripline PostgreSQL data source is implemented:
```
    class dripline_pgsql: public data_source {
      public:
        dripline_pgsql(string a_uri, name_chain a_basename=name_chain());
```
where `a_basename` is used to define a namespace for the DB entries (such as `ATDS` for the DB system for the UW Atomic Tritium test-stand).

Combined with the sensor table construction based on DB contents, a minimal system to fetch data is something like this:
(available in `install/Examples/Honeybee/demo-data-source.cxx`)
```
// demo-data-source.cxx //

#include <iostream>
#include <honeybee/honeybee.hh>
namespace hb = honeybee;

int main(int argc, char** argv)
{
    // define data source //
    hb::dripline_pgsql t_data_source("p8_db_user:****@localhost:5432/p8_sc_db");
   
    // construct Sensor Table //
    hb::sensor_table t_sensor_table;
    hb::sensor_config_by_names t_config;
    t_config.load(t_sensor_table, t_data_source.get_data_names());

    // bind sensor entries to DB contents //
    t_data_source.bind(t_sensor_table);
    
    // fetch //
    std::vector<int> t_sensors = t_sensor_table.find_like({{"Alicat"}});
    hb::datetime t_from("2022-01-26T08:00:00"), t_to("2022-01-26T08:01:00");
    std::vector<hb::series> t_series_list = t_data_source.read(t_sensors, t_from, t_to);

    // print //
    for (unsigned i: hb::arange(t_sensors)) {
        std::cout << t_sensor_table[t_sensors[i]].get_name().join(".") << ": ";
        std::cout << t_series_list[i].to_json() << std::endl;
    }
    
    return 0;
}
```
```
$ ./demo-data-source 
sccm.Alicat.Inj.Gas: {
  "start": 1643184000, "length": 60,
  "t": [1.3,11.3,21.3,31.3,41.3,51.3],
  "x": [0.002,0.002,0.002,0.002,0.002,0.002]
}
psia.Alicat.Inj.Gas: {
  "start": 1643184000, "length": 60,
  "t": [1.2,11.2,21.2,31.2,41.2,51.2],
  "x": [7.3,7.4,7.3,7.3,7.2,7.3]
}
degC.Alicat.Inj.Gas: {
  "start": 1643184000, "length": 60,
  "t": [1.3,11.3,21.3,31.3,41.3,51.3],
  "x": [30.85,30.85,30.85,30.85,30.85,30.84]
}
```

### Wrapping Up
The `honeybee_app` class wraps up the data access part for convenience:
```
    class honeybee_app {
      public:
        void add_config_file(const std::string& filepath);
        void add_dripline_db(const std::string& db_uri);
        void add_variable(const std::string& key, const KVariant& value);
        void set_delimiter(const std::string& delimiters);
        std::shared_ptr<sensor_table> get_sensor_table();
        std::shared_ptr<data_source> get_data_source();
        std::vector<std::string> find_like(const std::string a_name);
        series_bundle read(const vector<std::string>& a_sensor_list, double a_start, double a_stop);
```
If no sensor configuration is given by calling `add_config_file()` or `add_dripline_db()`, it will search for (a) configuration file(s) in the directory indicated by the `HONEYBEE_CONFIG_PATH` environmental variable.

The configuration file can contain, in addition to the sensor table configuration, the data source information.
```
# data_source:
#  dripline_psql:
#    uri: p8_db_user:****@localhost:5432/p8_sc_db
#    basename: ATDS

# sensor_table:
#   setup:
#     id: { name: ATDS, label: Atomic Tritium Demonstrator in Seattle }
#     valid_if: date > 20201001

#     section:
#       id: { name: Gas, label: Gas Handling }
#       subsection:
#         id: { name: Inj, label: Injection }
(... more lines)
```

With the environmental variable defined, and the Dripline DB URL is described in a configuration file (even without the sensor table part), the user C++ code becomes very simple.

To look up sensors:
```
#include "honeybee/honeybee.hh"
namespace hb = honeybee;

int main(int argc, char** argv)
{
    hb::honeybee_app t_honeybee;
    for (std::string& t_name: t_honeybee.find_like("mbar.Vac")) {
        std::cout << t_name << std::endl;
    }
...
```

To get data:
```
#include "honeybee/honeybee.hh"
namespace hb = honeybee;

int main(int argc, char** argv)
{
    std::vector<std::string> t_sensors = { "sccm.Alicat.Inj", "K.ThrmCpl.Diss", "mbar.IG.MS" };
    hb::datetime t_from("2022-01-26T08:00:00"), t_to("2022-01-26T08:10:00");

    hb::honeybee_app t_honeybee;
    auto t_series_bundle = t_honeybee.read(t_sensors, t_from, t_to);
...
```

Design and Implementation
=========================
## Sensor Configuration
Sensors are organized in a tree structure. Sensor table can be described in (a) YAML-like configuration file (s).
```
# sensor_table:
#   setup:
#     id: { name: ATDS, label: Atomic Tritium Demonstrator in Seattle }
#     valid_if: date > 20201001

#     section:
#       id: { name: Gas, label: Gas Handling }
#       subsection:
#         id: { name: Inj, label: Injection }
#         module:
#           id: { name: Alicat, label: Alicat Flowmeter }
#           channel:
#             id: { name: sccm, label: Flow }
#           channel:
#             id: { name: degC, label: Temperature }
#           channel:
#             id: { name: psia, label: Pressure }
#         module:
#           id: { name: CC10, label: CC10 Pressure Gauge }
#           channel:
#             id: { name: V, label: LabJunk Voltage }
#           channel:
#             id: { name: mbar, label: Pressure }
(... more lines)
```
(This format will be / cab be replaced with YAML. A major difference from YAML is that multiple child nodes with the same tag name create an array, which is actually convenient and improves readability and writablity...)

Layers are defined in this example like `setup` &rarr; `section` &rarr; `module` &rarr; `channel`, but the layers can be of any depth with any of the following layer tags:
- `experiment`, `setup`, `teststand`, `system`
- `section`, `subsection`, `division`, `segment`, `crate`
- `module`, `device`, `card`, `board`  (`unit` is NOT included on purpose)
- `channel`, `endpoint`, `metric`

Currently Honeybee does not care which layer has which tag, but this might be changed in the future. A good practice will be to use one in the first set for the top-level, and one in the bottom set for data-producing nodes (i.e., leaves of the tree).

Each layer must have an `id`, with at least a `name` and optionally with a `label`. A full sensor name will be a concatenation of the layer names, with an arbitrary separator (typically one of `.`, `_`, `-`, `/`, `:`, `;`), in the reversed order (leaf to root).

An integer sensor number will be assigned for each leaf node by the system at run-time. This assignment of the numbers is not fixed and cannot be used for persistent purposes.

Each layer can have a `valid_if` tag, with a value of the tag being a conditional expression with _variables_. The variables can be provided by users at run-time.

Optional key-value pairs can be added, with a key name staring with `x-`. Explicit binding to a Dripline endpoint, for example, is described with `x-dripline_endpoint`, where the content is specific to the Dripline data source.


## Data Entry Binding, Channel Mapping, and Calibrations
The `data_source::bind()` method manages bindings between the sensor entries and the DB contents, and/or mappings among sensor table entries (_calibrations_). 

If a sensor table is constructed from DB contents only, the sensor names are basically identical to those defined in the data source, optionally with an appended _basename_. If the names in the data source contain delimiters, such as `_`, the names are broken down into a chain, enabling users to use the partial name matching. Moreover, if a configuration file is to be used later, already using a consistent naming structure in the database (i.e., organized in a tree, separated by a delimiter) makes the future configuration simpler; here "convention over configuration" (CoC) takes place. For example, Honeybee can automatically infer the binding between a sensor name of `sccm.Alicat.Inj.Gas.AS.ATDS` and a dripline endpoint of `sccm_Alicat_Gas` in namespace `ATDS`.

Beyond simple data access using names in data source, configuration by file or database becomes necessary. The configuration can contain:
- explicit binding between sensor names and data store entries
- derived sensor entries, whose values are calculated from other sensor values (_calibrations_)
- conditional block depending on external parameters such as date

Note that the calibrations can be used for:
- calibrating the device
- converting units
- mapping between channels (physics quantity to sensor, sensor to digitizer, digitizer to end-point, etc)
- decoding packed bits

See the Quick Tour section for examples.


### Calibration implementation
Calibration is a text property of a sensor, stored in `std::string f_calibration` of the `sensor` class. The text is a lambda expression with a input from another sensor, or inputs from other sensors, in a form of:
```
INPUT: f(INPUT)
```

In a sensor configuration file, this is described as the `default_calibration` property of a channel:
```
#         module:
#           id: { name: ThrmCpl, label: Thermocouple }
#           channel:
#             id: { name: V, label: Voltage }
#             x-dripline_endpoint: V_ThermoCo_Diss_AS
#           channel:
#             id: { name: degC, label: Temperature in Celsius }
#             default_calibration: V:5.1865e-01+V*(-7.0934e+04+V*(-2.4686e+06+V*(-1.3643e+08+V*(-3.3825e+09+V*(-3.5836e+10)))))
#           channel:
#             id: { name: K, label: Temperature in Kelvin }
#             default_calibration: degC:degC+273.15
```
Here the `K.ThrmCpl` has a calibration with an input from `degC` (of `ThrmCpl` by default).

`data_source::bind()` parses the text at run-time (currently using the "Kebap" library) and holds an evaluator object together with a link to input sensor(s).


## Python Binding (Plan)
Export only:
- honeybee_app::find_like()
- honeybee_app::read(), returning a list of Panda's Series
