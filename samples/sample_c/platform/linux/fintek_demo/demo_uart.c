#include "fintek_api.h"
#include <time.h>

static sFintek_sio_data sio_data;

int multi_func_set_uart(int channel)
{
	int idx, r;

	if (sio_data.ic_type == eSIO_TYPE_F75115_HID) {
		for (idx = 0; idx < 6; ++idx) {
			r = MultiFunc_Select(idx, eMultiModeType_UART);
			if (r) {
				printf("%s: MultiFunc_Select failed\n", __func__);
				return -1;
			}
		}
	}

	return 0;
}

static void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek UART Demo API:\n");
	fprintf(stderr, "%s <idx>\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "\tidx: channel of UART\n");
}

int uart_demo(int argc, char *argv[])
{
	uint8_t tx[32], rx[32], read_len;
	uint32_t data;
	int tmp, r, i, idx;

	srand(time(NULL));

	memset(tx, 0, sizeof(tx));
	memset(rx, 0, sizeof(rx));

	if (argc != 2) {
		usage(argc, argv);
		return -1;
	}

	printf("Please connect TX/RX, DTR/DSR, RTS/CTS for loopback test\n");

	idx = strtol(argv[1], NULL, 16);
	if (idx >= UART_GetMaxChannel()) {
		printf("UART idx(%x) larger than max channel(%d)\n", idx, UART_GetMaxChannel());
		return -1;
	}

	r = multi_func_set_uart(idx);
	if (r) {
		printf("%s: multi_func_set_uart failed\n", __func__);
		return -1;
	}

	for (i = 0; i < sizeof(tx); ++i)
		tx[i] = rand() & 0xff;

	r = UART_SetBaudRate(idx, 115200);
	if (r) {
		printf("set baudrate 115200 failed\n");
		return -1;
	}

	printf("set baudrate 115200 ok\n");

	// clear all RX data
	while (1) {
		r = UART_RX(idx, rx, sizeof(rx), &read_len, 1000);
		if (r) {
			printf("RX failed\n");
			return -1;
		}

		if (!read_len)
			break;
	}

	r = UART_TX(idx, tx, sizeof(tx));
	if (r) {
		printf("TX failed\n");
		return -1;
	}

	printf("TX ok\n");

	// delay for all Received
	usleep(10000);

	r = UART_RX(idx, rx, sizeof(rx), &read_len, 1000);
	if (r) {
		printf("RX failed\n");
		return -1;
	}

	printf("RX ok, read_len: %d\n", read_len);

	printf("TX:\n");
	for (i = 0; i < sizeof(tx); ++i)
		printf("%02X ", tx[i]);
	printf("\n");

	printf("RX:\n");
	for (i = 0; i < read_len; ++i)
		printf("%02X ", rx[i]);
	printf("\n");

	// check RTS/CTS disable
	CHECK_RET(UART_SetRTS(idx, 0));
	usleep(100);

	CHECK_RET(UART_GetCTS(idx, &data));
	if (data == 0)
		printf("Set RTS disable, check CTS: %x ok\n", data);
	else
		printf("Set RTS disable, check CTS: %x failed\n", data);

	// check RTS/CTS enable
	CHECK_RET(UART_SetRTS(idx, 1));
	usleep(100);

	CHECK_RET(UART_GetCTS(idx, &data));
	if (data == 1)
		printf("Set RTS enable, check CTS: %x ok\n", data);
	else
		printf("Set RTS enable, check CTS: %x failed\n", data);

	// check DTR/DSR disable
	CHECK_RET(UART_SetDTR(idx, 0));
	usleep(100);

	CHECK_RET(UART_GetDSR(idx, &data));
	if (data == 0)
		printf("Set DTR disable, check DSR: %x ok\n", data);
	else
		printf("Set DTR disable, check DSR: %x failed\n", data);

	// check DTR/DSR enable
	CHECK_RET(UART_SetDTR(idx, 1));
	usleep(100);

	CHECK_RET(UART_GetDSR(idx, &data));
	if (data == 1)
		printf("Set DTR enable, check DSR: %x ok\n", data);
	else
		printf("Set DTR enable, check DSR: %x failed\n", data);

	// view all input pin
	r = UART_GetDCD(idx, &data);
	if (r)
		printf("read DCD, failed\n");
	else
		printf("read DCD: %x, ok\n", data);

	r = UART_GetDSR(idx, &data);
	if (r)
		printf("read DSR, failed\n");
	else
		printf("read DSR: %x, ok\n", data);

	r = UART_GetCTS(idx, &data);
	if (r)
		printf("read CTS, failed\n");
	else
		printf("read CTS: %x, ok\n", data);

	r = UART_GetRI(idx, &data);
	if (r)
		printf("read RI, failed\n");
	else
		printf("read RI: %x, ok\n", data);

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F75115_HID, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	uart_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
