#set -euo pipefail
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source $BASE_DIR/create_env.sh
source $BASE_DIR/functions.sh


function start_test() {
	build_env
	load_docker
	load_data_to_docker
	load_delimiter_data_to_docker
	start_regress_test
}

start_test
