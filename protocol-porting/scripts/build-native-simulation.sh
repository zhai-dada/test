#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/native-simulation"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
	-DLIBMODBUS_ROOT="${ROOT_DIR}/build/host" \
	-DBUILD_SIMULATION_TESTS=ON \
	-DBUILD_CAN_TESTS=ON \
	-DBUILD_IEC60870_TESTS=OFF \
	-DCMAKE_BUILD_TYPE=Debug
cmake --build "${BUILD_DIR}" -j"$(nproc)"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

"${BUILD_DIR}/modbus-tcp-server" 1502 >"${BUILD_DIR}/modbus-server.log" 2>&1 &
server_pid=$!
trap 'kill "${server_pid}" 2>/dev/null || true' EXIT
sleep 1
"${BUILD_DIR}/modbus-tcp-client" 127.0.0.1 1502
