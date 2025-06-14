#!/usr/bin/env bash
ACTION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source "${ACTION_DIR}/env.bash"
source "${ACTION_DIR}/build-functions.bash"
source "${ACTION_DIR}/extensions-functions.bash"

set -euxo pipefail

trap finish EXIT

function _main() {
  prepare
  #git_clone_cbdb_src
  dump_env
  #build_jansson
  generate_build_number
  configure_database
  build_database
  install_all_extensions
  unittest
  install_etcd
  #install_jansson
  install_jdk
  collect_dependencies
  validation
  package_create_rpm
  upload_rpm_package
}

_main "${@}"
