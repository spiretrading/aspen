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
  if [[ "${CONFIG,,}" == "clean" ]]; then
    clean_build "clean"
    return $?
  fi
  if [[ "${CONFIG,,}" == "reset" ]]; then
    clean_build "reset"
    return $?
  fi
  configure || return 1
  run_build
  return $?
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
    elif [[ "$arg" == "-DD" ]]; then
      is_dependency="1"
    else
      CONFIG="$arg"
    fi
    shift
  done
}

clean_build() {
  local clean_type="$1"
  local clean_error=0
  if [[ "$clean_type" == "reset" ]]; then
    rm -rf Dependencies 2>/dev/null || true
    git clean -ffxd || clean_error=1
  else
    git clean -ffxd -e "*Dependencies*" || clean_error=1
    if [[ -f "Dependencies/cache_files/aspen.txt" ]]; then
      rm "Dependencies/cache_files/aspen.txt" || clean_error=1
    fi
  fi
  return $clean_error
}

configure() {
  if [[ -z "$CONFIG" ]]; then
    if [[ -f "CMakeFiles/config.txt" ]]; then
      CONFIG=$(cat "CMakeFiles/config.txt")
    else
      CONFIG="Release"
    fi
  fi
  case "${CONFIG,,}" in
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
      echo "Error: Invalid configuration \"$CONFIG\"."
      return 1
      ;;
  esac
  if [[ -n "$DEPENDENCIES" ]]; then
    "$DIRECTORY/configure.sh" "$CONFIG" -DD="$DEPENDENCIES"
  else
    "$DIRECTORY/configure.sh" "$CONFIG"
  fi
}

run_build() {
  local jobs
  jobs=$(get_job_count)
  cmake --build "$ROOT" --target install --config "$CONFIG" -- -j"$jobs" ||
    return 1
  echo "$CONFIG" > "CMakeFiles/config.txt"
}

main "$@"
