#include <modbus/modbus.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	const char *host = argc > 1 ? argv[1] : "127.0.0.1";
	int port = argc > 2 ? atoi(argv[2]) : 1502;
	modbus_t *context = modbus_new_tcp(host, port);
	uint16_t registers[2] = {0, 0};

	if (context == NULL)
		return 1;
	modbus_set_slave(context, 1);
	if (modbus_connect(context) == -1)
	{
		modbus_free(context);
		return 1;
	}

	if (modbus_read_registers(context, 0, 2, registers) != 2)
		return 1;
	if (registers[0] != 1234 || registers[1] != 5678)
		return 1;
	if (modbus_write_register(context, 2, 4321) == -1)
		return 1;

	printf("Modbus TCP simulation passed: %u %u\n", registers[0], registers[1]);
	modbus_close(context);
	modbus_free(context);
	return 0;
}
