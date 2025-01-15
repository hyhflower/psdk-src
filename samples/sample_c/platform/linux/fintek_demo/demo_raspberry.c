#include "fintek_api.h"

void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek Rasperberry Demo API:\n");
	fprintf(stderr, "%s <idx> <direct> <type> <value>\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "\tidx: number of Board idx\n");
	fprintf(stderr, "\tdir: 0: In, 1: Out\n");
	fprintf(stderr, "\ttype: 0: Open Drain, 1: Push-Pull\n");
	fprintf(stderr, "\tvalue: 0/1 when single mode, bitwise when group mode\n");
}

int raspberry_demo(int argc, char *argv[], eSIO_TYPE ic_type)
{
	unsigned int idx, pin;
	unsigned int data;
	eGPIO_Direction dir_mode;
	char *desc;
	int status;
	int ic_mapping[41];

	if (ic_type != eSIO_TYPE_F75115_HID && ic_type != eSIO_TYPE_F75115) {
		printf("Currently support F75115/HID only\n");
		return 3;
	}

	memset(ic_mapping, -1, sizeof(ic_mapping));
	ic_mapping[7] = 0x37;
	ic_mapping[11] = 0x30;
	ic_mapping[12] = 0x31;
	ic_mapping[13] = 0x32;
	ic_mapping[15] = 0x33;
	ic_mapping[16] = 0x34;
	ic_mapping[18] = 0x35;
	ic_mapping[22] = 0x36;
	ic_mapping[29] = 0x05;
	ic_mapping[31] = 0x04;
	ic_mapping[32] = 0x00;
	ic_mapping[33] = 0x03;
	ic_mapping[35] = 0x02;
	ic_mapping[36] = 0x25;
	ic_mapping[37] = 0x01;
	ic_mapping[38] = 0x26;
	ic_mapping[40] = 0x27;

	if (argc < 5) {
		usage(argc, argv);
		return 3;
	}

	pin = strtol(argv[1], NULL, 10);
	if (pin < 1 || pin > 40) {
		printf("pin: 1~40 only\n");
		return 3;
	}

	idx = ic_mapping[pin];
	if (idx == -1) {
		printf("pin: %d cant change to GPIO\n", pin);
		return 3;
	}

	CHECK_RET(_EnableGPIO(idx, eGPIO_Mode_Enable));

	/* Read current setting */
	printf("Read current setting for GPIO%02x (Pin: %d)\n", idx, pin);
	CHECK_RET(_GetGpioOutputEnableIdx(idx, &data));
	printf("Dir: 0x%x => %s\n", data, (data == eGPIO_Direction_Out) ? "OUT" : "IN");

	if (data == eGPIO_Direction_Out) {
		status = _GetGpioDriveEnable(idx, &data);
		if (!status) {
			switch (data) {
			case eGPIO_Drive_Mode_OpenDrain:
				printf("Driving mode: open drain\n");
				break;
			case eGPIO_Drive_Mode_Pushpull:
				printf("Driving mode: push-pull\n");
				break;
			default:
			case eGPIO_Drive_Mode_Invalid:
				printf("Driving mode: invalid\n");
				break;
			}
		} else {
			printf("Not support drive mode setting\n");
		}

		status = _GetGpioOutputDataIdx(idx, &data);
		if (status) {
			printf("Get GPIO%02x output failed\n", idx);
		} else {
			printf("Output Pin: %x Value: 0x%x \n", idx, data);
		}

	} else {
		status = _GetGpioInputDataIdx(idx, &data);
		if (status) {
			printf("Get GPIO%02x input failed\n", idx);
		} else {
			printf("Read Pin: %x Value: 0x%x \n", idx, data);
		}
	}

	/* Setting new setting */
	printf("\nSetting to ....\n");
	printf("pin: %d, idx: 0x%x => set:%x, count:%x\n", pin, idx, (idx >> 4) & 0xf,
	       (idx >> 0) & 0xf);

	// out
	data = strtol(argv[2], NULL, 16);
	CHECK_RET(_SetGpioOutputEnableIdx(idx, data));
	printf("Dir: 0x%x => %s\n", data, (data == eGPIO_Direction_Out) ? "OUT" : "IN");

	if (data == eGPIO_Direction_Out) {

		data = strtol(argv[3], NULL, 16);
		CHECK_RET(_SetGpioDriveEnable(idx, data));
		printf("Driving Mode: 0x%x => %s\n", data, (data == eGPIO_Drive_Mode_OpenDrain)
		       ? "OpenDrain" : "Push-Pull");

		data = strtol(argv[4], NULL, 16);

		/* Single mode */
		status = _SetGpioOutputDataIdx(idx, data);
		if (!status) {
			printf("Write Value: 0x%x \n", data);
		} else {
			/* Group mode */
			status = _SetGpioGroupOutputDataIdx(idx >> 4, data);
			if (!status) {
				printf("Write Group Value: 0x%x \n", data);
			} else {
				printf("Write single/group failed\n");
			}
		}
	} else {
		status = _GetGpioInputDataIdx(idx, &data);
		if (status) {
			printf("This IC not support Single Input mode\n");

			status = _GetGpioGroupInputDataIdx(idx >> 4, &data);
			if (!status) {
				printf("Set: %x data: %x, Pin: %x: data: %x\n",
				       idx >> 4, data, idx, (int)(data & BIT(idx & 0xf)));

				return 0;
			} else {
				printf("This IC not support Group Input mode\n");
			}
		} else {
			printf("Read Pin: %x Value: 0x%x \n", idx, data);
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;
	sFintek_sio_data sio_data;
	eSIO_TYPE ic_type = eSIO_TYPE_F75115_HID;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(ic_type, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	raspberry_demo(argc, argv, ic_type);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
