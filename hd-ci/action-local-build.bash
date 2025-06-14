#!/usr/bin/env bash
ACTION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source "${ACTION_DIR}/env.bash"
source "${ACTION_DIR}/build-functions.bash"
source "${ACTION_DIR}/extensions-functions.bash"

set -euxo pipefail

trap finish EXIT

function package_create_local_rpm() {
  rm -rf ~/rpmbuild/
  mkdir -p ~/rpmbuild/SOURCES/

  pushd "${CBDB_INSTALL_DIRECTORY}"
  tar -czf ~/rpmbuild/SOURCES/bin_cbdb.tar.gz .
  popd >/dev/null

  export CBDB_VERSION=$("${CBDB_SRC_DIRECTORY}"/getversion --short)

  version=$(echo "${CBDB_VERSION}" | tr '-' '_')
  PATH=/usr/bin:$PATH rpmbuild \
    --define="version ${version}" \
    --define="cbdb_version ${CBDB_VERSION}" \
    -bb "${CBDB_RELEASE_SRC_DIRECTORY}"/.cloudberry-db.spec
}

function upload_local_rpm_package() {
  declare -r rpm_file_name=$(basename "${HOME}/rpmbuild/RPMS/${OS_ARCH}"/cloudberry-db*.rpm)
  upload_rpm_file_name=$(basename "${rpm_file_name}" ".rpm")-"${BUILD_ID}"-"${CBDB_BUILD_TYPE}"-localbuild.rpm

  if [[ "${CBDB_RPM_SUFFIX}" == "external_fts" ]]; then
    upload_rpm_file_name=$(basename "${upload_rpm_file_name}" ".rpm")-"${CBDB_RPM_SUFFIX}".rpm
  fi

  upload_rpm_file_name=$(basename "${upload_rpm_file_name}" ".rpm")-"${STORAGE_TYPE}".rpm
  aws s3 cp \
    ~/rpmbuild/RPMS/"${OS_ARCH}"/"${rpm_file_name}" \
    "s3://${CBDB_RELEASE_BUCKET}/cbdb/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/1.x/rpms/${upload_rpm_file_name}" \
    --endpoint-url "https://${CBDB_S3_ENDPOINT}" \
    --no-progress --no-verify-ssl

  download_url="http://${CBDB_RELEASE_BUCKET}.${CBDB_S3_ENDPOINT}/cbdb/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/1.x/rpms/${upload_rpm_file_name}"
  encode_download_url=$(
    echo ${download_url} | sed s/\+/%2B/g
  )
  cat <<EOF >"${CBDB_RELEASE_SRC_DIRECTORY}/cbdb-artifacts.txt"
os_type=${OS_TYPE}
os_arch=${OS_ARCH}
base_url="https://${CBDB_RELEASE_BUCKET}.${CBDB_S3_ENDPOINT}"
cbdb_major_version=1.x
cbdb_version=${CBDB_VERSION}
public_rpm_download_url="${encode_download_url}"
build_id=${BUILD_ID}
EOF

  cat "${CBDB_RELEASE_SRC_DIRECTORY}/cbdb-artifacts.txt"

}


function local_build_setup_cluster() {
  sudo chown -R gpadmin:gpadmin "${CBDB_SRC_DIRECTORY}"
  echo "127.0.0.1 $(hostname)" | sudo tee -a /etc/hosts
  source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
  set -exo pipefail

  # --- 从这里开始是你原来的核心命令 ---

  {
    set +e
    ssh-keyscan localhost
    ssh-keyscan 0.0.0.0
    ssh-keyscan $(hostname)
    ssh-keyscan 127.0.0.1
    ssh-keyscan $(hostname -I | awk '{print $1}')
    set -e
  } > ~/.ssh/known_hosts

  #echo "export PXF_BASE=${BUILD_ROOT}/pxf/pxf_base" >> \${HOME}/.bashrc
  #echo "export PXF_PORT=6888" >> \${HOME}/.bashrc

  export STATEMENT_MEM=250MB

  # pushd 和 popd 用于临时切换目录，这是一个好习惯
  pushd /builds/code/cbdb_src/gpAux/gpdemo
  LANG=en_US.utf8 PORT_BASE=5555 make create-demo-cluster
  popd
}

function local_gen_env() {
  cat >/tmp/local_build_run_test.sh <<-EOF
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

source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
source /builds/code/cbdb_src/gpAux/gpdemo/gpdemo-env.sh
cd "${CBDB_SRC_DIRECTORY}/src/test/regress"
make ${MAKE_TEST_COMMAND}
EOF

  chmod a+x /tmp/local_build_run_test.sh
}

function local_build_run_test() {
  bash /tmp/local_build_run_test.sh
}

function local_build_regression_test() {
  local_gen_env

  time local_build_run_test
}

function _main() {
  prepare
  #git_clone_cbdb_src
  dump_env
  #build_jansson
  generate_build_number
  configure_local_database
  build_database
  unittest
  install_etcd
  #install_jansson
  install_jdk
  collect_dependencies
  validation
  #package_create_local_rpm
  #upload_local_rpm_package
  export PGUSER="gpadmin"
  export PGPORT=5555
  local_build_setup_cluster

  local_build_regression_test
}

_main "${@}"
