#!/bin/bash
exit_status=0
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ -f "cache_files/aspen.txt" ]; then
  pt="$($STAT $directory/setup.sh | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT cache_files/aspen.txt | grep Modify | awk '{print $2 $3}')"
  if [[ ! "$pt" > "$mt" ]]; then
    exit 0
  fi
fi
let cores="`grep -c "processor" < /proc/cpuinfo`"
if [ ! -d "doctest-2.3.6" ]; then
  wget https://github.com/onqtam/doctest/archive/2.3.6.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip 2.3.6.zip
  else
    exit_status=1
  fi
  rm -f 2.3.6.zip
fi
if [ ! -d "pybind11-2.6.0" ]; then
  wget https://github.com/pybind/pybind11/archive/3e448c0b5e3abcd179781dd718df2bd2340ddb06.zip -O pybind11-2.6.0.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip pybind11-2.6.0.zip
    mv pybind11-3e448c0b5e3abcd179781dd718df2bd2340ddb06 pybind11-2.6.0
  else
    exit_status=1
  fi
  rm -f pybind11-2.6.0.zip
fi
if [ ! -d "Python-3.8.2" ]; then
  wget https://www.python.org/ftp/python/3.8.2/Python-3.8.2.tgz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c Python-3.8.2.tgz | tar -xf -
    pushd Python-3.8.2
    export CFLAGS="-fPIC"
    ./configure --prefix="$root/Python-3.8.2"
    make -j $cores
    make install
    unset CFLAGS
    popd
  else
    exit_status=1
  fi
  rm -rf Python-3.8.2.tgz
fi
if [ ! -d cache_files ]; then
  mkdir cache_files
fi
echo timestamp > cache_files/aspen.txt
exit $exit_status
