#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
CONFIG=""
RUN_CMAKE=""

main() {
  resolve_paths
  create_forwarding_scripts
  parse_args "$@"
  setup_dependencies || return 1
  check_hashes || return 1
  run_cmake
  return "$?"
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
  ROOT="$(pwd -P)"
}

create_forwarding_scripts() {
  if [[ ! -f "build.sh" ]]; then
    ln -s "$DIRECTORY/build.sh" build.sh
  fi
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$DIRECTORY/configure.sh" configure.sh
  fi
}

parse_args() {
  local is_dependency=""
  while [[ $# -gt 0 ]]; do
    local arg="$1"
    if [[ "$is_dependency" == "1" ]]; then
      DEPENDENCIES="$arg"
      is_dependency=""
    elif [[ "${arg:0:4}" == "-DD=" ]]; then
      DEPENDENCIES="${arg:4}"
      if [[ -z "$DEPENDENCIES" ]]; then
        echo "Error: -DD requires a path argument."
        return 1
      fi
    elif [[ "$arg" == "-DD" ]]; then
      is_dependency="1"
    else
      CONFIG="$arg"
    fi
    shift
  done
  if [[ "$is_dependency" == "1" ]]; then
    echo "Error: -DD requires a path argument."
    return 1
  fi
  if [[ -z "$CONFIG" ]]; then
    CONFIG="Release"
  fi
  if [[ -z "$DEPENDENCIES" ]]; then
    DEPENDENCIES="$ROOT/Dependencies"
  fi
}

setup_dependencies() {
  if [[ ! -d "$DEPENDENCIES" ]]; then
    mkdir -p "$DEPENDENCIES" || return 1
  fi
  DEPENDENCIES="$(cd "$DEPENDENCIES" && pwd -P)" || return 1
  if [[ -e "$ROOT/Dependencies" ]] &&
      [[ ! "$ROOT/Dependencies" -ef "$DEPENDENCIES" ]] &&
      [[ ! -L "$ROOT/Dependencies" ]]; then
    echo "Error: $ROOT/Dependencies exists and is not a symbolic link."
    return 1
  fi
  pushd "$DEPENDENCIES" > /dev/null || return 1
  "$DIRECTORY/setup.sh" || { popd > /dev/null; return 1; }
  popd > /dev/null
  if [[ ! "$ROOT/Dependencies" -ef "$DEPENDENCIES" ]]; then
    if [[ -L "$ROOT/Dependencies" ]]; then
      rm "$ROOT/Dependencies" || return 1
    fi
    ln -s "$DEPENDENCIES" "$ROOT/Dependencies" || return 1
  fi
}

md5hash() {
  if command -v md5sum >/dev/null; then
    md5sum | cut -d" " -f1
  else
    md5 -r | cut -d" " -f1
  fi
}

check_hashes() {
  if [[ ! -d "CMakeFiles" ]]; then
    mkdir -p CMakeFiles || return 1
    RUN_CMAKE=1
  fi
  check_cmake_hash
  check_file_hash "$CONFIG" "CMakeFiles/config.txt"
  check_file_hash "$DEPENDENCIES" "CMakeFiles/dependencies.txt"
  check_directory_hash "$DIRECTORY/Include" "CMakeFiles/hpp_hash.txt"
  check_directory_hash "$DIRECTORY/Source" "CMakeFiles/cpp_hash.txt"
}

check_cmake_hash() {
  local current_hash
  current_hash=$( (
    cat "$DIRECTORY/CMakeLists.txt"
    if [[ -d "$DIRECTORY/Config" ]]; then
      for f in "$DIRECTORY/Config"/*.cmake; do
        [[ -f "$f" ]] && cat "$f"
      done
      find "$DIRECTORY/Config" -name "CMakeLists.txt" -type f -print0 |
        sort -z | xargs -0 cat 2>/dev/null || true
    fi
  ) | md5hash)
  check_file_hash "$current_hash" "CMakeFiles/cmake_hash.txt"
}

check_file_hash() {
  local current_hash="$1"
  local hash_file="$2"
  if [[ -f "$hash_file" ]]; then
    local cached_hash
    cached_hash=$(< "$hash_file")
    if [[ "$current_hash" != "$cached_hash" ]]; then
      RUN_CMAKE=1
    fi
  else
    RUN_CMAKE=1
  fi
  if [[ "$RUN_CMAKE" == "1" ]]; then
    echo "$current_hash" > "$hash_file"
  fi
}

check_directory_hash() {
  local dir="$1"
  local hash_file="$2"
  if [[ ! -d "$dir" ]]; then
    return 0
  fi
  local current_hash
  current_hash=$(find "$dir" -type f | sort | md5hash)
  check_file_hash "$current_hash" "$hash_file"
}

run_cmake() {
  if [[ "$RUN_CMAKE" == "1" ]]; then
    if ! cmake -S "$DIRECTORY" -DCMAKE_BUILD_TYPE="$CONFIG" \
        -DD="$DEPENDENCIES"; then
      rm -f CMakeFiles/cmake_hash.txt
      return 1
    fi
  fi
}

main "$@"
