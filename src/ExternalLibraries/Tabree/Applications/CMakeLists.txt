set(MyBuildTargets
  ktf-dump
  ktf-embed
  ktf-treedump
)

foreach(MyTarget ${MyBuildTargets})
  add_executable(${MyTarget} ${MyTarget}.cxx)
  target_link_libraries(${MyTarget} PRIVATE TabreeLib)
  target_compile_features(${MyTarget} PRIVATE cxx_std_14)
endforeach(MyTarget)

install(TARGETS ${MyBuildTargets} RUNTIME DESTINATION bin)
