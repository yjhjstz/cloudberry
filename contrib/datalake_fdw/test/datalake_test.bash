#set -euo pipefail
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
source $BASE_DIR/create_env.sh
source $BASE_DIR/function.sh


function start_test() {
	build_env
	load_data
	start_regress_test
}

start_test
