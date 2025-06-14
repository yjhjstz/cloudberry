#!/usr/bin/env bash
ACTION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source "${ACTION_DIR}/env.bash"
set -euxo pipefail

function setup_minio() {
  mkdir -p ${HOME}/minio
  export MINIO_ROOT_USER=admin
  export MINIO_ROOT_PASSWORD=12345678
  nohup minio server --address ":9000" --console-address ":9001" ${HOME}/minio/data > ${HOME}/minio/minio.log 2>&1 &
  sleep 10
  mc alias set myminio http://localhost:9000 admin 12345678
  mc mb myminio/testci

  if [ ${DR_TEST_ENABLE} == "true" ]; then
    mc mb myminio/testci-master
    mc mb myminio/testci-slave
    mc mb myminio/replication-slot
  fi
}


function setup_hdfs() {
  chown -R gpadmin:gpadmin /opt
  su gpadmin -c bash -- -e <<EOF
pushd /opt
  wget -q https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/pxf/hadoop-3.3.4-update.tar.gz
  tar -xzf hadoop-3.3.4-update.tar.gz
  export HADOOP_HOME="/opt/hadoop-3.3.4"
  export HADOOP_CONF_PATH="\${HADOOP_HOME}/etc/hadoop"
  export PATH="\${HADOOP_HOME}/bin:\${HADOOP_HOME}/sbin:\${PATH}"
popd
  hdfs namenode -format 
  start-dfs.sh
  start-yarn.sh
  hadoop fs -mkdir /testci
EOF
  sed -i 's/^test: cdb_send_relstat$/#&/' ${CBDB_SRC_DIRECTORY}/src/test/regress_cloud/parallel_schedule_remote
}

function setup_time_travel() {
  sed -i 's/#track_commit_timestamp/track_commit_timestamp/' "${CBDB_INSTALL_DIRECTORY}"/share/postgresql/postgresql.conf.sample
  sed -i 's/#time_travel_minutes/time_travel_minutes/' "${CBDB_INSTALL_DIRECTORY}"/share/postgresql/postgresql.conf.sample
}

function setup_cluster() {
  source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
  if [ ${STORAGE_TYPE} == "oss" ]; then
    setup_minio
  elif [ ${STORAGE_TYPE} == "hdfs" ]; then
    setup_hdfs
  fi
  if [ ${TIME_TRAVEL} == "on" ]; then
    setup_time_travel
  fi
  pushd /usr/local/cloudberry-db-devel/unionstore
  mkdir -p target && cp -r release/ target/
  chown -R gpadmin:gpadmin "${CBDB_SRC_DIRECTORY}"
  echo "127.0.0.1 $(hostname)" >>/etc/hosts
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
  echo "export unionstore_install_dir=/usr/local/cloudberry-db-devel/unionstore" >> \${HOME}/.bashrc
  echo "export NEON_REPO_DIR=\${HOME}/data/unionstore/pageserver" >> \${HOME}/.bashrc
  source \${HOME}/.bashrc 

  source /usr/local/cloudberry-db-devel/greenplum_path.sh
  export STATEMENT_MEM=250MB

  pushd "${CBDB_SRC_DIRECTORY}"
  if [ "${DR_TEST_ENABLE}" == "true" ]; then
    source "${CBDB_SRC_DIRECTORY}"/gpAux/gpclouddemo/cdc_master_env.sh
    make create-clouddemo-cluster
    source "${CBDB_SRC_DIRECTORY}"/gpAux/gpclouddemo/cdc_backup_env.sh
    make create-clouddemo-cluster
  else
    LANG=en_US.utf8 make create-clouddemo-cluster
  fi

  createdb gpadmin

  popd
  source "${CBDB_SRC_DIRECTORY}"/gpAux/gpclouddemo/cloud-gpdemo-env.sh
EOF
}

function init_heap_env() {
  echo "INIT HEAP ENV"

  cat >/opt/run_test.sh <<-EOF
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

source /usr/local/cloudberry-db-devel/greenplum_path.sh
source /code/cbdb_src/gpAux/gpclouddemo/cloud-gpdemo-env.sh
cd "${CBDB_SRC_DIRECTORY}"
psql postgres -c "create warehouse test;"
sleep 30
psql postgres -c "set warehouse = 'test';"
psql postgres -c "create database regression;"
pushd src/test/regress
make
popd
EOF

cat >>/opt/run_test.sh <<-EOF
mkdir -p /home/gpadmin/heaptestdata
psql postgres -f '/opt/run_test.sql'
make ${MAKE_TEST_COMMAND}
EOF

cat >>/opt/run_test.sql <<-EOF
set c4139d65.f116.a4e0.c172.fb3024c97f04 = true;
CREATE TABLESPACE regress_hashdata_tablespace_test location '/home/gpadmin/newtestdata' with(storage='cloud');
GRANT ALL ON TABLESPACE regress_hashdata_tablespace_test TO public;
GRANT ALL ON TABLESPACE default_cloud_tablespace TO public;
CREATE TPSERVER regress_tpserver_test;
GRANT ALL ON TPSERVER regress_tpserver_test TO public;
EOF

  chmod a+x /opt/run_test.sh
  chmod a+x /opt/run_test.sql
}

function gen_env() {
  cat >/opt/run_test.sh <<-EOF
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

source /usr/local/cloudberry-db-devel/greenplum_path.sh
source /code/cbdb_src/gpAux/gpclouddemo/cloud-gpdemo-env.sh
cd "${CBDB_SRC_DIRECTORY}"
psql postgres -c "create warehouse test;"
sleep 30
psql postgres -c "set warehouse = 'test';"
psql postgres -c "create database regression;"
pushd src/test/regress
make
popd
EOF

if [ ${CLOUD_TYPE} == "public" ]; then
cat >>/opt/run_test.sh <<-EOF
mkdir -p /home/gpadmin/heaptestdata
psql postgres -f '/opt/run_test_public.sql'
PGOPTIONS='-c warehouse=test' psql hashdata_sample_database -c "insert into test_hashdata_sample_database select * from generate_series(1,100);"
echo "cloud.forbidden_databases = 'hashdata_sample_database' " >> /home/gpadmin/data/master/postgresql.conf
echo "cloud.forbidden_tablespaces = 'hashdata_sample_tablespace' " >> /home/gpadmin/data/master/postgresql.conf
pg_ctl -D /home/gpadmin/data/master/ -l "/home/gpadmin/data/master/log/restart.log" -w -t 20 -o "-p 5432 -c gp_role=dispatch" restart
sleep 30
EOF

cat >>/opt/run_test_public.sql <<-EOF
set c4139d65.f116.a4e0.c172.fb3024c97f04 = true;
CREATE TABLESPACE hashdata_sample_tablespace location '/home/gpadmin/heaptestdata' with(storage='cloud');
GRANT ALL ON TABLESPACE hashdata_sample_tablespace TO public;
CREATE DATABASE hashdata_sample_database;
\c hashdata_sample_database;
set c4139d65.f116.a4e0.c172.fb3024c97f04 = true;
create table test_hashdata_sample_database(a int) tablespace hashdata_sample_tablespace;
EOF
fi

cat >>/opt/run_test.sh <<-EOF
mkdir -p /home/gpadmin/heaptestdata
psql postgres -f '/opt/run_test.sql'
make ${MAKE_TEST_COMMAND}
EOF

if [ ${STORAGE_TYPE} == "local" ]; then
cat >>/opt/run_test.sql <<-EOF
set c4139d65.f116.a4e0.c172.fb3024c97f04 = true;
CREATE TABLESPACE regress_hashdata_tablespace_test location '/home/gpadmin/newtestdata' with(storage='cloud');
GRANT ALL ON TABLESPACE regress_hashdata_tablespace_test TO public;
GRANT ALL ON TABLESPACE default_cloud_tablespace TO public;
EOF
elif [ ${STORAGE_TYPE} == "oss" ]; then
cat >>/opt/run_test.sql <<-EOF
set c4139d65.f116.a4e0.c172.fb3024c97f04 = true;
CREATE STORAGE SERVER test_server OPTIONS(protocol 's3av2', endpoint 'localhost:9000', https 'false', virtual_host 'false');
CREATE STORAGE USER MAPPING FOR PUBLIC STORAGE SERVER test_server OPTIONS (accesskey 'admin', secretkey '12345678');
CREATE TABLESPACE regress_oss_test location '/testci/regress' with(storage='cloud') server test_server handler '/usr/local/cloudberry-db-devel/lib/postgresql/dfs_tablespace, remote_file_handler';
GRANT ALL ON TABLESPACE regress_oss_test TO public;
CREATE TABLESPACE regress_hashdata_tablespace_test location '/testci' with(storage='cloud') server test_server handler '/usr/local/cloudberry-db-devel/lib/postgresql/dfs_tablespace, remote_file_handler';
GRANT ALL ON TABLESPACE regress_hashdata_tablespace_test TO public;
GRANT ALL ON TABLESPACE default_cloud_tablespace TO public;
EOF
elif [ ${STORAGE_TYPE} == "hdfs" ]; then
cat >>/opt/run_test.sql <<-EOF
set c4139d65.f116.a4e0.c172.fb3024c97f04 = true;
CREATE STORAGE SERVER test_server OPTIONS(protocol 'hdfs', namenode '127.0.0.1', port '8020'););
CREATE STORAGE USER MAPPING FOR PUBLIC STORAGE SERVER test_server OPTIONS (auth_method 'simple');
CREATE TABLESPACE regress_oss_test location '/testci/regress' with(storage='cloud') server test_server handler '/usr/local/cloudberry-db-devel/lib/postgresql/dfs_tablespace, remote_file_handler';
GRANT ALL ON TABLESPACE regress_oss_test TO public;
CREATE TABLESPACE regress_hashdata_tablespace_test location '/testci' with(storage='cloud') server test_server handler '/usr/local/cloudberry-db-devel/lib/postgresql/dfs_tablespace, remote_file_handler';
GRANT ALL ON TABLESPACE regress_hashdata_tablespace_test TO public;
GRANT ALL ON TABLESPACE default_cloud_tablespace TO public;
EOF
fi
  chmod a+x /opt/run_test.sh
  chmod a+x /opt/run_test.sql
}

function install_cbdb_rpm() {
  rpm -ivh "${public_rpm_download_url}"
  killall cbuiserver || true
  ln -sf /usr/local/cloudberry-db-* /usr/local/cloudberry-db-devel
  chown -R gpadmin:gpadmin /usr/local/cloudberry-db*
}

function run_test() {
  su -l gpadmin -c "bash /opt/run_test.sh"
}

function export_log(){
  mkdir -p ${CI_PROJECT_DIR}/log/master
  mkdir -p ${CI_PROJECT_DIR}/log/qe
  cp /home/gpadmin/data/master/log/* ${CI_PROJECT_DIR}/log/master/
  cp /home/gpadmin/data/primary5433/log/* ${CI_PROJECT_DIR}/log/qe/
  cp /home/gpadmin/data/primary5434/log/* ${CI_PROJECT_DIR}/log/qe/
  cp /home/gpadmin/data/primary5435/log/* ${CI_PROJECT_DIR}/log/qe/
}

function regression_test() {
  if [ "${INIT_FUNCTION+x}" ]; then
    eval "${INIT_FUNCTION}"
  else
    gen_env
  fi

  time run_test

  if [ "${DUMP_DB}" == "true" ]; then
    mkdir sqldump
    chmod 777 sqldump
    su gpadmin -c "${CBDB_RELEASE_SRC_DIRECTORY}"/scripts/dumpdb.bash
  fi
}
