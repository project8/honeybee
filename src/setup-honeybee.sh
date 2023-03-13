#! /bin/bash


## Set default install dir (Honeybee/install)
BASEDIR=`cd ..; pwd`
if [ ! -d ExternalLibraries -o ! -d Honeybee ]; then \
    echo "ERROR: invalid location to run the script: " `pwd`
    exit -1
fi
INSTALLDIR=${BASEDIR}/install
echo "Honeybee will be installed at " ${INSTALLDIR}
read -r -p "Are you sure? [y/N] " response
if [ x$response != "xy" ]; then
    exit 0
fi


## Add CMake Search Dir ##
if [ x"${CMAKE_PREFIX_PATH}" != x ]; then
    CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}:${INSTALLDIR}/lib/cmake
else
    CMAKE_PREFIX_PATH=${INSTALLDIR}/lib/cmake
fi


## Compile and Install External Libraries ##
cd ${BASEDIR}/src
cmake -B ./build-extern -D CMAKE_INSTALL_PREFIX=${INSTALLDIR} ./ExternalLibraries || exit -1
cd ./build-extern && make -j install || exit -1


## Compile and Install Honeybee ##
cd ${BASEDIR}/src
cmake -B ./build-honeybee -D CMAKE_INSTALL_PREFIX=${INSTALLDIR} -D CMAKE_PREFIX_PATH=${INSTALLDIR}/lib ./Honeybee || exit -1
cd ./build-honeybee && make -j install || exit -1
