#include <modbus/modbus.h>
#include <stdio.h>

int main(void) {
	modbus_t* tcp = modbus_new_tcp("127.0.0.1", 502);
	if (tcp == NULL) {
		return 1;
	}

	modbus_set_slave(tcp, 1);
	modbus_free(tcp);

	modbus_t* rtu = modbus_new_rtu("/dev/null", 9600, 'N', 8, 1);
	if (rtu == NULL) {
		return 1;
	}
	modbus_free(rtu);

	puts("libmodbus v3.2.0 smoke test passed");
	return 0;
}
