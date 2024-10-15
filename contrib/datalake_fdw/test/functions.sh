function create_ext(){
    db=$1
    psql -d ${db} -c "DROP EXTENSION IF EXISTS datalake_fdw CASCADE"
    psql -d ${db} -c "DROP EXTENSION IF EXISTS hive_connector"
    psql -d ${db} -c "CREATE EXTENSION hive_connector"
    psql -d ${db} -c "CREATE EXTENSION datalake_fdw"
    psql -d ${db} -c "CREATE FOREIGN DATA WRAPPER datalake_fdw HANDLER datalake_fdw_handler VALIDATOR datalake_fdw_validator OPTIONS ( mpp_execute 'all segments' )"
}

function create_schema() {
    db=$1
    schema=$2
    psql -d $db -c "drop schema if exists $schema cascade"
    psql -d $db -c "create schema $schema"
}

function create_server(){
    db=$1
    server=$2
    hdfs_cluster=$3
    psql -d ${db} -c "DROP SERVER IF EXISTS ${server} CASCADE"
    psql -d ${db} -c "SELECT public.create_foreign_server('${server}', 'gpadmin', 'datalake_fdw', '${hdfs_cluster}')"
}

function create_oss_server(){
    db=$1
    server=$2
    psql -d ${db} -c "DROP SERVER IF EXISTS ${server} CASCADE"
    psql -d ${db} -c "CREATE SERVER ${server} FOREIGN DATA WRAPPER datalake_fdw OPTIONS (host 'pek3b.qingstor.com', protocol 'qs', isvirtual 'false', ishttps 'false')"
    psql -d ${db} -c "CREATE USER MAPPING FOR gpadmin SERVER ${server} OPTIONS (user 'gpadmin', accesskey 'KGCPPHVCHRMZMFEAWLLC', secretkey '0SJIWiIATh6jOlmAKr8DGq6hOAGBI1BnsnvgJmTs');"
}

function valgrind_check(){
    command="valgrind --leak-check=full --show-leak-kinds=all ${command}"
}

function get_time_elapse(){
    seconds=$1
    hour=$(( $seconds/3600 ))
    min=$(( ($seconds-${hour}*3600)/60 ))
    sec=$(( $seconds-${hour}*3600-${min}*60 ))
    HMS="${hour}h:${min}m:${sec}s"
}

function psql_exec_print_time(){
    command=$1
    msg=$2
    time_start=$(date +%s)
    eval "$command"
    time_end=$(date +%s)
    get_time_elapse $(( $time_end - $time_start ))
    echo "Time cost to $msg: $HMS"
}

function start_regress_test() {
	cat > /opt/run_regress_test.sh <<-EOF
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
			else
				echo "datalake regress ok"
				exit 0
			fi
		    done
		    exit 1
		}
		source /usr/local/cloudberry-db-devel/greenplum_path.sh
		export PGPORT=7000
		export COORDINATOR_DATA_DIRECTORY=/code/gpdb_src/gpAux/gpdemo/datadirs/qddir/demoDataDir-1
		pushd "/code/gpdb_src/contrib/datalake_fdw/"
		(
			make > /dev/null 2>&1
			make installcheck
		)
		popd
	EOF

	chmod a+x /opt/run_regress_test.sh

	/opt/run_regress_test.sh

	if [ $? -ne 0 ]; then
		echo "datalake regress test failed!"
		exit 1
	fi
}

function start_tpcds_test() {
    BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
    source ${CBDB_INSTALL_DIRECTORY}/greenplum_path.sh
	$BASE_DIR/tpcds/run.sh
}

function load_docker() {
	export OS_ARCH=$(uname -m)
	ARCH="${OS_ARCH}"
	BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
	sudo chown -R gpadmin. /opt
	usermod -aG docker gpadmin
	pushd /opt
	echo "download docker image"
	wget --no-check-certificate -nv -O datalake-services-mysql-${OS_ARCH}.tar.gz https://hashdata-download.obs.cn-north-4.myhuaweicloud.com/docker-image/datalake-services-mysql-${OS_ARCH}.tar.gz
	docker load < ./datalake-services-mysql-${OS_ARCH}.tar.gz

	wget --no-check-certificate -nv -O datalake-services-hive-${OS_ARCH}.tar.gz https://hashdata-download.obs.cn-north-4.myhuaweicloud.com/docker-image/datalake-services-hive-${OS_ARCH}.tar.gz
	docker load < ./datalake-services-hive-${OS_ARCH}.tar.gz

	wget --no-check-certificate -nv -O backup-${OS_ARCH}.sql https://hashdata-download.obs.cn-north-4.myhuaweicloud.com/docker-image/backup-${OS_ARCH}.sql
	mv backup-${OS_ARCH}.sql backup.sql
	popd
	echo "start deploy docker"

	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml up -d
	sleep 30
	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml exec -u root services-hive service sshd start
	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml exec services-hive sh -c "sudo /usr/sbin/sshd"
	docker cp /opt/backup.sql services-mysql:/opt/backup.sql
	docker exec -i services-mysql sh -c "/usr/bin/mysql -u root --password=123456 < /opt/backup.sql"
	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml exec services-hive sh /scripts/start.sh
	sleep 30
}

function load_data_to_docker() {
	BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
	docker cp $BASE_DIR/sql/test_logerror.sql /sql/
	docker exec -it services-hive hive -f /sql/test_logerror.sql
}