cmake_minimum_required(VERSION 3.14)
project(tabree CXX)

add_subdirectory(Source)
add_subdirectory(Applications)

add_compile_options(-Wall -Wextra -Werror)

install(DIRECTORY Examples/ DESTINATION Examples/Tabree)
