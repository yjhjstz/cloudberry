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

function local_build_prepare() {
  rm -rf /etc/yum.repos.d/vagrant.repo || true
  rm -rf /etc/yum.repos.d/docker-ce.repo || true

  # 主要缓存了 pljava 和 hive-connector的jar包
  wget -q "${CBDB_CACHE_M2}"
  tar -xzf "$(basename ${CBDB_CACHE_M2})" -C "${HOME}"

  rm -rf "${CBDB_INSTALL_DIRECTORY}"

  #wget https://artifactory.hashdata.xyz/artifactory/opensource-codes/curl/binaries/curl-7.58.0-el7-x86_64.tar.gz
  #tar -C /usr/ --strip-components=1 -xzvf curl-7.58.0-el7-x86_64.tar.gz
}

function local_build_setup_cluster() {
  chown -R gpadmin:gpadmin "${CBDB_SRC_DIRECTORY}"
  echo "127.0.0.1 $(hostname)" >>/etc/hosts
  source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
  su gpadmin -c bash -- -e <<EOF
  set -exo pipefail
  {
    ssh-keyscan localhost
    ssh-keyscan 0.0.0.0
    ssh-keyscan $(hostname)
    ssh-keyscan 127.0.0.1
    ssh-keyscan $(hostname -I | awk '{print $1}')
  } > ~/.ssh/known_hosts
  echo "export PXF_BASE=${BUILD_ROOT}/pxf/pxf_base" >> \${HOME}/.bashrc
  echo "export PXF_PORT=6888" >> \${HOME}/.bashrc
  source \${HOME}/.bashrc
  export STATEMENT_MEM=250MB
  pushd /builds/code/cbdb_src/gpAux/gpdemo
  LANG=en_US.utf8 make create-demo-cluster
  popd
EOF
}

function local_gen_env() {
  cat >/opt/local_build_run_test.sh <<-EOF
trap look4diffs ERR

function look4diffs() {
    diff_files=\`find .. -name regression.diffs\`

    for diff_file in \${diff_files}; do
    if [ -f "\${diff_file}" ]; then
      cat <<-FEOF

        ======================================================================
        DIFF FILE: \${diff_file}
        ----------------------------------------------------------------------

        \$(cat "\${diff_file}")
FEOF
    fi
    done
    exit 1
}

source ${CBDB_INSTALL_DIRECTORY}/greenplum_path.sh
source /builds/code/cbdb_src/gpAux/gpdemo/gpdemo-env.sh
cd "${CBDB_SRC_DIRECTORY}/src/test/regress"
make ${MAKE_TEST_COMMAND}
EOF

  chmod a+x /opt/local_build_run_test.sh
}

function local_build_run_test() {
  su -l gpadmin -c "bash /opt/local_build_run_test.sh"
}

function local_build_regression_test() {
  local_gen_env

  time local_build_run_test

  if [ "${DUMP_DB}" == "true" ]; then
    mkdir sqldump
    chmod 777 sqldump
    su gpadmin -c "${CBDB_RELEASE_SRC_DIRECTORY}"/scripts/dumpdb.bash
  fi
}

function local_build_export_log(){
  mkdir -p ${CI_PROJECT_DIR}/log/master
  mkdir -p ${CI_PROJECT_DIR}/log/standby
  mkdir -p ${CI_PROJECT_DIR}/log/qe
  mkdir -p ${CI_PROJECT_DIR}/log/mirror

  cp /code/cbdb_src/gpAux/gpdemo/datadirs/qddir/demoDataDir-1/log/* ${CI_PROJECT_DIR}/log/master/
  cp /code/cbdb_src/gpAux/gpdemo/datadirs/dbfast1/demoDataDir0/log/* ${CI_PROJECT_DIR}/log/qe/
  cp /code/cbdb_src/gpAux/gpdemo/datadirs/dbfast2/demoDataDir1/log/* ${CI_PROJECT_DIR}/log/qe/
  cp /code/cbdb_src/gpAux/gpdemo/datadirs/dbfast3/demoDataDir2/log/* ${CI_PROJECT_DIR}/log/qe/

  cp /code/cbdb_src/gpAux/gpdemo/datadirs/standby/log/* ${CI_PROJECT_DIR}/log/standby/
  cp /code/cbdb_src/gpAux/gpdemo/datadirs/dbfast_mirror1/demoDataDir0/log/* ${CI_PROJECT_DIR}/log/mirror/
  cp /code/cbdb_src/gpAux/gpdemo/datadirs/dbfast_mirror2/demoDataDir1/log/* ${CI_PROJECT_DIR}/log/mirror/
  cp /code/cbdb_src/gpAux/gpdemo/datadirs/dbfast_mirror3/demoDataDir2/log/* ${CI_PROJECT_DIR}/log/mirror/
}

function _main() {
  source "${CBDB_RELEASE_SRC_DIRECTORY}"/cbdb-artifacts.txt
  local_build_prepare
  #install_cbdb_rpm

  (
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    local_build_configure_database
  )

  export PGUSER="gpadmin"
  local_build_setup_cluster

  local_build_regression_test

  local_build_export_log
}

_main "${@}"
