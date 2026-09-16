#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
CACHE_NAME=""
SETUP_HASH=""
DEPENDENCIES=()

sha256() {
  if command -v sha256sum >/dev/null; then
    sha256sum "$1" | cut -d" " -f1
  else
    shasum -a 256 "$1" | cut -d" " -f1
  fi
}

get_core_count() {
  nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4
}

main() {
  resolve_paths
  check_cache "aspen" || exit 0
  add_dependency "doctest-2.4.12" \
    "https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.zip" \
    "7a7afb5f70d0b749d49ddfcb8a454299a8fcd53e9db9c131abe99b456e88a1fe"
  add_dependency "pybind11-3.0.1" \
    "https://github.com/pybind/pybind11/archive/refs/tags/v3.0.1.zip" \
    "20fb420fe163d0657a262a8decb619b7c3101ea91db35f1a7227e67c426d4c7e"
  add_dependency "Python-3.14.4" \
    "https://www.python.org/ftp/python/3.14.4/Python-3.14.4.tgz" \
    "b4c059d5895f030e7df9663894ce3732bfa1b32cd3ab2883980266a45ce3cb3b" \
    "build_python"
  install_dependencies || return 1
  commit
}

build_python() {
  local cores
  cores=$(get_core_count)
  export CFLAGS="-fPIC"
  ./configure --prefix="$ROOT/Python-3.14.4" --with-ensurepip=no || return 1
  make -j "$cores" || return 1
  make install || return 1
  unset CFLAGS
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

check_cache() {
  CACHE_NAME="$1"
  SETUP_HASH=$(sha256 "$DIRECTORY/setup.sh")
  if [[ -f "cache_files/$CACHE_NAME.txt" ]]; then
    local cached_hash
    cached_hash=$(< "cache_files/$CACHE_NAME.txt")
    if [[ "$SETUP_HASH" == "$cached_hash" ]]; then
      return 1
    fi
  fi
  return 0
}

commit() {
  if [[ ! -d "cache_files" ]]; then
    mkdir -p cache_files || return 1
  fi
  echo "$SETUP_HASH" > "cache_files/$CACHE_NAME.txt"
}

add_dependency() {
  local name="$1"
  local url="$2"
  local hash="$3"
  local build="${4:-}"
  DEPENDENCIES+=("$name|$url|$hash|$build")
}

install_dependencies() {
  for dep in "${DEPENDENCIES[@]}"; do
    IFS='|' read -r name url hash build <<< "$dep"
    download_and_extract "$name" "$url" "$hash" "$build" || return 1
  done
}

download_and_extract() {
  local folder="$1"
  local url="$2"
  local expected_hash="$3"
  local build_hash="$expected_hash $SETUP_HASH"
  local build_func="$4"
  local archive="${url##*/}"
  if [[ -f "$folder/.aspen_build_complete" ]] &&
      [[ "$(< "$folder/.aspen_build_complete")" == "$build_hash" ]]; then
    return 0
  fi
  if [[ ! -f "$folder/.aspen_extract_complete" ]] ||
      [[ "$(< "$folder/.aspen_extract_complete")" != "$expected_hash" ]]; then
    if [[ ! -f "$archive" ]]; then
      curl -fsSL -o "$archive" "$url" || return 1
    fi
    local actual_hash
    actual_hash=$(sha256 "$archive")
    if [[ "$actual_hash" != "$expected_hash" ]]; then
      echo "Error: SHA256 mismatch for $archive."
      echo "  Expected: $expected_hash"
      echo "  Actual:   $actual_hash"
      rm -f "$archive"
      return 1
    fi
    mkdir -p "$folder" || return 1
    if [[ "$archive" == *.zip ]]; then
      unzip -qo "$archive" || return 1
    else
      tar -xf "$archive" --strip-components=1 -C "$folder" || return 1
    fi
    echo "$expected_hash" > "$folder/.aspen_extract_complete" || return 1
  fi
  if [[ -n "$build_func" ]]; then
    pushd "$folder" > /dev/null || return 1
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
  echo "$build_hash" > "$folder/.aspen_build_complete" || return 1
  rm -f "$archive"
}

main "$@"
