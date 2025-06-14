#!/usr/bin/env bash
set -euxo pipefail

# region 0. madlib
function build_madlib() {
  pushd "${BUILD_ROOT}"
  rm -rf madlib
  git clone --branch "${CBDB_MADLIB_BRANCH}" "${CBDB_MADLIB_REPO}"

  mkdir -p madlib/build
  popd

  pushd "${BUILD_ROOT}"/madlib/build
  (
    source "/usr/local/cloudberry-db-devel/greenplum_path.sh"
    export ARCH="${OS_ARCH}"

    cmake -DBOOST_TAR_SOURCE="/opt/boost_1_61_0.tar.gz" \
      -DPYXB_TAR_SOURCE="/opt/PyXB-1.2.6.tar.gz" \
      -DEIGEN_TAR_SOURCE="/opt/3.2.tar.gz" \
      -DCLOUDBERRY_1_PG_CONFIG="/usr/local/cloudberry-db-devel/bin/pg_config" \
      -DCMAKE_BUILD_TYPE=Release \
      -DGPPKG_VER=-1 \
      ..

    ln -sf /usr/bin/python3 /usr/bin/python
    make
    make -j4
    make package
    make gppkg
    ln -sf /usr/bin/python2 /usr/bin/python
  )
  popd
}

function test_madlib() {
  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

source /usr/local/cloudberry-db-devel/greenplum_path.sh
source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh

createdb testdb
gppkg -i "${BUILD_ROOT}"/madlib/build/deploy/gppkg/1/madlib-*.gppkg

/usr/local/cloudberry-db-devel/madlib/bin/madpack -p cloudberry -c 127.0.0.1:7000/testdb install
/usr/local/cloudberry-db-devel/madlib/bin/madpack -p cloudberry -c 127.0.0.1:7000/testdb install-check
EOF
}

function upload_madlib_gppkg() {
  pushd "${BUILD_ROOT}"/madlib/build/deploy/gppkg/1/
  gppkg_path=$(ls madlib-*.gppkg)
  gppkg_filename=$(basename ${gppkg_path} ".gppkg")-${BUILD_ID}.gppkg

  aws s3 cp "${gppkg_path}" \
    "s3://${CBDB_RELEASE_BUCKET}/madlib/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/${gppkg_filename}" \
    --endpoint-url "https://${CBDB_S3_ENDPOINT}" \
    --no-progress --no-verify-ssl
  popd

  cat <<EOF
public_gppkg_download_url="http://${CBDB_RELEASE_BUCKET}.${CBDB_S3_ENDPOINT}/madlib/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/${gppkg_filename}"
EOF
}
# endregion

# region 1. pxf
function build_pxf() {
  pushd "${BUILD_ROOT}"
  rm -rf pxf
  git clone --branch "${CBDB_PXF_BRANCH}" "${CBDB_PXF_REPO}"
  popd

  pushd "${BUILD_ROOT}"/pxf
  (
    wget -q "${CBDB_CACHE_GRADLE}"
    tar -xzf "$(basename ${CBDB_CACHE_GRADLE})" -C "${HOME}"

    export GOPATH="${HOME}"/go
    go install github.com/onsi/ginkgo/ginkgo@v1.16.5
    cp ${GOPATH}/bin/ginkgo /usr/local/bin/

    source "/usr/local/cloudberry-db-devel/greenplum_path.sh"

    make
    make rpm
  )
  popd
}

function test_pxf() {
  chown -R gpadmin:gpadmin "${BUILD_ROOT}/pxf"
  chown -R gpadmin:gpadmin /usr/local/cloudberry-db*
  chown -R gpadmin:gpadmin /opt
  rpm -ivh "${BUILD_ROOT}"/pxf/build/rpmbuild/RPMS/"${OS_ARCH}"/*.rpm
  usermod -aG docker gpadmin

  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

# 这些环境变量不仅 pxf 服务会使用到, cbdb 数据库也会用到, 因此需要在启动数据库之前就要设置
# 因此, 在 test-functons.bash#setup_cluster() 中进行了设定, 实现了启动数据库之前设置这两个环境变量.
# 这里也再次强制设置一下, 重复设置两次也没有问题, 效果是一样的.
# 为什么要换端口? dlagent 一个java进程, 占用了 5888 端口.
# 参考: https://docs.vmware.com/en/VMware-Greenplum-Platform-Extension-Framework/6.7/greenplum-platform-extension-framework/cfghostport.html

echo "export PXF_BASE=${BUILD_ROOT}/pxf/pxf_base" >> \${HOME}/.bashrc
echo "export PXF_PORT=6888" >> \${HOME}/.bashrc

source \${HOME}/.bashrc

pushd /opt
wget -q https://hashdata-releng.obs.cn-north-4.myhuaweicloud.com/cache/msic/pxf/hadoop-3.3.4-update.tar.gz
tar -xzf hadoop-3.3.4-update.tar.gz

wget -q https://hashdata-releng.obs.cn-north-4.myhuaweicloud.com/cache/msic/pxf/apache-hive-3.1.3-bin-update.tar.gz
tar -xzf apache-hive-3.1.3-bin-update.tar.gz

wget -q https://hashdata-releng.obs.cn-north-4.myhuaweicloud.com/cache/msic/pxf/docker-image-mysql-8.0.31-${OS_ARCH}.tar.gz
docker load < ./docker-image-mysql-8.0.31-${OS_ARCH}.tar.gz

wget -q https://hashdata-releng.obs.cn-north-4.myhuaweicloud.com/cache/msic/pxf/docker-image-postgres-15.1-${OS_ARCH}.tar.gz
docker load < ./docker-image-postgres-15.1-${OS_ARCH}.tar.gz

export HADOOP_HOME="/opt/hadoop-3.3.4"
export HADOOP_CONF_PATH="\${HADOOP_HOME}/etc/hadoop"
export HIVE_HOME="/opt/apache-hive-3.1.3-bin"
export HIVE_CONF_PATH="\${HIVE_HOME}/conf"
export PATH="\${HIVE_HOME}/bin:\${HADOOP_HOME}/bin:\${HADOOP_HOME}/sbin:/usr/local/pxf-cbdb1/bin:\${PATH}"
popd

# 初始化 hadoop 环境
hdfs namenode -format 
start-dfs.sh
start-yarn.sh

# 初始化 hive 环境
docker run --name mysql \
  -e MYSQL_ROOT_PASSWORD=123456 \
  -p 3306:3306 \
  -d mysql:8.0.31
sleep 30

docker run --network host --rm mysql:8.0.31 \
  mysql -uroot -h127.0.0.1 -uroot -p123456 -e "create database metastore;"
  
schematool -initSchema -dbType mysql -verbose
nohup hive --service metastore > \${HIVE_HOME}/hive.out 2>&1 &
sleep 30

cp /usr/local/pxf-cbdb1/gpextable/pxf*.sql /usr/local/cloudberry-db-devel/share/postgresql/extension/
cp /usr/local/pxf-cbdb1/gpextable/pxf*.control /usr/local/cloudberry-db-devel/share/postgresql/extension/
cp /usr/local/pxf-cbdb1/gpextable/pxf*.so /usr/local/cloudberry-db-devel/lib/postgresql/

source /usr/local/cloudberry-db-devel/greenplum_path.sh
source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh

pxf prepare
pxf start
pxf cluster register

psql postgres -c "create extension pxf;" 

pushd "\${PXF_BASE}"/servers/default
  cp "\${HADOOP_CONF_PATH}"/core-site.xml .
  cp "\${HADOOP_CONF_PATH}"/hdfs-site.xml .
  cp "\${HADOOP_CONF_PATH}"/mapred-site.xml .
  cp "\${HADOOP_CONF_PATH}"/yarn-site.xml .
  cp "\${HIVE_CONF_PATH}"/hive-site.xml .
popd

pxf cluster sync
pxf cluster stop
pxf cluster start

# 测试访问 hdfs 的功能
hadoop dfs -put "${CBDB_RELEASE_SRC_DIRECTORY}"/pxf/pxf_test.txt /
psql postgres -c "CREATE EXTERNAL TABLE pxf_test(id text, city text)LOCATION ('pxf://pxf_test.txt?PROFILE=hdfs:text')FORMAT 'TEXT' (delimiter=E',');"
psql postgres -c "select * from pxf_test;"

# 测试访问 hive 的功能
hive -f "${CBDB_RELEASE_SRC_DIRECTORY}"/pxf/hive.sql
psql postgres -c "CREATE EXTERNAL TABLE hive_salesinfo2(location text, month text, number_of_orders int, total_sales float8)LOCATION ('pxf://default.sales_info?PROFILE=Hive')FORMAT 'custom' (formatter='pxfwritable_import')"
psql postgres -c "select * from hive_salesinfo2;"

# 测试访问 pg 的功能
docker run --name postgres \
  -e POSTGRES_PASSWORD=123456 \
  -p 5432:5432 \
  -d postgres:15.1
sleep 30

docker run --network host --rm postgres:15.1 \
  psql --command "create table pgtest1(id int);insert into pgtest1 values(20130519);" \
  "host=127.0.0.1 hostaddr=127.0.0.1 port=5432 user=postgres password=123456 dbname=postgres";
  
mkdir -p "\${PXF_BASE}/servers/pg"
pushd "\${PXF_BASE}/servers/pg"
 cp "${CBDB_RELEASE_SRC_DIRECTORY}"/pxf/jdbc-site.xml .
 psql postgres -c "CREATE EXTERNAL TABLE pxf_pg(id int)LOCATION ('pxf://public.pgtest1?PROFILE=Jdbc&SERVER=pg')FORMAT 'CUSTOM' (FORMATTER='pxfwritable_import');"
 psql postgres -c "select * from pxf_pg;"
 popd
EOF
}

function upload_pxf_rpm() {
  pushd "${BUILD_ROOT}"/pxf/build/rpmbuild/RPMS/"${OS_ARCH}"/
  rpm_path=$(ls pxf-*.rpm)
  rpm_filename=$(basename ${rpm_path} ".rpm")-${BUILD_ID}.rpm

  aws s3 cp "${rpm_path}" \
    "s3://${CBDB_RELEASE_BUCKET}/pxf/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/${rpm_filename}" \
    --endpoint-url "https://${CBDB_S3_ENDPOINT}" \
    --no-progress --no-verify-ssl
  popd

  cat <<EOF
public_rpm_download_url="http://${CBDB_RELEASE_BUCKET}.${CBDB_S3_ENDPOINT}/pxf/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/${rpm_filename}"
EOF
}
#endregion

# region 2. zombodb
function build_zombodb() {
  chown -R gpadmin:gpadmin "${BUILD_ROOT}"

  su gpadmin -c bash -- -e <<EOF
set -exo pipefail

source "\$HOME/.cargo/env"

pushd "${BUILD_ROOT}"

rm -rf pgx
git clone --branch "${CBDB_PGX_BRANCH}" "${CBDB_PGX_REPO}"

pushd pgx
(
  source /usr/local/cloudberry-db-devel/greenplum_path.sh
  rustup override set nightly
  cargo install --path cargo-pgx/ -Z no-index-update
)
popd

rm -rf zombodb
git clone --branch "${CBDB_ZOMBODB_BRANCH}" "${CBDB_ZOMBODB_REPO}"

pushd zombodb
(
  source /usr/local/cloudberry-db-devel/greenplum_path.sh

  export PGVER=14
  export OSVER=${OS_TYPE}
  
  rm -rf ~/.pgx || true
  cargo pgx init --pg"\${PGVER}"=\$(which pg_config)
  rustup override set nightly
  cargo build -Z no-index-update --release

  export GPPKGDIR="${BUILD_ROOT}/zombodb/gppkg"
  ./gppkg/package.sh "\${PGVER}" "\${OSVER}"
)
popd

popd
EOF
}

function test_zombodb() {
  su gpadmin -c bash -- -e <<EOF
    set -exo pipefail
    # 启动本地的 elasticsearch 服务
    wget https://artifactory.hashdata.xyz/artifactory/opensource-codes/cbdb-depdencies/centos7/x86_64/elasticsearch-7.17.6.tar.gz
    tar -zxvf elasticsearch-7.17.6.tar.gz
    nohup ./elasticsearch-7.17.6/bin/elasticsearch > es.log 2>&1 &
    pushd zombodb
    (
      source "/home/gpadmin/.cargo/env"
      source /usr/local/cloudberry-db-devel/greenplum_path.sh
      source "${CBDB_SRC_DIRECTORY}"/gpAux/gpdemo/gpdemo-env.sh

      # 服务器配置
      gpconfig -c client_min_messages -v "'warning'"
      gpconfig -c autovacuum -v "'off'" --skipvalidation
      gpconfig -c fsync -v "'off'" --skipvalidation
      gpconfig -c zdb.default_elasticsearch_url -v "'http://localhost:9200/'" --skipvalidation
      gpconfig -c zdb.log_level -v "'LOG'" --skipvalidation
      gpconfig -c zdb.default_replicas -v 0 --skipvalidation
      gpstop -u

      createdb gpadmin
      gppkg -i gppkg/artifacts/zombodb-3000.1.5-cbdb1-centos7-x86_64.gppkg

      # --------------------------------------
      unset TEST
      unset TEST_TYPE

      # heap 默认测试
      ./installcheck

      # heap 遗留测试
      export TEST=all-legacy-tests
      ./installcheck

      # --------------------------------------
      unset TEST
      unset TEST_TYPE

      # ao 默认测试
      export TEST_TYPE=AO
      ./installcheck

      # ao 遗留测试
      export TEST_TYPE=AO
      export TEST=all-legacy-tests
      ./installcheck

      # --------------------------------------
      unset TEST
      unset TEST_TYPE

      # aocs 默认测试
      export TEST_TYPE=AOCS
      ./installcheck

      # aocs 遗留测试
      export TEST=all-legacy-tests
      ./installcheck

      # 单元测试, 需要关闭es服务
      jps | grep Elasticsearch | awk '{print $1}' | xargs kill -9
      export PGVER=14
      gpconfig -r zdb.log_level  --skipvalidation
      cargo test --all --no-default-features --features "pg${PGVER} pg_test" -- --nocapture
    )
    popd
EOF

}

function upload_zombodb_gppkg() {
  pushd "${BUILD_ROOT}"/zombodb/gppkg/artifacts/
  gppkg_path=$(ls zombodb-*.gppkg)
  gppkg_filename=$(basename ${gppkg_path} ".gppkg")-${BUILD_ID}.gppkg

  aws s3 cp "${gppkg_path}" \
    "s3://${CBDB_RELEASE_BUCKET}/zombodb/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/${gppkg_filename}" \
    --endpoint-url "https://${CBDB_S3_ENDPOINT}" \
    --no-progress --no-verify-ssl
  popd

  cat <<EOF
public_gppkg_download_url="http://${CBDB_RELEASE_BUCKET}.${CBDB_S3_ENDPOINT}/zombodb/${OS_TYPE}/${OS_ARCH}/${CBDB_BUILD_TYPE}/${gppkg_filename}"
EOF
}
# endregion
