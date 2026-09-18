#include <errno.h>
#include <modbus/modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
	int port = argc > 1 ? atoi(argv[1]) : 1502;
	modbus_t* context = modbus_new_tcp("127.0.0.1", port);
	modbus_mapping_t* mapping;
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	int server_socket;
	int client_socket;

	if (context == NULL) return 1;
	mapping = modbus_mapping_new(16, 16, 32, 32);
	if (mapping == NULL) {
		modbus_free(context);
		return 1;
	}

	mapping->tab_registers[0] = 1234;
	mapping->tab_registers[1] = 5678;
	server_socket = modbus_tcp_listen(context, 1);
	if (server_socket == -1) return 1;
	if (modbus_tcp_accept(context, &server_socket) == -1) return 1;
	client_socket = modbus_get_socket(context);

	while (1) {
		int length = modbus_receive(context, query);
		if (length > 0) {
			if (modbus_reply(context, query, length, mapping) == -1) break;
		} else if (length == -1) {
			if (errno == EINTR) continue;
			break;
		}
	}

	if (client_socket >= 0) close(client_socket);
	modbus_mapping_free(mapping);
	modbus_close(context);
	modbus_free(context);
	return 0;
}
