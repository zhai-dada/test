#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

usage() {
	cat <<EOF
Usage: $0 <command>

Commands:
  host    Build native static libraries and native test programs
  target  Build AArch64 static libraries and target smoke programs
  test    Run native Modbus TCP, Modbus RTU, and IEC104 simulations
  all     Build host and target, then run native simulations
  clean   Remove generated build output
EOF
}

build_host() {
	"${ROOT_DIR}/scripts/build-libmodbus-host.sh"
	"${ROOT_DIR}/scripts/build-lib60870-host.sh"
	rm -rf "${BUILD_DIR}/host-tests"
	cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}/host-tests" \
		-DLIBMODBUS_ROOT="${BUILD_DIR}/host" \
		-DLIB60870_ROOT="${BUILD_DIR}/host" \
		-DBUILD_SIMULATION_TESTS=ON \
		-DBUILD_RTU_SIMULATION_TESTS=ON \
		-DBUILD_IEC60870_TESTS=ON \
		-DBUILD_CAN_TESTS=ON \
		-DCMAKE_BUILD_TYPE=Debug
	cmake --build "${BUILD_DIR}/host-tests" -j"$(nproc)"
}

build_target() {
	"${ROOT_DIR}/scripts/build-libmodbus-aarch64.sh"
	"${ROOT_DIR}/scripts/build-lib60870-aarch64.sh"
	rm -rf "${BUILD_DIR}/target-tests"
	cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}/target-tests" \
		-DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/cmake/Toolchain-aarch64-linux-gnu.cmake" \
		-DLIBMODBUS_ROOT="${BUILD_DIR}/target-aarch64" \
		-DLIB60870_ROOT="${BUILD_DIR}/target-aarch64" \
		-DBUILD_IEC60870_TESTS=ON \
		-DBUILD_SIMULATION_TESTS=OFF \
		-DBUILD_RTU_SIMULATION_TESTS=OFF \
		-DBUILD_CAN_TESTS=OFF \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build "${BUILD_DIR}/target-tests" --target libmodbus-smoke lib60870-smoke -j"$(nproc)"
}

run_tests() {
	if [[ ! -x "${BUILD_DIR}/host-tests/modbus-tcp-client" ]]; then
		build_host
	fi
	ctest --test-dir "${BUILD_DIR}/host-tests" --output-on-failure
	"${BUILD_DIR}/host-tests/modbus-tcp-server" 1502 >"${BUILD_DIR}/host-tests/modbus-server.log" 2>&1 &
	local server_pid=$!
	trap 'kill "${server_pid}" 2>/dev/null || true' RETURN
	sleep 1
	"${BUILD_DIR}/host-tests/modbus-tcp-client" 127.0.0.1 1502
	"${BUILD_DIR}/host-tests/modbus-rtu-pty-test"
	"${BUILD_DIR}/host-tests/iec104-loopback-test"
}

command="${1:-}"
case "${command}" in
	host)
		build_host
		;;
	target)
		build_target
		;;
	test)
		run_tests
		;;
	all)
		build_host
		build_target
		run_tests
		;;
	clean)
		rm -rf "${BUILD_DIR}"
		;;
	*)
		usage
		exit 1
		;;
esac
