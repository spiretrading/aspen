#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
CONFIG=""

main() {
  DIRECTORY="$(pwd -P)"
  parse_args "$@"
  install_python
}

parse_args() {
  CONFIG="${1:-Release}"
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
