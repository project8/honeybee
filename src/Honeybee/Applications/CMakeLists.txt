set(MyBuildTargets
  hb-get-data
  hb-list-sensors
)

foreach(MyTarget ${MyBuildTargets})
  add_executable(${MyTarget} ${MyTarget}.cxx)
  target_link_libraries(${MyTarget} PRIVATE HoneybeeLib)
  target_compile_features(${MyTarget} PRIVATE cxx_std_14)
endforeach(MyTarget)

install(TARGETS ${MyBuildTargets} RUNTIME DESTINATION bin)
