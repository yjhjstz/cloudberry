#!/usr/bin/env bash
set -exo pipefail

# region 0. 内部工具
function _build_extension() {
  project_dir=$1
  pushd "${project_dir}"
  (
    source ${CBDB_INSTALL_DIRECTORY}/greenplum_path.sh
    make -j$(nproc)
    make install
  )
  popd >/dev/null
}
# endregion

# region 1. 安装扩展或工具
function install_common_extensions() {
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/pg_gophermeta"
}

function install_hive_connector_extension() {
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/datalake_fdw"
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/hive_connector"
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/datalake_proxy"
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/datalake_agent"
}

function install_pg_gophermeta() {
  pushd "${CBDB_SRC_DIRECTORY}/contrib/pg_gophermeta"
  (
    source ${CBDB_INSTALL_DIRECTORY}/greenplum_path.sh
    make -j$(nproc)
	make install
  )
  popd >/dev/null
}

function install_dfs_tablespac_extension() {
  pushd "${CBDB_SRC_DIRECTORY}/contrib/dfs-tablespace-ext"
  (
    source ${CBDB_INSTALL_DIRECTORY}/greenplum_path.sh
    make USE_PGXS=1 && make USE_PGXS=1 install
  )
  popd >/dev/null
}

function install_postgis_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf postgis-2.5.4
    git clone --branch "${CBDB_POSTGIS_BRANCH}" "${CBDB_POSTGIS_REPO}"

    wget -q "${CBDB_GDAL_BIN_URL}"
    wget -q "${CBDB_GEOS_BIN_URL}"
    wget -q "${CBDB_SFCGAL_BIN_URL}"
    wget -q "${CBDB_CGAL_BIN_URL}"
    wget -q "${CBDB_JSON_C_BIN_URL}"

    tar -xzf "gdal-${CBDB_GDAL_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz" -C /opt
    tar -xzf "geos-${CBDB_GEOS_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz" -C /opt
    tar -xzf "sfcgal-${CBDB_SFCGAL_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz" -C /opt
    tar -xzf "cgal-${CBDB_CGAL_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz" -C /opt
    tar -xzf "json-c-${CBDB_JSON_C_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz" -C /opt

    cat <<EOF >>/etc/ld.so.conf
/opt/gdal-${CBDB_GDAL_VERSION}/lib
/opt/geos-${CBDB_GEOS_VERSION}/lib
/opt/sfcgal-${CBDB_SFCGAL_VERSION}/lib
/opt/sfcgal-${CBDB_SFCGAL_VERSION}/lib64
/opt/cgal-${CBDB_CGAL_VERSION}/lib64
/opt/json-c-${CBDB_JSON_C_VERSION}/lib
EOF
    ldconfig

    pushd postgis-2.5.4/postgis/build/postgis-2.5.4
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    ./autogen.sh
    LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib" ./configure \
      --prefix="${GPHOME}" \
      --with-pgconfig="${GPHOME}"/bin/pg_config \
      --with-raster \
      --without-topology \
      --with-gdalconfig="/opt/gdal-${CBDB_GDAL_VERSION}/bin/gdal-config" \
      --with-geosconfig="/opt/geos-${CBDB_GEOS_VERSION}/bin/geos-config" \
      --with-sfcgal="/opt/sfcgal-${CBDB_SFCGAL_VERSION}/bin/sfcgal-config"

    make -j$(nproc)
    make install
    popd
  )
  popd
}

function install_plr_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf plr
    git clone --branch "${CBDB_PLR_BRANCH}" "${CBDB_PLR_REPO}"

    pushd plr/src
    wget -q "${CBDB_R_BIN_URL}"
    tar -xzf R-${CBDB_R_VERSION}-"${OS_TYPE}"-"${OS_ARCH}".tar.gz -C /usr/lib64

    export LD_LIBRARY_PATH=/usr/lib64/R/lib64/R/lib:/usr/lib64/R/lib64/R/extlib
    export R_HOME=/usr/lib64/R/lib64/R
    export PATH=/usr/lib64/R/bin/:$PATH
    export USE_PGXS=1

    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make clean
    make
    make install
    popd
  )
  popd
}

function install_pljava_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf pljava
    git clone --branch "${CBDB_PLJAVA_BRANCH}" "${CBDB_PLJAVA_REPO}"

    pushd pljava
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make
    make install
    popd
  )
  popd
}

function install_gpbackup() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf gpbackup
    rm -rf gp-common-go-libs
    git clone --branch "${CBDB_GPBACKUP_BRANCH}" "${CBDB_GPBACKUP_REPO}"
    git clone --branch "${CBDB_GP_COMMON_GO_LIBS_BRANCH}" "${CBDB_GP_COMMON_GO_LIBS_REPO}"

    pushd gpbackup
    export GOPATH="${HOME}/go"
    export PATH=${GOPATH}/bin:${PATH}
    make depend
    go install github.com/onsi/ginkgo/ginkgo@latest
    popd

    cp gp-common-go-libs/cluster/cluster.go \
      ${GOPATH}/pkg/mod/github.com/greenplum-db/gp-common-go-libs@v1.0.10/cluster

    pushd gpbackup
    make depend
    make build
    make test
    popd

    cp -a "${GOPATH}"/bin/gpbackup \
      "${GOPATH}"/bin/gprestore \
      "${CBDB_INSTALL_DIRECTORY}"/bin
  )
  popd
}

function install_pg_anonymizer_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf postgresql-anonymizer
    git clone --branch ${CBDB_ANON_BRANCH} "${CBDB_ANON_REPO}"

    pushd postgresql-anonymizer
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make
    make install
    popd
  )
  popd
}

function install_kafka_fdw_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf kafka_fdw
    git clone --branch "${CBDB_KAFKA_FDW_BRANCH}" "${CBDB_KAFKA_FDW_REPO}"

    pushd kafka_fdw
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make
    make install
    popd
  )
  popd
}

function install_pgvector_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf pgvector
    git clone --branch "${CBDB_PGVECTOR_BRANCH}" "${CBDB_PGVECTOR_REPO}"

    pushd pgvector
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make
    make install
    popd
  )
  popd
}

function install_unionstore() {
  if [ -e ${CBDB_SRC_DIRECTORY}/GIT_COMMIT ]; then
    GIT_COMMIT=$(cat "${CBDB_SRC_DIRECTORY}"/GIT_COMMIT)
  fi

  pushd "${BUILD_ROOT}"
  (
    rm -rf unionstore
    git clone --branch "${UNIONSTORE_BRANCH}" "${UNIONSTORE_REPO}"
    pushd unionstore
    rm -rf vendor/unionstore_walredo
    rm -rf vendor/hdfs-native
    git clone -b serverless https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/unionstore_walredo.git vendor/unionstore_walredo
    git clone -b hdfs_native_0.6.0 https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/hdfs-native.git vendor/hdfs-native
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    MAKELEVEL=0 CARGO_BUILD_FLAGS="--config net.git-fetch-with-cli=true" BUILD_CBDB=no make -j$(nproc)
    rm -rf target/release/build
    rm -rf pg_install/build
    rm -rf target/release/deps
    popd
  )
  popd
}

function install_unionstore_extension() {
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/unionstore_ext"
}
function install_unionstore_cloud_extension() {
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/unionstore_ext_cloud"
}

function install_vectorization_extension() {
  _build_extension "${CBDB_SRC_DIRECTORY}/contrib/vectorization"
}

function install_pgpool_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf pgpool-II-${CBDB_PGPOOL_VERSION}*

    wget -q "${CBDB_PGPOOL_SRC_URL}"
    tar -xzf "pgpool-II-${CBDB_PGPOOL_VERSION}.tar.gz"

    pushd pgpool-II-"${CBDB_PGPOOL_VERSION}"
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh

    ./configure
    make

    cp ./src/pgpool "${CBDB_INSTALL_DIRECTORY}"/bin/
    mkdir -p "${CBDB_INSTALL_DIRECTORY}"/etc/ || true
    cp ./src/sample/pgpool.conf.sample "${CBDB_INSTALL_DIRECTORY}"/etc/

    popd
  )
  popd
}

function install_gp_exttable_delimiter_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf gp_exttable_delimiter
    git clone --branch "${CBDB_GP_EXTTABLE_DELIMITER_BRANCH}" "${CBDB_GP_EXTTABLE_DELIMITER_REPO}"

    pushd gp_exttable_delimiter
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make
    make install
    popd
  )
  popd
}

function install_cbdb_ui_extension() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf cloudberryui
    export GOPROXY="https://goproxy.cn,direct"
    git clone --branch "${CBDB_UI_BRANCH}" "${CBDB_UI_REPO}"

    pushd cloudberryui
    go mod tidy
    popd

    pushd cloudberryui/server
    go build -buildvcs=false -o ../cbuiserver
    popd

    pushd cloudberryui
    mkdir -p "${CBDB_INSTALL_DIRECTORY}"/server
    cp -r web "${CBDB_INSTALL_DIRECTORY}"
    cp -r server/bash "${CBDB_INSTALL_DIRECTORY}/server/bash"
    cp -r cloudberryUI "${CBDB_INSTALL_DIRECTORY}/cloudberryUI"
    cp cloudberryUI/cloudberry-ui.bash "${CBDB_INSTALL_DIRECTORY}"
    cp cbuiserver "${CBDB_INSTALL_DIRECTORY}"
    popd
  )
  popd
}

function install_hashcopy() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf hashcopy
    mkdir -p /tmp/go/src /tmp/go/bin /tmp/go/pkg
    export GOPATH="/tmp/go"
    export GOPROXY="https://goproxy.cn,direct"
    git clone --branch "${HASHCOPY_BRANCH}" "${HASHCOPY_REPO}"

    pushd hashcopy
    go mod tidy

    make depend
    make build
    cd /tmp/go/
    cp bin/hashcopy bin/hashcopy_helper bin/gpcopy_helper bin/hashexport bin/hashimport bin/hashcheck "${CBDB_INSTALL_DIRECTORY}/bin"
    popd
  )
  popd
}

function install_vbf() {
  pushd "${CBDB_SRC_DIRECTORY}/contrib/vbf"
  source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
  ./build_vbf.sh --prefix=${CBDB_INSTALL_DIRECTORY}

}

function install_pax_storage() {
  pushd "${BUILD_ROOT}"
  (
    rm -rf pax_storage
    git clone --branch "${PAX_STORAGE_BRANCH}" "${PAX_STORAGE_REPO}"

    pushd pax_storage
    git submodule update --init
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    mkdir -p build && cd build && cmake .. -DENBALE_DEBUG=ON -DBUILD_GTEST=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DBUILD_PAX_FORMAT=ON -DCMAKE_INSTALL_PREFIX=${CBDB_INSTALL_DIRECTORY}
    make
    make install
    popd
  )
  popd
}
function install_storage_am() {

  install_pax_storage
  (

    pushd "${CBDB_SRC_DIRECTORY}/contrib/storage_am"
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh

    #wget https://artifactory.hashdata.xyz/artifactory/opensource-codes/curl/binaries/curl-7.58.0-el7-x86_64.tar.gz
    #tar -C /usr/ --strip-components=1 -xzvf curl-7.58.0-el7-x86_64.tar.gz

    mkdir -p build && cd build
	if [ ${STORAGE_TYPE} = "local" ]; then
		cmake .. -DBUILD_GTEST=ON -DUSE_OPENSSL=ON -DCMAKE_INSTALL_PREFIX=${CBDB_INSTALL_DIRECTORY}
	else
		cmake .. -DBUILD_GTEST=ON -DUSE_OPENSSL=ON -DBUILD_REMOTE_STORAGE=ON -DCMAKE_INSTALL_PREFIX=${CBDB_INSTALL_DIRECTORY}
	fi
    make
    make install-extension
    popd
  )
}


function install_all_extensions() {
#  install_common_extensions

  install_unionstore
#  [ "${CBDB_BUILD_HIVE_CONNECTOR}" == "on" ] && install_hive_connector_extension
#  [ "${CBDB_BUILD_POSTGIS}" == "on" ] && install_postgis_extension
#  [ "${CBDB_BUILD_PLR}" == "on" ] && install_plr_extension
#  [ "${CBDB_BUILD_PLJAVA}" == "on" ] && install_pljava_extension
#  [ "${CBDB_BUILD_GPBACKUP}" == "on" ] && install_gpbackup
#  [ "${CBDB_BUILD_ANON}" == "on" ] && install_pg_anonymizer_extension
#  [ "${CBDB_BUILD_HASHCOPY}" == "on" ] && install_hashcopy
  # [ "${CBDB_BUILD_VECTORIZATION}" == "on" ] && install_vectorization_extension
  # [ "${CBDB_BUILD_KAFKA_FDW}" == "on" ] && install_kafka_fdw_extension
  # [ "${CBDB_BUILD_PGVECTOR}" == "on" ] && install_pgvector_extension
  # [ "${CBDB_BUILD_UNIONSTORE_EXT}" == "on" ] && install_unionstore_extension
  # [ "${CBDB_BUILD_PG_POOL}" == "on" ] && install_pgpool_extension
  # [ "${CBDB_BUILD_GP_EXTTABLE_DELIMITER}" == "on" ] && install_gp_exttable_delimiter_extension
  # [ "${CBDB_BUILD_CBDB_UI}" == "on" ] && install_cbdb_ui_extension
}
# endregion

# region 2. 扩展测试
function test_pg_anonymizer_extension() {
  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

pushd "${BUILD_ROOT}"
(
  rm -rf postgresql-anonymizer
  git clone --branch ${CBDB_ANON_BRANCH} "${CBDB_ANON_REPO}"

  pushd postgresql-anonymizer
  source /usr/local/cloudberry-db-devel/greenplum_path.sh
  source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh
  make installcheck
  popd
)
popd

EOF
}

function test_pljava_extension() {
  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

pushd "${BUILD_ROOT}"
(
  rm -rf pljava
  git clone --branch "${CBDB_PLJAVA_BRANCH}" "${CBDB_PLJAVA_REPO}"

  pushd pljava
  
  echo "export LD_LIBRARY_PATH=/usr/local/cloudberry-db-devel/ext/jdk/jre/lib/amd64/server/:/usr/local/cloudberry-db-devel/ext/jdk/jre/lib/aarch64/server/:\\\${LD_LIBRARY_PATH}" >> /usr/local/cloudberry-db-devel/greenplum_path.sh
  source /usr/local/cloudberry-db-devel/greenplum_path.sh
  source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh
  
  gpconfig -c pljava_classpath -v "/usr/local/cloudberry-db-devel/lib/postgresql/java"
  gpstop -ar


  make test
  popd
)
popd

EOF
}

function test_plr_extension() {
  pushd "${BUILD_ROOT}"
  wget -q ${CBDB_R_BIN_URL}
  tar -xzf R-${CBDB_R_VERSION}-"${OS_TYPE}"-"${OS_ARCH}".tar.gz -C /usr/lib64
  popd

  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

pushd "${BUILD_ROOT}"
(
  rm -rf plr
  git clone --branch "${CBDB_PLR_BRANCH}" "${CBDB_PLR_REPO}"

  pushd plr/src
  
  echo "export LD_LIBRARY_PATH=/usr/lib64/R/lib64/R/lib:/usr/lib64/R/lib64/R/extlib:\\\${LD_LIBRARY_PATH}" >> /usr/local/cloudberry-db-devel/greenplum_path.sh
  export R_HOME=/usr/lib64/R/lib64/R
  export PATH=/usr/lib64/R/bin/:$PATH
  export USE_PGXS=1
  
  source /usr/local/cloudberry-db-devel/greenplum_path.sh
  source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh
  
  gpstop -ar
  
  PGOPTIONS='-c optimizer=off' make USE_PGXS=1 installcheck
  popd
)
popd

EOF
}

function test_postgis_extension() {
  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

pushd "${BUILD_ROOT}"
(
  source /usr/local/cloudberry-db-devel/greenplum_path.sh
  source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh
  psql -c 'create extension postgis'
)
popd

EOF
}

function test_all_extensions() {
  chown -R gpadmin:gpadmin "${BUILD_ROOT}"

  test_pg_anonymizer_extension
  test_pljava_extension
  test_plr_extension
  test_postgis_extension
}
# endregion
