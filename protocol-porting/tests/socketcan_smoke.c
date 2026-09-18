#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
	const char* interface_name = argc > 1 ? argv[1] : "vcan0";
	struct ifreq ifr;
	int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (socket_fd < 0) return 1;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
	if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
		close(socket_fd);
		return 1;
	}
	close(socket_fd);
	printf("SocketCAN interface available: %s\n", interface_name);
	return 0;
}
