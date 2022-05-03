cmake_minimum_required(VERSION 3.14)
project(honeybee-examples CXX)

# temporary 
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/cmake)

find_package(HoneybeeLib REQUIRED)
find_package(KebapLib REQUIRED)
find_package(TabreeLib REQUIRED)

file(GLOB MyBuildTargets *.cxx)
foreach(source ${MyBuildTargets})
  get_filename_component(target ${source} NAME_WE)
  add_executable(${target} ${target}.cxx)
  target_link_libraries(${target} PRIVATE HoneybeeLib)
  target_compile_features(${target} PRIVATE cxx_std_14)
endforeach(source)
