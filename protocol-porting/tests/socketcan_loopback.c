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
	struct sockaddr_can address;
	struct can_frame sent;
	struct can_frame received;
	int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	int receive_own_messages = 1;
	if (socket_fd < 0) return 1;
	if (setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
		&receive_own_messages, sizeof(receive_own_messages)) < 0) return 1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
	if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) return 1;

	memset(&address, 0, sizeof(address));
	address.can_family = AF_CAN;
	address.can_ifindex = ifr.ifr_ifindex;
	if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) return 1;

	memset(&sent, 0, sizeof(sent));
	sent.can_id = 0x123;
	sent.len = 2;
	sent.data[0] = 0x34;
	sent.data[1] = 0x12;
	if (write(socket_fd, &sent, sizeof(sent)) != sizeof(sent)) return 1;

	if (read(socket_fd, &received, sizeof(received)) != sizeof(received)) return 1;
	if (received.can_id != sent.can_id || received.len != sent.len ||
		received.data[0] != sent.data[0] || received.data[1] != sent.data[1]) return 1;

	close(socket_fd);
	puts("SocketCAN loopback passed");
	return 0;
}
