find_path(LIBMODBUS_INCLUDE_DIR
	NAMES modbus/modbus.h
	HINTS "${LIBMODBUS_ROOT}/include"
)

find_library(LIBMODBUS_LIBRARY
	NAMES modmodbus modbus
	HINTS "${LIBMODBUS_ROOT}/lib"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibModbus
	REQUIRED_VARS LIBMODBUS_INCLUDE_DIR LIBMODBUS_LIBRARY
)

if(LibModbus_FOUND AND NOT TARGET LibModbus::Modbus)
	add_library(LibModbus::Modbus STATIC IMPORTED)
	set_target_properties(LibModbus::Modbus PROPERTIES
		IMPORTED_LOCATION "${LIBMODBUS_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${LIBMODBUS_INCLUDE_DIR}"
	)
endif()

mark_as_advanced(LIBMODBUS_INCLUDE_DIR LIBMODBUS_LIBRARY)
