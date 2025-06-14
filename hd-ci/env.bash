#!/usr/bin/env bash
CWD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# region 01.系统配置
export BUILD_ROOT="/builds/code"
export BUILD_ID=${CI_PIPELINE_ID:-"20160324"}

export OS_ARCH=$(uname -m)
export OS_TYPE=${OS_TYPE:-"unknown"}
export DUMP_DB=${DUMP_DB:-"false"}

# 发布的所有内容都会上传到该 bucket 中
export CBDB_RELEASE_BUCKET="hashdata-cloud"
export CBDB_S3_ENDPOINT="obs.cn-north-4.myhuaweicloud.com"
# endregion

# region 02.核心项目配置
export CBDB_BUILD_TYPE=${CBDB_BUILD_TYPE:-"debug"}

export CBDB_SRC_REPO_NAME="hashdata-cloud"
export CBDB_SRC_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/${CBDB_SRC_REPO_NAME}.git"

export CBDB_SRC_DIRECTORY="${BUILD_ROOT}/cbdb_src"
export CBDB_SRC_BRANCH="${CBDB_SRC_BRANCH:-"main"}"
export CBDB_SRC_DEPTH="${CBDB_SRC_DEPTH:-"5"}"

export CBDB_PG_BSD_INDENT_DIRECTORY="${CBDB_SRC_DIRECTORY}/contrib/pg_bsd_indent"

export CBDB_RELEASE_SRC_DIRECTORY="${GIT_CLONE_PATH:-"/code/cbdb_release"}"

export CBDB_INSTALL_DIRECTORY="${CBDB_INSTALL_DIRECTORY:-"/usr/local/cloudberry-db-devel"}"
export CBDB_RPM_SUFFIX="internal"
# endregion

# region 03. 组件启用开关配置
export CBDB_BUILD_ANON=${CBDB_BUILD_ANON:-"on"}
export CBDB_BUILD_CBDB_UI=${CBDB_BUILD_CBDB_UI:-"on"}
export CBDB_BUILD_GPBACKUP=${CBDB_BUILD_GPBACKUP:-"on"}
export CBDB_BUILD_GP_EXTTABLE_DELIMITER=${CBDB_BUILD_GP_EXTTABLE_DELIMITER:-"on"}
export CBDB_BUILD_KAFKA_FDW=${CBDB_BUILD_KAFKA_FDW:-"on"}
export CBDB_BUILD_PGVECTOR=${CBDB_BUILD_PGVECTOR:-"on"}
export CBDB_BUILD_PG_POOL=${CBDB_BUILD_PG_POOL:-"on"}
export CBDB_BUILD_PLJAVA=${CBDB_BUILD_PLJAVA:-"on"}
export CBDB_BUILD_PLR=${CBDB_BUILD_PLR:-"on"}
export CBDB_BUILD_POSTGIS=${CBDB_BUILD_POSTGIS:-"on"}
# endregion

# region 04. 组件和扩展的代码仓库配置
export CBDB_JASNSSON_VERSION="2.13.1"
export CBDB_JASNSSON_SRC_URL="https://cbdb-repository-2.obs.cn-north-4.myhuaweicloud.com/misc/jansson-${CBDB_JASNSSON_VERSION}.tar.gz"

export CBDB_ETCD_VERSION="v3.3.25"
export CBDB_ETCD_BIN_URL="https://cbdb-repository-2.obs.cn-north-4.myhuaweicloud.com/misc/etcd-${CBDB_ETCD_VERSION}-linux-${OS_ARCH}.tar.gz"

export CBDB_JDK_VERSION="8u291"
export CBDB_JDK_BIN_URL="https://cbdb-repository-2.obs.cn-north-4.myhuaweicloud.com/misc/jdk-${CBDB_JDK_VERSION}-linux-${OS_ARCH}.tar.gz"

export CBDB_PGPOOL_VERSION="4.4.3"
export CBDB_PGPOOL_SRC_URL="https://cbdb-repository-2.obs.cn-north-4.myhuaweicloud.com/misc/pgpool-II-${CBDB_PGPOOL_VERSION}.tar.gz"

# extensions-functions.bash
export CBDB_ANON_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/postgresql-anonymizer.git"
export CBDB_ANON_BRANCH=${CBDB_ANON_BRANCH:-"hashdata-cloud"}

export CBDB_GPBACKUP_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/gpbackup.git"
export CBDB_GPBACKUP_BRANCH=${CBDB_GPBACKUP_BRANCH:-"main"}

export CBDB_GP_COMMON_GO_LIBS_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/gp-common-go-libs.git"
export CBDB_GP_COMMON_GO_LIBS_BRANCH=${CBDB_GP_COMMON_GO_LIBS_BRANCH:-"main"}

export CBDB_GP_EXTTABLE_DELIMITER_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/gp_exttable_delimiter.git"
export CBDB_GP_EXTTABLE_DELIMITER_BRANCH=${CBDB_GP_EXTTABLE_DELIMITER_BRANCH:-"main"}

export CBDB_KAFKA_FDW_REPO="https://code.hashdata.xyz/cloudberry/kafka_fdw.git"
export CBDB_KAFKA_FDW_BRANCH=${CBDB_KAFKA_FDW_BRANCH:-"master"}

export CBDB_PGVECTOR_REPO="https://code.hashdata.xyz/cloudberry/pgvector.git"
export CBDB_PGVECTOR_BRANCH=${CBDB_PGVECTOR_BRANCH:-"master"}

export CBDB_PLJAVA_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/pljava.git"
export CBDB_PLJAVA_BRANCH=${CBDB_PLJAVA_BRANCH:-"cbdb_1X"}

export CBDB_PLR_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/plr.git"
export CBDB_PLR_BRANCH=${CBDB_PLR_BRANCH:-"master"}

export CBDB_POSTGIS_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/postgis-2.5.4.git"
export CBDB_POSTGIS_BRANCH=${CBDB_POSTGIS_BRANCH:-"master"}

export CBDB_UI_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/cloudberryui.git"
export CBDB_UI_BRANCH=${CBDB_UI_BRANCH:-"rc/1.0"}

export UNIONSTORE_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/unionstore.git"
export UNIONSTORE_BRANCH=${UNIONSTORE_BRANCH:-"serverless"}

export PAX_STORAGE_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/pax_storage.git"
export PAX_STORAGE_BRANCH=${PAX_STORAGE_BRANCH:-"feature/support_storage_am"}

export HASHCOPY_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/hashcopy.git"
export HASHCOPY_BRANCH=${HASHCOPY_BRANCH:-"master"}

# components-functions.bash
export CBDB_MADLIB_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/madlib.git"
export CBDB_MADLIB_BRANCH=${CBDB_MADLIB_BRANCH:-"master"}

export CBDB_PXF_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/pxf.git"
export CBDB_PXF_BRANCH=${CBDB_PXF_BRANCH:-"cbdb_1X"}

export CBDB_PGX_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/pgx.git"
export CBDB_PGX_BRANCH=${CBDB_PGX_BRANCH:-"master"}

export CBDB_ZOMBODB_REPO="https://${GIT_USERNAME}:${GIT_PASSWORD}@code.hashdata.xyz/cloudberry/zombodb.git"
export CBDB_ZOMBODB_BRANCH=${CBDB_ZOMBODB_BRANCH:-"master"}
# endregion

# region 05. 缓存配置
export CBDB_CACHE_CODE_BUCKET="hashdata-releng-private"

export CBDB_CACHE_M2="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/m2_all.tar.gz"
export CBDB_CACHE_GRADLE="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/pxf/pxf_gradle.tar.gz"

# PLR 相关依赖
export CBDB_R_VERSION="3.3.3"
export CBDB_R_BIN_URL="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/plr/R-${CBDB_R_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz"

# POSTGIS 相关依赖
export CBDB_GDAL_VERSION="2.2.1"
export CBDB_GDAL_BIN_URL="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/postgis/gdal-${CBDB_GDAL_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz"
export CBDB_GEOS_VERSION="3.7.0"
export CBDB_GEOS_BIN_URL="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/postgis/geos-${CBDB_GEOS_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz"
export CBDB_SFCGAL_VERSION="1.3.6"
export CBDB_SFCGAL_BIN_URL="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/postgis/sfcgal-${CBDB_SFCGAL_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz"
export CBDB_CGAL_VERSION="4.13"
export CBDB_CGAL_BIN_URL="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/postgis/cgal-${CBDB_CGAL_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz"
export CBDB_JSON_C_VERSION="0.12"
export CBDB_JSON_C_BIN_URL="https://hashdata-releng-2.obs.cn-north-4.myhuaweicloud.com/cache/msic/postgis/json-c-${CBDB_JSON_C_VERSION}-${OS_TYPE}-${OS_ARCH}.tar.gz"
# endregion

# region 06. 杂项设置
export PYTHONWARNINGS="ignore:Unverified HTTPS request"
export PATH="${PATH}:/usr/local/bin/"
# endregion