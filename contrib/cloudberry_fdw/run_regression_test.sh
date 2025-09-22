#!/usr/bin/env bash
set -euxo pipefail

export PORT_BASE=${PORT_BASE:-5432}
export WITH_MIRRORS=${WITH_MIRRORS:-true}
export DATADIRS=${DATADIRS:-$(pwd)/../../contrib/cloudberry_fdw/cbdb_test_data/datadirs}

# equal to local cluster segments
echo 'running tests with 3 segments in remote cluster...'
export NUM_PRIMARY_MIRROR_PAIRS=3
make installcheck

# less than local cluster segments
echo 'running tests with 1 segment in remote cluster...'
export NUM_PRIMARY_MIRROR_PAIRS=1
make installcheck

# more than local cluster segments
echo 'running tests with 5 segments in remote cluster...'
export NUM_PRIMARY_MIRROR_PAIRS=5
make installcheck

# cleanup
echo 'tests done, clean remote cluster...'
./cloudberry_clean.bash
