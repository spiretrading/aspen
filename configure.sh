#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
CONFIG=""
RUN_CMAKE=""
HASH_FILES=()
HASH_VALUES=()

main() {
  resolve_paths
  create_forwarding_scripts
  parse_args "$@"
  setup_dependencies || return 1
  if [[ "${ASPEN_SKIP_CMAKE:-}" == "1" ]]; then
    return 0
  fi
  generated_files begin || return 1
  local configure_error=0
  configure_build || configure_error=$?
  generated_files end || return 1
  return "$configure_error"
}

configure_build() {
  check_hashes || return 1
  run_cmake || return 1
  commit_hashes
}

generated_files() {
  cmake -DBUILD_DIRECTORY:PATH="$ROOT" \
    -DDEPENDENCIES_DIRECTORY:PATH="$DEPENDENCIES" -DACTION="$1" \
    -P "$DIRECTORY/Config/generated_files.cmake"
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
  if [[ ! -f "install_python.sh" ]]; then
    ln -s "$DIRECTORY/install_python.sh" install_python.sh
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
    if [[ -f "CMakeFiles/config.txt" ]]; then
      CONFIG=$(< "CMakeFiles/config.txt")
    else
      CONFIG="Release"
    fi
  fi
  shopt -s nocasematch
  case "$CONFIG" in
    release) CONFIG="Release" ;;
    debug) CONFIG="Debug" ;;
    relwithdebinfo) CONFIG="RelWithDebInfo" ;;
    minsizerel) CONFIG="MinSizeRel" ;;
    *)
      shopt -u nocasematch
      echo "Error: Invalid configuration \"$CONFIG\"."
      return 1
      ;;
  esac
  shopt -u nocasematch
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
  local scripts=(CMakeFiles/aspen_clean_*.cmake)
  if [[ ! -f "${scripts[0]}" ]]; then
    RUN_CMAKE=1
  fi
  if [[ ! -f "CMakeCache.txt" ]]; then
    RUN_CMAKE=1
  else
    local cached_config="" configuration_types="" key value
    while IFS='=' read -r key value; do
      case "$key" in
        CMAKE_BUILD_TYPE:*) cached_config="${value%$'\r'}" ;;
        CMAKE_CONFIGURATION_TYPES:*) configuration_types="${value%$'\r'}" ;;
      esac
    done < CMakeCache.txt
    if [[ -z "$configuration_types" && "$cached_config" != "$CONFIG" ]]; then
      RUN_CMAKE=1
    fi
  fi
  if [[ ! -d "CMakeFiles" ]]; then
    mkdir -p CMakeFiles || return 1
    RUN_CMAKE=1
  fi
  check_file_hash "$CONFIG" "CMakeFiles/config.txt"
  check_file_hash "$DEPENDENCIES" "CMakeFiles/dependencies.txt"
  check_directory_hash "$DIRECTORY/Include" "CMakeFiles/hpp_hash.txt"
  check_directory_hash "$DIRECTORY/Source" "CMakeFiles/cpp_hash.txt"
  check_cmake_hash
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
  HASH_FILES+=("$hash_file")
  HASH_VALUES+=("$current_hash")
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
    rm -f CMakeFiles/cmake_hash.txt || return 1
    cmake -S "$DIRECTORY" -DCMAKE_BUILD_TYPE="$CONFIG" -DD="$DEPENDENCIES" ||
      return 1
  fi
}

commit_hashes() {
  if [[ "$RUN_CMAKE" == "1" ]]; then
    local i
    for ((i = 0; i < ${#HASH_FILES[@]}; ++i)); do
      printf '%s\n' "${HASH_VALUES[i]}" > "${HASH_FILES[i]}" || return 1
    done
  fi
}

main "$@"
