add_library(HoneybeeLib STATIC
  honeybee.cc
  calibration.cc
  data_source.cc
  pgsql.cc
  sensor_table.cc
  series.cc
  utils.cc
)

set(MyPublicHeaders
  honeybee.hh
  calibration.hh
  data_source.hh
  pgsql.hh
  sensor_table.hh
  series.hh
  utils.hh
)

target_compile_features(HoneybeeLib PRIVATE cxx_std_14)

find_package(TabreeLib REQUIRED)
find_package(KebapLib REQUIRED)
find_package(PostgreSQL REQUIRED)

target_include_directories(HoneybeeLib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  ${PostgreSQL_INCLUDE_DIRS}
  ${KebapLib_INCLUDE_DIRS}
  ${TabreeLib_INCLUDE_DIRS}
)
  
target_link_libraries(HoneybeeLib PUBLIC TabreeLib KebapLib ${PostgreSQL_LIBRARIES})

set_target_properties(HoneybeeLib PROPERTIES PUBLIC_HEADER "${MyPublicHeaders}")

install(TARGETS HoneybeeLib
  EXPORT honeybeelib-config
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/honeybee
)
install(EXPORT honeybeelib-config DESTINATION lib/cmake)
