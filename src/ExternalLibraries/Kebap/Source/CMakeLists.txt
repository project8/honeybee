add_library(KebapLib STATIC
  KPBuiltinFunction.cxx
  KPFunction.cxx
  KPObject.cxx
  KPStandardLibrary.cxx
  KPToken.cxx
  KPValue.cxx
  KPEvaluator.cxx
  KPMathLibrary.cxx
  KPOperator.cxx
  KPStatement.cxx
  KPTokenizer.cxx
  KPExpression.cxx
  KPModule.cxx
  KPParser.cxx
  KPSymbolTable.cxx
  KPTokenTable.cxx
)

set(MyPublicHeaders
  Kebap.h
  KPException.h
  KPMathLibrary.h
  KPOperator.h
  KPStatement.h
  KPTokenizer.h
  KPBuiltinFunction.h
  KPExpression.h
  KPModule.h
  KPParser.h
  KPSymbolTable.h
  KPTokenTable.h
  KPEvaluator.h
  KPFunction.h
  KPObject.h
  KPStandardLibrary.h
  KPToken.h
  KPValue.h
)

target_compile_features(KebapLib PRIVATE cxx_std_14)

target_include_directories(KebapLib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
  
set_target_properties(KebapLib PROPERTIES PUBLIC_HEADER "${MyPublicHeaders}")

install(TARGETS KebapLib
  EXPORT kebaplib-config
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/kebap
)
install(EXPORT kebaplib-config DESTINATION lib/cmake)
