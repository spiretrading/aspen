#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
root="$(pwd)"
if [ ! -d "Catch2-2.6.1" ]; then
  git clone --branch v2.6.1 https://github.com/catchorg/Catch2.git Catch2-2.6.1
fi
if [ ! -d "pybind11-2.2.4" ]; then
  git clone --branch v2.2.4 https://github.com/pybind/pybind11.git pybind11-2.2.4
fi
if [ ! -d "Python-3.6.7" ]; then
  wget https://www.python.org/ftp/python/3.6.7/Python-3.6.7.tgz --no-check-certificate
  if [ -f Python-3.6.7.tgz ]; then
    gzip -d -c Python-3.6.7.tgz | tar -xf -
    pushd Python-3.6.7
    export CFLAGS="-fPIC"
    ./configure --prefix="$root/Python-3.6.7"
    make -j $cores
    make install
    unset CFLAGS
    popd
    rm Python-3.6.7.tgz
  fi
fi
