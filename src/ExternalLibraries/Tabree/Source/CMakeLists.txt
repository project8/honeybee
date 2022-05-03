add_library(TabreeLib STATIC
  KArgumentList.cxx
  KTabree.cxx
  KTreeFile.cxx
  KInifileTreeFormat.cxx
  KTabreeFile.cxx
  KTreeFormat.cxx
  KJsonParser.cxx
  KTabreeFormat.cxx
  KTreeSerializer.cxx
  KJsonTreeFormat.cxx
  KTabreeFormatProcessor.cxx
  KTreeWalker.cxx
  KKtfTreeFormat.cxx
  KTreeBuilder.cxx
  KVariant.cxx
  KTable.cxx
  KTree.cxx
  KXpvpTreeFormat.cxx
)

set(MyPublicHeaders
  KArgumentList.h
  KJsonTreeFormat.h
  KTabreeFormat.h
  KTreeFile.h
  KTreeWalker.h
  KException.h
  KKtfTreeFormat.h
  KTabreeFormatProcessor.h
  KTreeFormat.h
  KVariant.h
  KInifileTreeFormat.h
  KTable.h
  KTabree.h
  KTree.h
  KXpvpTreeFormat.h
  KJsonParser.h
  KTabreeFile.h
  KTreeBuilder.h
  KTreeSerializer.h
)

target_compile_features(TabreeLib PRIVATE cxx_std_14)

target_include_directories(TabreeLib INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
  

set_target_properties(TabreeLib PROPERTIES PUBLIC_HEADER "${MyPublicHeaders}")

install(TARGETS TabreeLib
  EXPORT tabreelib-config
  LIBRARY DESTINATION lib
  INCLUDES DESTINATION include
  PUBLIC_HEADER DESTINATION include/tabree
)

install(EXPORT tabreelib-config DESTINATION lib/cmake)

