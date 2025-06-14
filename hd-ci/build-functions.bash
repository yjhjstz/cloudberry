#!/usr/bin/env bash
set -euxo pipefail

function finish() {
  if [ $? -ne 0 ]; then
    sleep 21600
  fi
  echo "finish callback: sync filesystem before exit"
  sync
}

function prepare() {
  rm -rf /etc/yum.repos.d/vagrant.repo || true
  rm -rf /etc/yum.repos.d/docker-ce.repo || true

  # 主要缓存了 pljava 和 hive-connector的jar包
  wget -q "${CBDB_CACHE_M2}"
  tar -xzf "$(basename ${CBDB_CACHE_M2})" -C "${HOME}"

  rm -rf "${CBDB_INSTALL_DIRECTORY}"
  # mkdir -p "${CBDB_INSTALL_DIRECTORY}/{bin,lib,include}"
  #rust_cache="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/rust/${OS_ARCH}/rust-cache-${OS_ARCH}-20240319.tar.gz"
  #wget --no-check-certificate -nv "${rust_cache}"
  #tar -xzf "$(basename ${rust_cache})" -C "${HOME}"

  source "$HOME/.cargo/env" || true

  #wget https://artifactory.hashdata.xyz/artifactory/opensource-codes/curl/binaries/curl-7.58.0-el7-x86_64.tar.gz
  #tar -C /usr/ --strip-components=1 -xzvf curl-7.58.0-el7-x86_64.tar.gz
  # 切换 conan 的为 http
  conan remote remove conan-public || true
  conan remote add conan-public http://artifactory.hashdata.xyz/artifactory/api/conan/conan-public
  # coredump文件生成到指定目录
  #sed -i 's|^kernel\.core_pattern = core|kernel.core_pattern = /opt/core|' /etc/sysctl.conf
  #sysctl -p
}

function git_clone_cbdb_src() {
  if [[ ${CI} == "true" ]]; then
    src_tar_gz_filename=$(basename ${CBDB_SRC_DIRECTORY})-${BUILD_ID}.tar.gz

    aws s3 cp \
      "s3://${CBDB_CACHE_CODE_BUCKET}/tmp/${src_tar_gz_filename}" ${BUILD_ROOT}/ \
      --endpoint-url "https://${CBDB_S3_ENDPOINT}" \
      --no-progress --no-verify-ssl

    tar -xzf ${BUILD_ROOT}/${src_tar_gz_filename} -C ${BUILD_ROOT}
  else
    git clone --depth "${CBDB_SRC_DEPTH}" \
#      --branch "${CBDB_SRC_BRANCH}" \
      "${CBDB_SRC_REPO}" "${CBDB_SRC_DIRECTORY}"
  fi

  git -C "${CBDB_SRC_DIRECTORY}" --no-pager log -1

  # 临时解决方法, 这是在解决unionstore编译的问题
  # chmod 0755 "${BUILD_INSTALL_DIRECTORY}"/bin/* ==> chmod 0755 "${BUILD_INSTALL_DIRECTORY}"/bin/* || true
  # sed -i 's@chmod 0755 "${BUILD_INSTALL_DIRECTORY}"/bin/\*@chmod 0755 "${BUILD_INSTALL_DIRECTORY}"/bin/\* || true@g' \
  #   "${CBDB_SRC_DIRECTORY}"/deploy/dependencies.sh

  export CBDB_VERSION=$("${CBDB_SRC_DIRECTORY}"/getversion --short)
  ln -sf "${CBDB_SRC_DIRECTORY}/src" "${CBDB_RELEASE_SRC_DIRECTORY}"
}

function dump_env() {
  rm -f "${CBDB_SRC_DIRECTORY}/build_env.sh"
      cat >~/.cargo/config <<EOF
[source.crates-io]
replace-with = 'aliyun'
[source.aliyun]
registry = "sparse+https://mirrors.aliyun.com/crates.io-index/"
EOF

  for variable in "${!CBDB_@}"; do
    echo "${variable}=\"${!variable}\"" >>"${CBDB_SRC_DIRECTORY}/build_env.sh"
  done
}

function build_jansson() {
  jansson_tar_gz_filename="$(basename ${CBDB_JASNSSON_SRC_URL})"
  rm -rf /opt/jansson/

  pushd "${BUILD_ROOT}"
  wget -q "${CBDB_JASNSSON_SRC_URL}"
  tar -xzf "${jansson_tar_gz_filename}"

  jansson_dir="$(basename ${jansson_tar_gz_filename} .tar.gz)"
  pushd "${jansson_dir}"
  ./configure --prefix=/opt/jansson --disable-static
  make
  make install
  cp /opt/jansson/lib/libjansson.so* /usr/lib
  cp -p /usr/lib/libjansson.so* /lib64
  popd

  popd
}

function install_jansson() {
  # 之前必须先调用过 build_jansson 函数
  cp /opt/jansson/lib/libjansson.so* "${CBDB_INSTALL_DIRECTORY}"/lib/
}

function install_etcd() {
  etcd_tar_gz_filename="$(basename ${CBDB_ETCD_BIN_URL})"

  pushd "${BUILD_ROOT}"
  wget -q "${CBDB_ETCD_BIN_URL}"
  tar -xzf "${etcd_tar_gz_filename}"

  etcd_dir=$(basename ${etcd_tar_gz_filename} .tar.gz)
  cp "${etcd_dir}"/etcd "${CBDB_INSTALL_DIRECTORY}"/bin
  cp "${etcd_dir}"/etcdctl "${CBDB_INSTALL_DIRECTORY}"/bin

  rm -rf "${etcd_dir}" "${etcd_tar_gz_filename}"
  popd
}

function install_jdk() {
  jdk_tar_gz_filename="$(basename ${CBDB_JDK_BIN_URL})"
  pushd "${BUILD_ROOT}"
  wget -q "${CBDB_JDK_BIN_URL}"
  tar -xzf "${jdk_tar_gz_filename}"

  mkdir -p "${CBDB_INSTALL_DIRECTORY}"/ext

  tar -xzf "${jdk_tar_gz_filename}" -C "${CBDB_INSTALL_DIRECTORY}"/ext
  ln -sf "${CBDB_INSTALL_DIRECTORY}"/ext/jdk* "${CBDB_INSTALL_DIRECTORY}"/ext/jdk

  rm -f "${jdk_tar_gz_filename}"
  popd
}

function configure_local_database() {
  (
  local ADDITIONAL_OPTS=""
  CONFIGURE_FLAGS=${CONFIGURE_FLAGS:-""}

  if [ "${CBDB_BUILD_TYPE}" == "debug" ]; then
    export LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib"
    export CPPFLAGS="-I${CBDB_INSTALL_DIRECTORY}/include -Wno-nonnull"
    ADDITIONAL_OPTS='--enable-cassert --enable-tap-tests --enable-depend --enable-debug'
  else
    export CFLAGS="-g -O3"
    export CXXFLAGS="-g -O3"
    export LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib"
    export CPPFLAGS="-I${CBDB_INSTALL_DIRECTORY}/include -Wno-nonnull"
  fi

  CONFIGURE_FLAGS="${CONFIGURE_FLAGS} ${ADDITIONAL_CONFIGURE_FLAGS}"

  if [ "${BUILD_TOOL}" == "clang" ]; then
    export CC="/usr/local/bin/clang-14"
  fi

  pushd "${CBDB_SRC_DIRECTORY}"
  ./configure \
    --prefix="${CBDB_INSTALL_DIRECTORY}" \
    --enable-ic-proxy \
    --enable-debug-extensions \
    --enable-orafce \
    --enable-orca \
    --with-gssapi \
    --with-ldap \
    --with-libxml \
    --with-lz4 \
    --with-openssl \
    --with-pam \
    --with-perl \
    --with-pgport=5432 \
    --with-python PYTHON=python3 \
    --with-pythonsrc-ext \
    --with-ssl=openssl \
    --with-libraries="${CBDB_INSTALL_DIRECTORY}"/lib \
    PYTHON=python3 PKG_CONFIG_PATH="${CBDB_INSTALL_DIRECTORY}/lib/pkgconfig" ${ADDITIONAL_OPTS} ${CONFIGURE_FLAGS}

  popd
  )
}

function local_build_configure_database() {
  (
  local ADDITIONAL_OPTS=""
  CONFIGURE_FLAGS=${CONFIGURE_FLAGS:-""}

  if [ "${CBDB_BUILD_TYPE}" == "debug" ]; then
    export LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib"
    export CPPFLAGS="-I${CBDB_INSTALL_DIRECTORY}/include -Wno-nonnull"
    ADDITIONAL_OPTS='--enable-cassert --enable-tap-tests --enable-depend --enable-debug'
  else
    export CFLAGS="-g -O3"
    export CXXFLAGS="-g -O3"
    export LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib"
    export CPPFLAGS="-I${CBDB_INSTALL_DIRECTORY}/include -Wno-nonnull"
  fi

  CONFIGURE_FLAGS="${CONFIGURE_FLAGS} ${ADDITIONAL_CONFIGURE_FLAGS}"

  if [ "${BUILD_TOOL}" == "clang" ]; then
    export CC="/usr/local/bin/clang-14"
  fi
  export LD_LIBRARY_PATH=/usr/local/xerces-c/lib:$LD_LIBRARY_PATH

  pushd "${CBDB_SRC_DIRECTORY}"
  ./configure \
    --prefix="${CBDB_INSTALL_DIRECTORY}" \
    --enable-ic-proxy \
    --enable-debug-extensions \
    --enable-orafce \
    --enable-orca \
    --with-gssapi \
    --with-ldap \
    --with-libxml \
    --with-lz4 \
    --with-openssl \
    --with-pam \
    --with-perl \
    --with-pgport=5432 \
    --with-python PYTHON=python3 \
    --with-pythonsrc-ext \
    --with-ssl=openssl \
    --with-libraries="${CBDB_INSTALL_DIRECTORY}"/lib \
    PYTHON=python3 PKG_CONFIG_PATH="${CBDB_INSTALL_DIRECTORY}/lib/pkgconfig" ${ADDITIONAL_OPTS} ${CONFIGURE_FLAGS}

  popd
  )
}

function configure_database() {
  (
  local ADDITIONAL_OPTS=""
  CONFIGURE_FLAGS=${CONFIGURE_FLAGS:-""}

  if [ "${CBDB_BUILD_TYPE}" == "debug" ]; then
    export LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib"
    export CPPFLAGS="-I${CBDB_INSTALL_DIRECTORY}/include -I/usr/include/apr-1/ -Wno-nonnull"
    export CFLAGS='-O3 -g3' 
    export CXXFLAGS='-O3 -std=c++14 -g3' 
    ADDITIONAL_OPTS='--enable-cassert --enable-tap-tests --enable-depend --enable-debug --enable-faultinjector'
  else
    export CFLAGS="-O3"
    export CXXFLAGS="-O3"
    export LDFLAGS="-L${CBDB_INSTALL_DIRECTORY}/lib"
    export CPPFLAGS="-I${CBDB_INSTALL_DIRECTORY}/include -I/usr/include/apr-1/ -Wno-nonnull"
    ADDITIONAL_OPTS='--disable-faultinjector'
  fi

  if [[ "${STORAGE_TYPE}" = "remote" || "${STORAGE_TYPE}" = "hdfs" || "${STORAGE_TYPE}" = "oss" ]]; then
    CONFIGURE_FLAGS="${CONFIGURE_FLAGS} --enable-remote"
  fi

  if [ "${CLOUD_TYPE}" = "public" ]; then
	CONFIGURE_FLAGS="${CONFIGURE_FLAGS} --enable-public-cloud"
  fi

  CONFIGURE_FLAGS="${CONFIGURE_FLAGS} ${ADDITIONAL_CONFIGURE_FLAGS}"

  if [ "${BUILD_TOOL}" == "clang" ]; then
    export CC="/usr/local/bin/clang-14"
  fi

  pushd "${CBDB_SRC_DIRECTORY}"
  ./configure \
    --prefix="${CBDB_INSTALL_DIRECTORY}" \
    --enable-pax \
    --enable-serverless \
    --enable-tpserver \
    --enable-unionstore \
    --enable-ic-proxy \
    --enable-debug-extensions \
    --enable-orafce \
    --enable-orca \
    --with-gssapi \
    --with-ldap \
    --with-libxml \
    --with-lz4 \
    --with-openssl \
    --with-pam \
    --with-perl \
    --with-pgport=5432 \
    --with-python PYTHON=python3 \
    --with-pythonsrc-ext \
    --with-internal-version=mycbdb \
    --with-ssl=openssl \
    --with-libraries="${CBDB_INSTALL_DIRECTORY}"/lib \
    PYTHON=python3 PKG_CONFIG_PATH="${CBDB_INSTALL_DIRECTORY}/lib/pkgconfig" ${ADDITIONAL_OPTS} ${CONFIGURE_FLAGS}

  popd
  )
}

function build_database() {
  pushd "${CBDB_SRC_DIRECTORY}"
  (
    git config --global --add safe.directory "${CBDB_SRC_DIRECTORY}"
    git submodule update --init
    make -j$(nproc)
    make install
  )
  popd >/dev/null
}

function build_pgindent() {
	pushd "${CBDB_PG_BSD_INDENT_DIRECTORY}"
	(
		source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
		make -j4 && make install
	)
	popd >/dev/null
}

function pg_indentcheck() {
	pushd "${CBDB_SRC_DIRECTORY}/contrib/storage_am/"
	(
		source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
		flist=$(find ./backend -name "*.c")
		priv_hlist=$(find ./backend -name "*.h")
		hlist=$(find ./include -name "*.h")
		for f in ${flist}
		do
			${CBDB_SRC_DIRECTORY}/src/tools/pgindent/pgindent $f
		done
		for f in ${hlist}
		do
			${CBDB_SRC_DIRECTORY}/src/tools/pgindent/pgindent $f
		done
		for f in ${priv_hlist}
		do
			${CBDB_SRC_DIRECTORY}/src/tools/pgindent/pgindent $f
		done

		git diff "${CBDB_SRC_DIRECTORY}/contrib/storage_am" > ${BUILD_ROOT}/indent.diff
		if [ -s ${BUILD_ROOT}/indent.diff ]
		then
			cat ${BUILD_ROOT}/indent.diff
			echo "require pgindent code"
			exit 1
		fi

		find . -name *.BAK > ${BUILD_ROOT}/bak.diff
		if [ -s ${BUILD_ROOT}/bak.diff ]
		then
			cat ${BUILD_ROOT}/bak.diff
			echo "require pgindent code (pgindent generate bak error)"
			exit 1
		fi

		rm ${BUILD_ROOT}/bak.diff
		rm ${BUILD_ROOT}/indent.diff
	)
	popd >/dev/null
}

function check_file_nameformat() {

	pushd "${CBDB_SRC_DIRECTORY}/contrib/storage_am/"
	(
		pushd backend
		find . -name "*.c" | grep -vE "./[a-zA-Z0-9_-/]+/hd_" >> ${BUILD_ROOT}/fileformat.diff
		find . -name "*.h" | grep -vE "./[a-zA-Z0-9_-/]+/hd_" >> ${BUILD_ROOT}/fileformat.diff
		popd >/dev/null
		pushd include
		find . -name "*.h" | grep -vE "./[a-zA-Z0-9_-/]+/hd_" >> ${BUILD_ROOT}/fileformat.diff
		popd >/dev/null
	)

	if [ -s ${BUILD_ROOT}/fileformat.diff ]
	then
		cat ${BUILD_ROOT}/fileformat.diff
		echo "file name require start with \'hd_\'"
		exit 1
	fi

	rm ${BUILD_ROOT}/fileformat.diff
	popd >/dev/null
}

function generate_build_number() {
	pushd ${CBDB_SRC_DIRECTORY}
	# Only if its git repo, add commit SHA as build number
	# BUILD_NUMBER file is used by getversion file in GPDB to append to version
	if [ -d .git ]; then
		echo "$(git rev-parse HEAD)" >GIT_COMMIT
	fi
	popd
}

function unittest() {
  if [[ "${CBDB_BUILD_TYPE}" == "release" ]]; then
    echo "SKIP unittest for release build type.."
    return 0
  fi

  pushd "${CBDB_SRC_DIRECTORY}"
  (
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh
    make GPROOT=/usr/local -s unittest-check
  )
  popd
}

function collect_dependencies() {
  pushd "${CBDB_INSTALL_DIRECTORY}"

  # borrowed from manylinux2014 https://www.python.org/dev/peps/pep-0599/
  # 移除了 libstdc++.so.6 库文件, 因为kylin10上使用了自己编译的 gcc-10.3.0, 需要将新版的so, 复制到安装目录下的
  whitelist=(
    libgpg-error.so.0
    libgcc_s.so.1
    # libstdc++.so.6
    libm.so.6
    libdl.so.2
    librt.so.1
    libc.so.6
    libutil.so.1
    libpthread.so.0
    libresolv.so.2
    libX11.so.6
    libXext.so.6
    libXrender.so.1
    libICE.so.6
    libSM.so.6
    libGL.so.1
    libgobject-2.0.so.0
    libgthread-2.0.so.0
    libglib-2.0.so.0
    libpython
    libpython3.6m.so.1.0
    libnss3.so
    libsmime3.so
    libssl3.so
    libnssutil3.so
    libnsssysinit.so
  )

  (
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh

    find ./ | xargs file | grep ELF |
      awk '{print substr($1, 1, length($1)-1)}' | (xargs ldd 2>/dev/null || true) |
      awk '{print $3}' |
      grep -e '^/' | grep -v -e "^${CBDB_INSTALL_DIRECTORY}" |
      sort | uniq >/tmp/dependencies.txt
  )

  while read -r lib; do
    if [[ " ${whitelist[@]} " =~ " $(basename ${lib}) " ]]; then
      echo "Skip whitelist library $lib"
    else
      echo "cp -p $lib ${CBDB_INSTALL_DIRECTORY}/lib/"
      cp -p "$lib" "${CBDB_INSTALL_DIRECTORY}"/lib/
    fi
  done </tmp/dependencies.txt

  popd >/dev/null
}

function package_create_rpm() {
  rm -rf ~/rpmbuild/
  mkdir -p ~/rpmbuild/SOURCES/

  pushd "${CBDB_INSTALL_DIRECTORY}"
  mkdir unionstore
  cp -r ${BUILD_ROOT}/unionstore/target/release unionstore/
  cp -r ${BUILD_ROOT}/unionstore/pg_install unionstore/
  tar -czf ~/rpmbuild/SOURCES/bin_cbdb.tar.gz .
  popd >/dev/null

  export CBDB_VERSION=$("${CBDB_SRC_DIRECTORY}"/getversion --short)

  version=$(echo "${CBDB_VERSION}" | tr '-' '_')
  PATH=/usr/bin:$PATH rpmbuild \
    --define="version ${version}" \
    --define="cbdb_version ${CBDB_VERSION}" \
    -bb "${CBDB_RELEASE_SRC_DIRECTORY}"/.cloudberry-db.spec
}

function upload_rpm_package() {
  declare -r rpm_file_name=$(basename "${HOME}/rpmbuild/RPMS/${OS_ARCH}"/cloudberry-db*.rpm)
  upload_rpm_file_name=$(basename "${rpm_file_name}" ".rpm")-"${BUILD_ID}"-"${CBDB_BUILD_TYPE}".rpm

  if [[ "${CBDB_RPM_SUFFIX}" == "external_fts" ]]; then
    upload_rpm_file_name=$(basename "${upload_rpm_file_name}" ".rpm")-"${CBDB_RPM_SUFFIX}".rpm
  fi

  upload_rpm_file_name=$(basename "${upload_rpm_file_name}" ".rpm")-"${STORAGE_TYPE}"-"${CLOUD_TYPE}".rpm
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

function validation() {
  (
    source "${CBDB_INSTALL_DIRECTORY}"/greenplum_path.sh

    postgres --version
    initdb --version
    createdb --version
    psql --version
    gpssh --version
    gpfdist --version
  )
}
