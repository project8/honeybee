add_executable(kebap kebap.cxx)

target_link_libraries(kebap PRIVATE KebapLib)
target_compile_features(kebap PRIVATE cxx_std_14)

install(TARGETS kebap RUNTIME DESTINATION bin)
