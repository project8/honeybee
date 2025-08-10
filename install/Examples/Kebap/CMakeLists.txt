cmake_minimum_required(VERSION 3.14)
project(kebap-examples CXX)

# temporary
set(KebapLib_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/cmake)

find_package(KebapLib REQUIRED)

file(GLOB MyBuildTargets *.cxx)
foreach(source ${MyBuildTargets})
  get_filename_component(target ${source} NAME_WE)
  add_executable(${target} ${target}.cxx)
  target_link_libraries(${target} PRIVATE KebapLib)
  target_compile_features(${target} PRIVATE cxx_std_14)
endforeach(source)
