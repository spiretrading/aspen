#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
CONFIG=""

main() {
  resolve_paths
  parse_args "$@"
  install_python
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
}

parse_args() {
  CONFIG="${1:-Release}"
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
}

install_python() {
  local python_path
  python_path=$(python3 -m site --user-site 2>/dev/null) || {
    echo "Error: Unable to retrieve Python user-site path."
    return 1
  }
  if [[ ! -d "$python_path" ]]; then
    mkdir -p "$python_path" || {
      echo "Error: Unable to create directory \"$python_path\"."
      return 1
    }
  fi
  local source_file="$DIRECTORY/Libraries/$CONFIG/aspen.so"
  if [[ ! -f "$source_file" ]]; then
    echo "Error: Source file \"$source_file\" not found."
    return 1
  fi
  cp "$source_file" "$python_path/" || {
    echo "Error: Failed to copy \"aspen.so\" to \"$python_path\"."
    return 1
  }
}

main "$@"
