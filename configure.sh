#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ ! -d  CMakeFiles ]; then
  echo "Error: CMake needs to run before configuration."
  exit 1
fi
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
for i in "$@"; do
  case $i in
    -DD=*)
      dependencies="${i#*=}"
      shift
      ;;
  esac
done
if [ "$dependencies" = "" ]; then
  dependencies="$root/Dependencies"
fi
if [ ! -d "$dependencies" ]; then
  mkdir -p "$dependencies"
fi
pushd "$dependencies"
if [ ! -f "last_check.txt" ]; then
  run_setup=1
else
  pt="$($STAT $directory/setup.sh | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT last_check.txt | grep Modify | awk '{print $2 $3}')"
  if [ "$pt" \> "$mt" ]; then
    run_setup=1
  fi
fi
if [ "$run_setup" = "1" ]; then
  "$directory"/setup.sh
  echo "timestamp" > last_check.txt
fi
popd
if [ "$dependencies" != "$root/Dependencies" ] && [ ! -d Dependencies ]; then
  rm -rf Dependencies
  ln -s "$dependencies" Dependencies
fi
if [[ "$@" != "" ]]; then
  configuration="-DCMAKE_BUILD_TYPE=$@"
fi
include_count=$(find $directory/Include -name "*.hpp" | wc -l)
if [ -f "CMakeFiles/hpp_count.txt" ]; then
  hpp_count=$(cat "CMakeFiles/hpp_count.txt")
  if [ "$include_count" != "$hpp_count" ]; then
    run_cmake=1
  fi
else
  run_cmake=1
fi
if [ "$run_cmake" = "1" ]; then
  echo $include_count > "CMakeFiles/hpp_count.txt"
fi
source_count=$(find $directory/Source -name "*.cpp" | wc -l)
if [ -f "CMakeFiles/cpp_count.txt" ]; then
  cpp_count=$(cat "CMakeFiles/cpp_count.txt")
  if [ "$source_count" != "$cpp_count" ]; then
    run_cmake=1
  fi
else
  run_cmake=1
fi
if [ "$run_cmake" = "1" ]; then
  echo $source_count > "CMakeFiles/cpp_count.txt"
fi
if [ "$run_cmake" = "1" ]; then
  cmake -S "$directory" $configuration -DD=$dependencies
fi
