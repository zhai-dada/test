cmake_minimum_required(VERSION 3.16)
project(protocol-porting LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LIBMODBUS_ROOT
	"${CMAKE_SOURCE_DIR}/build/target-aarch64"
	CACHE PATH "libmodbus installation prefix")

set(LIB60870_ROOT
	"${CMAKE_SOURCE_DIR}/build/target-aarch64"
	CACHE PATH "lib60870 installation prefix")

option(BUILD_SIMULATION_TESTS "Build native Modbus TCP simulation tests" OFF)
option(BUILD_CAN_TESTS "Build native SocketCAN tests" OFF)
option(BUILD_IEC60870_TESTS "Build native IEC104 tests" OFF)
option(BUILD_RTU_SIMULATION_TESTS "Build native Modbus RTU PTY simulation" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(LibModbus REQUIRED)

add_executable(libmodbus-smoke tests/libmodbus_smoke.c)
target_link_libraries(libmodbus-smoke PRIVATE LibModbus::Modbus)

if(NOT CMAKE_CROSSCOMPILING)
	enable_testing()
	add_test(NAME libmodbus-smoke COMMAND libmodbus-smoke)
endif()

if(BUILD_SIMULATION_TESTS AND NOT CMAKE_CROSSCOMPILING)
	add_executable(modbus-tcp-server tests/modbus_tcp_server.c)
	target_link_libraries(modbus-tcp-server PRIVATE LibModbus::Modbus)

	add_executable(modbus-tcp-client tests/modbus_tcp_client.c)
	target_link_libraries(modbus-tcp-client PRIVATE LibModbus::Modbus)
endif()

if(BUILD_RTU_SIMULATION_TESTS AND NOT CMAKE_CROSSCOMPILING)
	add_executable(modbus-rtu-pty-test tests/modbus_rtu_pty_test.c)
	target_link_libraries(modbus-rtu-pty-test PRIVATE LibModbus::Modbus util)
	add_test(NAME modbus-rtu-pty COMMAND modbus-rtu-pty-test)
endif()

if(BUILD_CAN_TESTS AND NOT CMAKE_CROSSCOMPILING)
	add_executable(socketcan-smoke tests/socketcan_smoke.c)
	add_executable(socketcan-loopback tests/socketcan_loopback.c)
endif()

if(BUILD_IEC60870_TESTS)
	find_package(Lib60870 REQUIRED)
	add_executable(lib60870-smoke tests/lib60870_smoke.c)
	target_link_libraries(lib60870-smoke PRIVATE Lib60870::Lib60870)
	if(NOT CMAKE_CROSSCOMPILING)
		add_executable(iec104-loopback-test tests/iec104_loopback_test.c)
		target_link_libraries(iec104-loopback-test PRIVATE Lib60870::Lib60870)
	endif()
	if(NOT CMAKE_CROSSCOMPILING)
		add_test(NAME lib60870-smoke COMMAND lib60870-smoke)
		add_test(NAME iec104-loopback COMMAND iec104-loopback-test)
	endif()
endif()
