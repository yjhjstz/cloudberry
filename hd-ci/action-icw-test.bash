#!/usr/bin/env bash
ACTION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source "${ACTION_DIR}/env.bash"
source "${ACTION_DIR}/build-functions.bash"
source "${ACTION_DIR}/test-functions.bash"

set -euxo pipefail
function on_clean() {
  if [ $? -ne 0 ]; then
    sleep 21600
  fi
  echo "finish callback: sync filesystem before exit"
  sync

}
trap on_clean EXIT

function _main() {
  source "${CBDB_RELEASE_SRC_DIRECTORY}"/cbdb-artifacts.txt
  prepare
  install_cbdb_rpm
  #git_clone_cbdb_src

  (
    export CONFIGURE_FLAGS="--disable-cassert --enable-tap-tests --enable-debug-extensions"
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    configure_database
  )

  export PGUSER="gpadmin"
  setup_cluster

  regression_test

  if [ ${DR_TEST_ENABLE} == "false" ]; then
    export_log
  fi

}

_main "${@}"
