#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/third_party/libmodbus"
BUILD_DIR="${ROOT_DIR}/build/libmodbus-aarch64"
INSTALL_DIR="${ROOT_DIR}/build/target-aarch64"
SOURCE_COPY="${BUILD_DIR}/source"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cp -a "${SOURCE_DIR}" "${SOURCE_COPY}"
if [[ -f "${SOURCE_COPY}/Makefile" ]]; then
	make -C "${SOURCE_COPY}" distclean >/dev/null 2>&1 || true
fi

cd "${BUILD_DIR}"
"${SOURCE_COPY}/configure" \
	--build=x86_64-pc-linux-gnu \
	--host=aarch64-linux-gnu \
	--prefix="${INSTALL_DIR}" \
	--disable-tests \
	--disable-shared \
	--enable-static \
	CC=aarch64-linux-gnu-gcc \
	AR=aarch64-linux-gnu-ar \
	RANLIB=aarch64-linux-gnu-ranlib

make -j"$(nproc)"
make install

file "${INSTALL_DIR}/lib/libmodbus.a"
