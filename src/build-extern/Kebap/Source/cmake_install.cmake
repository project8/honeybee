# Install script for directory: /Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/build-extern/Kebap/Source/libKebapLib.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKebapLib.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKebapLib.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libKebapLib.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kebap" TYPE FILE FILES
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/Kebap.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPException.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPMathLibrary.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPOperator.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPStatement.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPTokenizer.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPBuiltinFunction.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPExpression.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPModule.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPParser.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPSymbolTable.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPTokenTable.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPEvaluator.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPFunction.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPObject.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPStandardLibrary.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPToken.h"
    "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/ExternalLibraries/Kebap/Source/KPValue.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/kebaplib-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/kebaplib-config.cmake"
         "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/build-extern/Kebap/Source/CMakeFiles/Export/c220ae0af1591e9e9e916bba91f25986/kebaplib-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/kebaplib-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/kebaplib-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake" TYPE FILE FILES "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/build-extern/Kebap/Source/CMakeFiles/Export/c220ae0af1591e9e9e916bba91f25986/kebaplib-config.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake" TYPE FILE FILES "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/build-extern/Kebap/Source/CMakeFiles/Export/c220ae0af1591e9e9e916bba91f25986/kebaplib-config-noconfig.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/nobeltsegai/Documents/CENPA/project-8/honeybee/src/build-extern/Kebap/Source/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
