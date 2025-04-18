
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

	wget --no-check-certificate -nv -O datalake-services-mysql-s3-${OS_ARCH}.tar.gz https://hashdata-download.obs.cn-north-4.myhuaweicloud.com/docker-image/datalake-services-mysql-s3-${OS_ARCH}.tar.gz
	docker load < ./datalake-services-mysql-s3-${OS_ARCH}.tar.gz

	wget --no-check-certificate -nv -O datalake-services-hive-s3-${OS_ARCH}.tar.gz https://hashdata-download.obs.cn-north-4.myhuaweicloud.com/docker-image/datalake-services-hive-s3-${OS_ARCH}.tar.gz
	docker load < ./datalake-services-hive-s3-${OS_ARCH}.tar.gz
	
	popd
	echo "start deploy docker"

	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml up -d
	docker-compose -f $BASE_DIR/docker/docker-compose-s3-ci.yml up -d
	sleep 30
	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml exec -u root services-hive service sshd start
	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml exec services-hive sh -c "sudo /usr/sbin/sshd"
	docker cp /opt/backup.sql services-mysql:/opt/backup.sql
	docker exec -i services-mysql sh -c "/usr/bin/mysql -u root --password=123456 < /opt/backup.sql"
	docker-compose -f $BASE_DIR/docker/docker-compose-ci.yml exec services-hive sh /scripts/start.sh

	docker-compose -f $BASE_DIR/docker/docker-compose-s3-ci.yml exec -u root services-hive-s3 service sshd start
	docker-compose -f $BASE_DIR/docker/docker-compose-s3-ci.yml exec services-hive-s3 sh -c "sudo /usr/sbin/sshd"
	docker exec -i services-mysql-s3 sh -c "/usr/bin/mysql -u root --password=123456 < /opt/backup.sql"
	docker-compose -f $BASE_DIR/docker/docker-compose-s3-ci.yml exec services-hive-s3 sh /scripts/start-s3.sh
	docker-compose -f $BASE_DIR/docker/docker-compose-s3-ci.yml exec services-hive-s3 sh /scripts/start-hive.sh
	sleep 30
}

function load_data_to_docker() {
	BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
	docker cp $BASE_DIR/sql/load_logerror_data.sql services-hive:/sql/
	docker exec -i services-hive sh -c "hive -f /sql/load_logerror_data.sql"

	docker cp $BASE_DIR/sql/load_hive_specify_partition.sql services-hive:/sql/
	docker exec -i services-hive sh -c "hive -f /sql/load_hive_specify_partition.sql"
}

function load_delimiter_data_to_docker() {
	BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
	docker cp $BASE_DIR/gen/gen_text.py services-hive:/opt/
	docker exec -i services-hive sh -c "python /opt/gen_text.py"
}