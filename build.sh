#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
CONFIG=""

get_job_count() {
  local cores mem_gb jobs
  cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
  if [[ -f /proc/meminfo ]]; then
    mem_gb=$(awk '/MemTotal/ {print int($2/8388608)}' /proc/meminfo)
  else
    local mem_bytes
    mem_bytes=$(sysctl -n hw.memsize 2>/dev/null || echo 8589934592)
    mem_gb=$(( mem_bytes / 8589934592 ))
  fi
  cores=$(( cores / 2 + 1 ))
  jobs=$(( cores < mem_gb ? cores : mem_gb ))
  echo $(( jobs > 0 ? jobs : 1 ))
}

main() {
  resolve_paths
  parse_args "$@"
  shopt -s nocasematch
  if [[ "$CONFIG" == "clean" ]]; then
    shopt -u nocasematch
    clean_build "clean"
    return $?
  fi
  if [[ "$CONFIG" == "reset" ]]; then
    shopt -u nocasematch
    clean_build "reset"
    return $?
  fi
  shopt -u nocasematch
  configure || return 1
  generated_files begin || return 1
  local build_error=0
  run_build || build_error=$?
  generated_files end || return 1
  return "$build_error"
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
}

clean_build() {
  local clean_type="$1"
  local clean_error=0
  if [[ -f "$ROOT/CMakeCache.txt" ]]; then
    local scripts=("$ROOT"/CMakeFiles/aspen_clean_*.cmake)
    if [[ ! -f "${scripts[0]}" ]]; then
      CONFIG=""
      configure || return 1
      scripts=("$ROOT"/CMakeFiles/aspen_clean_*.cmake)
    fi
    if [[ ! -f "${scripts[0]}" ]]; then
      echo "Error: Configuration did not generate cleanup scripts."
      return 1
    fi
    generated_files begin || return 1
    for script in "${scripts[@]}"; do
      cmake -P "$script" || clean_error=1
    done
    generated_files end || return 1
  fi
  if [[ "$clean_error" == "0" ]]; then
    generated_files clean || clean_error=1
  fi
  if [[ "$clean_error" == "0" && "$clean_type" == "reset" ]]; then
    cmake -DBUILD_DIRECTORY:PATH="$ROOT" \
      -P "$DIRECTORY/Config/reset.cmake" || clean_error=1
  fi
  return "$clean_error"
}

generated_files() {
  cmake -DBUILD_DIRECTORY:PATH="$ROOT" \
    -DDEPENDENCIES_DIRECTORY:PATH="$DEPENDENCIES" -DACTION="$1" \
    -P "$DIRECTORY/Config/generated_files.cmake"
}

configure() {
  if [[ -z "$CONFIG" ]]; then
    if [[ -f "CMakeFiles/config.txt" ]]; then
      CONFIG=$(< "CMakeFiles/config.txt")
    else
      CONFIG="Release"
    fi
  fi
  shopt -s nocasematch
  case "$CONFIG" in
    release)
      CONFIG="Release"
      ;;
    debug)
      CONFIG="Debug"
      ;;
    relwithdebinfo)
      CONFIG="RelWithDebInfo"
      ;;
    minsizerel)
      CONFIG="MinSizeRel"
      ;;
    *)
      shopt -u nocasematch
      echo "Error: Invalid configuration \"$CONFIG\"."
      return 1
      ;;
  esac
  shopt -u nocasematch
  if [[ -n "$DEPENDENCIES" ]]; then
    "$DIRECTORY/configure.sh" "$CONFIG" -DD="$DEPENDENCIES"
  else
    "$DIRECTORY/configure.sh" "$CONFIG"
  fi
}

run_build() {
  local jobs
  jobs=$(get_job_count)
  cmake --build "$ROOT" --config "$CONFIG" --parallel "$jobs" || return 1
  cmake --install "$ROOT" --config "$CONFIG" || return 1
  echo "$CONFIG" > "CMakeFiles/config.txt"
}

main "$@"
