# Protocol Porting

This workspace ports `libmodbus` v3.2.0, `lib60870-C` v2.4.1, and Linux
SocketCAN independently from the gateway application. The target libraries
are built as static AArch64 archives with `aarch64-linux-gnu-gcc`. Native
binaries are used only for simulation tests.

## Fixed Source

- Source archive: `third_party/libmodbus-v3.2.0.zip`
- Version: `3.2.0`
- SHA-256: `ffe922a061f35bd65546690b4b8a6edef3f0dc3b733ba7bc94847dfecb534513`
- License: LGPL-2.1-or-later

IEC 60870:

- Source archive: `third_party/lib60870-v2.4.1.zip`
- Version: `2.4.1`
- SHA-256: `1a8a952ebb1ef77402072b96fb251cd007bd6340b78d8aa4fe61711a15c2f327`
- License: GPLv3 or commercial license

CAN:

- Linux SocketCAN kernel API
- No third-party userspace library is required

## Directory Layout

```text
protocol-porting/
├── cmake/                         # CMake find modules and toolchains
├── scripts/                       # reproducible build and test scripts
├── tests/                         # native simulation and smoke tests
├── third_party/libmodbus/         # fixed upstream source tree
├── third_party/lib60870/          # fixed upstream source tree
├── build/host/                    # native static installation
└── build/target-aarch64/          # AArch64 static installation
```

Generated files stay under `build/`; the upstream source tree is not used as
an in-place build directory.

## Native Build

The source uses Autotools. The native static build was verified with:

```bash
cd third_party/libmodbus
bash ./autogen.sh
./configure \
  --prefix="$PWD/../../build/host" \
  --disable-tests \
  --disable-shared \
  --enable-static
make -j$(nproc)
make install
```

The installed files are:

```text
build/host/include/modbus/*.h
build/host/lib/libmodbus.a
```

## AArch64 Static Build

The cross compiler is expected to be available in `PATH`:

```bash
command -v aarch64-linux-gnu-gcc
command -v aarch64-linux-gnu-ar
command -v aarch64-linux-gnu-ranlib
```

Build the target archive:

```bash
./scripts/build-libmodbus-aarch64.sh
```

The script uses:

```text
aarch64-linux-gnu-gcc
aarch64-linux-gnu-ar
aarch64-linux-gnu-ranlib
```

It disables shared libraries and tests and installs only the static target
library into `build/target-aarch64`.

Verify the target architecture:

```bash
file build/target-aarch64/lib/libmodbus.a
cmake -S . -B build/aarch64-cmake \
  -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-aarch64-linux-gnu.cmake \
  -DLIBMODBUS_ROOT="$PWD/build/target-aarch64"
cmake --build build/aarch64-cmake -j$(nproc)
file build/aarch64-cmake/libmodbus-smoke
```

The resulting executable should report `ARM aarch64` with `file`.

## CMake Smoke Test

```bash
cmake -S . -B build/cmake
cmake --build build/cmake -j$(nproc)
ctest --test-dir build/cmake --output-on-failure
```

## Modbus TCP Simulation

The native simulation uses libmodbus's server API and does not require a real
Modbus device:

```bash
./scripts/build-native-simulation.sh
./build/native-simulation/modbus-tcp-server 1502 &
SERVER_PID=$!
./build/native-simulation/modbus-tcp-client 127.0.0.1 1502
kill "$SERVER_PID"
```

The simulation server exposes registers 0 and 1 with values `1234` and
`5678`. The client reads both registers and writes register 2.

## IEC 60870-5-104 Static Build

Build the latest GitHub release with TLS disabled and static library output:

```bash
./scripts/build-lib60870-aarch64.sh
```

The target output is:

```text
build/target-aarch64/lib/liblib60870.a
build/target-aarch64/include/lib60870/
```

The AArch64 smoke executable can be built with:

```bash
cmake -S . -B build/aarch64-iec-cmake \
  -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-aarch64-linux-gnu.cmake \
  -DLIB60870_ROOT="$PWD/build/target-aarch64" \
  -DBUILD_IEC60870_TESTS=ON
cmake --build build/aarch64-iec-cmake --target lib60870-smoke
```

TLS is intentionally disabled for this first port because lib60870 requires
an additional mbedTLS source tree for TLS support.

## SocketCAN

SocketCAN is provided by the Linux kernel. The project includes native
compile tests for the raw CAN API:

```bash
cmake -S . -B build/native-can-test \
  -DBUILD_CAN_TESTS=ON
cmake --build build/native-can-test \
  --target socketcan-smoke socketcan-loopback
```

To run the loopback test, the host needs a `vcan0` interface and permission
to configure it:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
./build/native-can-test/socketcan-smoke vcan0
./build/native-can-test/socketcan-loopback vcan0
sudo ip link del vcan0
```

The current environment has the SocketCAN headers and `ip` command, but does
not allow creation of `vcan0` without elevated privileges, so the runtime
loopback test could not be executed here.

The SocketCAN programs still compile successfully on the host. Their runtime
test requires a kernel with SocketCAN and a configured `vcan0` or physical
CAN interface.

RTU testing requires either a real serial adapter or a virtual serial-port
pair. `pyserial` is available in the current environment, but no virtual
TTY-pair utility is installed, so RTU simulation is not run automatically.

## Basic Test Results

- libmodbus v3.2.0 static smoke test: passed
- Modbus TCP simulated server/client read-write test: passed
- Modbus RTU PTY virtual-serial read/write test: passed
- lib60870-C v2.4.1 static smoke test: passed
- IEC104 local server/client loopback test: passed
- AArch64 lib60870 smoke executable: built and identified as ARM64
- SocketCAN compile smoke tests: passed
- SocketCAN runtime loopback: waiting for `vcan0` permissions/interface

## Cross Compilation

Use the same configure flow with the target toolchain and sysroot:

```bash
./configure \
  --host=aarch64-linux-gnu \
  --build=x86_64-pc-linux-gnu \
  --prefix=/usr \
  --disable-tests \
  --disable-shared \
  --enable-static \
  CC=aarch64-linux-gnu-gcc \
  AR=aarch64-linux-gnu-ar \
  RANLIB=aarch64-linux-gnu-ranlib
```

Replace the compiler prefix and sysroot paths with the BM1688 SDK values.
The first target-side validation should be a TCP client test, followed by an
RTU test against the actual serial device.
