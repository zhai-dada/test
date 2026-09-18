#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIR="${ROOT_DIR}/third_party/libmodbus"
BUILD_DIR="${ROOT_DIR}/build/libmodbus-host"
INSTALL_DIR="${ROOT_DIR}/build/host"
SOURCE_COPY="${BUILD_DIR}/source"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cp -a "${SOURCE_DIR}" "${SOURCE_COPY}"
if [[ -f "${SOURCE_COPY}/Makefile" ]]; then
	make -C "${SOURCE_COPY}" distclean >/dev/null 2>&1 || true
fi

cd "${BUILD_DIR}"
"${SOURCE_COPY}/configure" \
	--prefix="${INSTALL_DIR}" \
	--disable-tests \
	--disable-shared \
	--enable-static

make -j"$(nproc)"
make install
