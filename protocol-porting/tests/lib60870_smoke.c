#include <cs104_connection.h>
#include <stdio.h>

int main(void) {
	CS104_Connection connection = CS104_Connection_create("127.0.0.1", 2404);
	if (connection == NULL) return 1;
	CS104_Connection_destroy(connection);
	puts("lib60870 v2.4.1 smoke test passed");
	return 0;
}
