#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/third_party/lib60870/lib60870-C"
BUILD_DIR="${ROOT_DIR}/build/lib60870-host"
INSTALL_DIR="${ROOT_DIR}/build/host"

rm -rf "${BUILD_DIR}"
cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" \
	-DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
	-DBUILD_HAL=ON \
	-DBUILD_COMMON=ON \
	-DBUILD_EXAMPLES=OFF \
	-DBUILD_TESTS=OFF

cmake --build "${BUILD_DIR}" --target lib60870 -j"$(nproc)"
mkdir -p "${INSTALL_DIR}/include/lib60870" "${INSTALL_DIR}/lib"
cp "${BUILD_DIR}/src/liblib60870.a" "${INSTALL_DIR}/lib/"
cp "${SOURCE_DIR}"/src/inc/api/*.h "${INSTALL_DIR}/include/lib60870/"
cp "${SOURCE_DIR}"/src/hal/inc/*.h "${INSTALL_DIR}/include/lib60870/"
