#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <modbus/modbus.h>
#include <pty.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

static uint16_t crc16(const uint8_t* data, size_t length) {
	uint16_t crc = 0xffff;
	for (size_t i = 0; i < length; ++i) {
		crc ^= data[i];
		for (int bit = 0; bit < 8; ++bit) {
			crc = (crc & 1) ? (crc >> 1) ^ 0xa001 : crc >> 1;
		}
	}
	return crc;
}

static int read_frame(int fd, uint8_t* frame, size_t capacity) {
	fd_set read_set;
	int length = 0;

	while (length < 8) {
		struct timeval timeout = {2, 0};
		FD_ZERO(&read_set);
		FD_SET(fd, &read_set);
		if (select(fd + 1, &read_set, NULL, NULL, &timeout) <= 0) return -1;
		ssize_t count = read(fd, frame + length, capacity - (size_t)length);
		if (count <= 0) return -1;
		length += (int)count;
		if (length >= 8 && (frame[1] == 3 || frame[1] == 6)) return 8;
	}
	return length;
}

static int run_slave(int master_fd) {
	uint8_t request[256];
	int length = read_frame(master_fd, request, sizeof(request));
	if (length < 6 || request[0] != 1) {
		fprintf(stderr, "invalid RTU read request length=%d slave=%u\n", length, request[0]);
		for (int i = 0; i < length; ++i) fprintf(stderr, "%02x ", request[i]);
		fputc('\n', stderr);
		return 1;
	}

	if (request[1] == 3) {
		uint8_t response[] = {1, 3, 2, 0x04, 0xd2, 0, 0};
		uint16_t crc = crc16(response, 5);
		response[5] = crc & 0xff;
		response[6] = crc >> 8;
		if (write(master_fd, response, sizeof(response)) != (ssize_t)sizeof(response)) return 1;
		fprintf(stderr, "sent RTU response\n");
	} else {
		fprintf(stderr, "unexpected RTU function=%u\n", request[1]);
		return 1;
	}

	length = read_frame(master_fd, request, sizeof(request));
	if (length < 8 || request[1] != 6) return 1;
	uint8_t response[8];
	memcpy(response, request, 6);
	uint16_t write_crc = crc16(response, 6);
	response[6] = write_crc & 0xff;
	response[7] = write_crc >> 8;
	if (write(master_fd, response, sizeof(response)) != (ssize_t)sizeof(response)) return 1;
	return 0;
}

static int run_client(const char* device) {
	modbus_t* context = modbus_new_rtu(device, 9600, 'N', 8, 1);
	uint16_t value = 0;
	if (context == NULL) { fprintf(stderr, "modbus_new_rtu failed\n"); return 1; }
	modbus_set_slave(context, 1);
	modbus_set_response_timeout(context, 2, 0);
	if (modbus_connect(context) == -1) { fprintf(stderr, "modbus_connect failed: %s\n", modbus_strerror(errno)); return 1; }
	if (modbus_read_registers(context, 0, 1, &value) != 1 || value != 1234) { fprintf(stderr, "read failed: %s value=%u\n", modbus_strerror(errno), value); return 1; }
	if (modbus_write_register(context, 2, 4321) == -1) { fprintf(stderr, "write failed: %s\n", modbus_strerror(errno)); return 1; }
	modbus_close(context);
	modbus_free(context);
	return 0;
}

int main(void) {
	int master_fd;
	int slave_fd;
	char slave_name[128];
	struct termios raw;
	pid_t child;
	int status;

	if (openpty(&master_fd, &slave_fd, slave_name, NULL, NULL) != 0) return 1;
	if (tcgetattr(slave_fd, &raw) != 0) return 1;
	cfmakeraw(&raw);
	if (tcsetattr(slave_fd, TCSANOW, &raw) != 0) return 1;
	child = fork();
	if (child < 0) return 1;
	if (child == 0) {
		close(master_fd);
		close(slave_fd);
		_exit(run_client(slave_name));
	}

	status = run_slave(master_fd);
	waitpid(child, &status, 0);
	close(master_fd);
	close(slave_fd);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 1;
	puts("Modbus RTU PTY simulation passed");
	return 0;
}
