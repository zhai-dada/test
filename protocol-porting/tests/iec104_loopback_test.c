#include "cs104_connection.h"
#include "cs104_slave.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile int client_received = 0;

static bool asdu_handler(void* parameter, int address, CS101_ASDU asdu) {
	(void)parameter;
	(void)address;
	if (CS101_ASDU_getTypeID(asdu) == M_SP_NA_1 && CS101_ASDU_getNumberOfElements(asdu) > 0) {
		SinglePointInformation point = (SinglePointInformation)CS101_ASDU_getElement(asdu, 0);
		if (point && InformationObject_getObjectAddress((InformationObject)point) == 100 &&
			SinglePointInformation_getValue(point)) {
			client_received = 1;
		}
		if (point) SinglePointInformation_destroy(point);
	}
	return true;
}

static bool interrogation_handler(void* parameter, IMasterConnection connection,
	CS101_ASDU asdu, uint8_t qoi) {
	(void)parameter;
	(void)asdu;
	if (qoi == IEC60870_QOI_STATION) {
		CS101_AppLayerParameters params = IMasterConnection_getApplicationLayerParameters(connection);
		CS101_ASDU response = CS101_ASDU_create(params, false, CS101_COT_ACTIVATION_CON,
			0, 1, false, false);
		InformationObject command = (InformationObject)InterrogationCommand_create(NULL, 0, qoi);
		CS101_ASDU_addInformationObject(response, command);
		InformationObject_destroy(command);
		IMasterConnection_sendASDU(connection, response);
		CS101_ASDU_destroy(response);

		response = CS101_ASDU_create(params, false, CS101_COT_INTERROGATED_BY_STATION,
			0, 1, false, false);
		SinglePointInformation point = SinglePointInformation_create(NULL, 100, true,
			IEC60870_QUALITY_GOOD);
		CS101_ASDU_addInformationObject(response, (InformationObject)point);
		InformationObject_destroy((InformationObject)point);
		IMasterConnection_sendASDU(connection, response);
		CS101_ASDU_destroy(response);

		response = CS101_ASDU_create(params, false, CS101_COT_ACTIVATION_TERMINATION,
			0, 1, false, false);
		command = (InformationObject)InterrogationCommand_create(NULL, 0, qoi);
		CS101_ASDU_addInformationObject(response, command);
		InformationObject_destroy(command);
		IMasterConnection_sendASDU(connection, response);
		CS101_ASDU_destroy(response);
	}
	return true;
}

static int run_server(void) {
	CS104_Slave slave = CS104_Slave_create(10, 10);
	if (!slave) return 1;
	CS104_Slave_setLocalAddress(slave, "127.0.0.1");
	CS104_Slave_setInterrogationHandler(slave, interrogation_handler, NULL);
	CS104_Slave_start(slave);
	if (!CS104_Slave_isRunning(slave)) return 1;
	sleep(5);
	CS104_Slave_stop(slave);
	CS104_Slave_destroy(slave);
	return 0;
}

static int run_client(void) {
	CS104_Connection connection = CS104_Connection_create("127.0.0.1", 2404);
	if (!connection) return 1;
	CS104_Connection_setASDUReceivedHandler(connection, asdu_handler, NULL);
	if (!CS104_Connection_connect(connection)) return 1;
	CS104_Connection_sendStartDT(connection);
	Thread_sleep(300);
	CS104_Connection_sendInterrogationCommand(connection, CS101_COT_ACTIVATION, 1,
		IEC60870_QOI_STATION);
	for (int i = 0; i < 20 && !client_received; ++i) Thread_sleep(100);
	CS104_Connection_destroy(connection);
	return client_received ? 0 : 1;
}

int main(void) {
	pid_t child = fork();
	int status;
	if (child < 0) return 1;
	if (child == 0) _exit(run_server());
	Thread_sleep(300);
	status = run_client();
	waitpid(child, NULL, 0);
	if (status != 0) return status;
	puts("IEC104 loopback simulation passed");
	return 0;
}
