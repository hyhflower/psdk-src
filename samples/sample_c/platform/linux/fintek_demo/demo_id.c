#include "fintek_api.h"

#define MAX_DEVICE	4
#define NOT_AVAIL	-1

static int m_device_map[MAX_DEVICE];

int get_device_id(int idx, int *id)
{
	sFintek_sio_data sio_data;
	unsigned int tmp = 0;
	int nRet;

	if (nRet = init_fintek_sio(eSIO_TYPE_F75114, idx, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		return -1;
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);

	CHECK_RET(_EnableGPIO(0x03, eGPIO_Mode_Enable));
	CHECK_RET(_EnableGPIO(0x04, eGPIO_Mode_Enable));

	_SetGpioOutputEnableIdx(0x03, eGPIO_Direction_In);
	_SetGpioOutputEnableIdx(0x04, eGPIO_Direction_In);

	_GetGpioInputDataIdx(0x03, &tmp);
	*id = tmp << 1;

	_GetGpioInputDataIdx(0x04, &tmp);
	*id |= tmp;

	DeactiveSIO(sio_data.ic_port);

	return 0;
}

int list_device()
{
	psFintek_sio_data sio_data, next;
	int nRet;
	int i = 0, id;

	for (i = 0; i < MAX_DEVICE; ++i) {
		m_device_map[i] = NOT_AVAIL;
	}

	nRet = get_device_list(eSIO_TYPE_F75114, &sio_data);

	printf("Total F75114: %d\n", nRet);

	i = 0;
	while (sio_data) {
		if (i < MAX_DEVICE) {
			get_device_id(i, &id);

			m_device_map[i] = id;
			printf("  index: %d, ID: %d\n", i++, id);
			printf("    path_name: %s\n", sio_data->path_name);
			printf("    device_arch: %s\n", sio_data->device_arch);
		}

		next = sio_data->next;
		free(sio_data);
		sio_data = next;
	}
}

void usage(int argc, char *argv[])
{
	fprintf(stderr, "Fintek GPIO-ID Demo API:\n");
	fprintf(stderr, "%s <ID> <value>\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "\tID: ID of F75114\n");
	fprintf(stderr, "\tvalue: 0-3\n");
	fprintf(stderr, "\t  0: GPIO01:0, GPIO00:0\n");
	fprintf(stderr, "\t  1: GPIO01:0, GPIO00:1\n");
	fprintf(stderr, "\t  2: GPIO01:1, GPIO00:0\n");
	fprintf(stderr, "\t  3: GPIO01:1, GPIO00:1\n");
	fprintf(stderr,
		"for examples: %s 3 2 will set ID 3 with GPIO01:1, GPIO00:0\n",
		argv[0]);
}

int gpio_control(int argc, char *argv[])
{
	sFintek_sio_data sio_data;
	int id = strtol(argv[1], NULL, 10);
	int value = strtol(argv[2], NULL, 10);
	int i, nRet;

	if (id >= MAX_DEVICE) {
		fprintf(stderr, "\n\nID: %d is too large\n\n", id);
		usage(argc, argv);
		return 4;
	}

	if (value >= 4) {
		fprintf(stderr, "\n\nvalue: %d is wrong\n\n", id);
		usage(argc, argv);
		return 4;
	}

	for (i = 0; i < MAX_DEVICE; ++i) {
		if (m_device_map[i] == id)
			break;
	}

	if (i == MAX_DEVICE) {
		fprintf(stderr, "Cannot found ID:%d device\n", id);
		return 4;
	}

	printf("id: %d, value: %d\n", id, value);

	if (nRet = init_fintek_sio(eSIO_TYPE_F75114, i, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		return -1;
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);

	_SetGpioOutputEnableIdx(0x00, eGPIO_Direction_Out);
	_SetGpioOutputEnableIdx(0x01, eGPIO_Direction_Out);

	/* Default Open-Drain mode, uncomment following to Push-Pull mode */
	//_SetGpioDriveEnable(0x00 , eGPIO_Drive_Mode_Pushpull);
	//_SetGpioDriveEnable(0x01 , eGPIO_Drive_Mode_Pushpull);

	/* Default Open-Drain mode, disable internal Pull-High resistor */
	_SetGpioPullMode(0x00, eGPIO_Pull_Disable);
	_SetGpioPullMode(0x01, eGPIO_Pull_Disable);

	_SetGpioOutputDataIdx(0x00, (value & 0x01) >> 0);
	_SetGpioOutputDataIdx(0x01, (value & 0x02) >> 1);

	DeactiveSIO(sio_data.ic_port);

}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		usage(argc, argv);
		return 3;
	}

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	/* Create index-to-ID mapping */
	list_device();

	gpio_control(argc, argv);

	return 0;
}
