#!/bin/bash
set -o errexit
set -o pipefail
root=$(pwd)
for i in "$@"; do
  case $i in
    -DD=*)
      dependencies="${i#*=}"
      shift
      ;;
    *)
      config="$i"
      shift
      ;;
  esac
done
if [ "$config" = "" ]; then
  config="Release"
fi
if [ "$config" = "clean" ]; then
  git clean -ffxd -e *Dependencies*
  if [ -f "Dependencies/last_check.txt" ]; then
    rm "Dependencies/last_check.txt"
  fi
elif [ "$config" = "reset" ]; then
  git clean -ffxd
  if [ -f "Dependencies/last_check.txt" ]; then
    rm "Dependencies/last_check.txt"
  fi
else
  cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
  mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 8388608"
  jobs="$(($cores<$mem?$cores:$mem))"
  if [ "$dependencies" != "" ]; then
    cmake -S "$root" -DCMAKE_BUILD_TYPE=$config -DD=$dependencies
  else
    cmake -S "$root" -DCMAKE_BUILD_TYPE=$config
  fi
  cmake --build "$root" --target install -- -j$jobs
fi
