FROM ubuntu:20.04

ARG build_type=Release
ARG make_jobs="-j"

# Set bash as the default shell
SHELL ["/bin/bash", "-c"]

# Install as many dependencies as possible using apt
RUN apt-get update &&\
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential \
        dpkg-dev \
        cmake \
        postgresql-server-dev-all \
        libpq-dev \
        && \
    apt-get clean && \
    /bin/true

COPY ./SensorTable /usr/local/src/SensorTable
COPY ./src /usr/local/src/src

RUN mkdir -p /usr/local/build-externals && \
    cd /usr/local/build-externals && \
    cmake -DCMAKE_BUILD_TYPE=${build_type} \
        -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \ 
        ../src/src/ExternalLibraries && \
    make ${make_jobs} install && \
    /bin/true

RUN mkdir -p /usr/local/build-honeybee && \
    cd /usr/local/build-honeybee && \
    cmake -DCMAKE_BUILD_TYPE=${build_type} \
        -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
        -DCMAKE_PREFIX_PATH=/usr/local/lib/cmake \ 
        ../src/src/Honeybee && \
    make ${make_jobs} install && \
    /bin/true
