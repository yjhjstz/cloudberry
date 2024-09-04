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

function load_data() {
    BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
	echo "hadoop leave saft mode"
	hadoop dfsadmin -safemode leave
	echo "load orc, parquet and avro data waiting..."
	export HADOOP_HEAPSIZE=2048
    date
    echo "begin load orc waiting..."
	hive -f $BASE_DIR/sql/hive_load_orc_data.sql > $BASE_DIR/hive_load_orc.log 2>&1
    date
    echo "begin load parquet waiting..."
	hive -f $BASE_DIR/sql/hive_load_parquet_data.sql > $BASE_DIR/hive_load_parquet.log 2>&1
	hive -f $BASE_DIR/sql/hive_load_avro_data.sql > $BASE_DIR/hive_load_avro.log 2>&1
	hive -f $BASE_DIR/sql/hive_load_empty_text_data.sql > $BASE_DIR/hive_load_empty_text_data.log 2>&1
	hive -f $BASE_DIR/sql/load_hive_new_text_deflate.sql > $BASE_DIR/load_hive_new_text_deflate.log 2>&1
	hive -f $BASE_DIR/sql/load_hive_new_text_partition.sql > $BASE_DIR/load_hive_new_text_partition.log 2>&1
	hive -f $BASE_DIR/sql/load_hive_text_partition.sql > $BASE_DIR/load_hive_text_partition.log 2>&1
}