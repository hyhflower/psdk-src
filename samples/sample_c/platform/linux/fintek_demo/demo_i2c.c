#include "fintek_api.h"

static unsigned int dev_addr = 0xa0;

void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n");
	fprintf(stderr, "I2C single byte Read/Write Demo: \n");
	fprintf(stderr, "%s <r,w,d> <ch> <dev> <reg> <val>\n\n", argv[0]);
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "\tRead reg 0x30: \n\t\t%s r 0 0xa0 0x30 \n", argv[0]);
	fprintf(stderr, "\tWrite reg 0x30, val 0x1f:\n\t\t%s w 0 0xa0 0x30 0x1f\n", argv[0]);
	fprintf(stderr, "\tDump all reg:\n\t\t%s d 0 0xa0\n", argv[0]);
}

int eeprom_dump(void)
{
	int i;
	int status;
	unsigned int data;

	for (i = 0x00; i <= 0xff; ++i) {
		status = ReadI2CData(dev_addr, i, &data);
		if (status) {
			fprintf(stderr, "%s: Read EEPROM Error\n", __func__);
			return -1;
		}

		if (!(i % 16))
			printf("\n");

		printf("%02x ", data);
	}

	printf("\n");

	return 0;
}

int eeprom_demo(int argc, char *argv[])
{
	int i;
	int status;
	unsigned int reg;
	unsigned int data;
	FILE *file;
	int set;

	if (argc < 4) {
		usage(argc, argv);
		return 1;
	}

	set = strtol(argv[2], NULL, 0x16);
	CHECK_RET(_EnableI2C(set));

	status = SelectI2CChannel(set);
	if (status) {
		fprintf(stderr, "%s: SelectI2CChannel Error\n", __func__);
		return status;
	}

	do {
		switch (argc) {
		case 4:
			if (argv[1][0] == 'd') {
				dev_addr = strtol(argv[3], NULL, 16) & 0xff;
				return eeprom_dump();
			}

		case 5:
			if (argv[1][0] == 'r') {
				dev_addr = strtol(argv[3], NULL, 16) & 0xff;
				reg = strtol(argv[4], NULL, 16) & 0xff;

				printf("dev_addr: %x, reg: %x\n", dev_addr, reg);

				status = ReadI2CData(dev_addr, reg, &data);
				if (status) {
					fprintf(stderr, "%s: read failed\n", __func__);
					return status;
				}

				printf("Read reg:0x%02x, value:0x%02x\n", reg & 0xff, data & 0xff);
				return 0;
			}

		case 6:
			if (argv[1][0] == 'w') {
				dev_addr = strtol(argv[3], NULL, 16) & 0xff;
				reg = strtol(argv[4], NULL, 16) & 0xff;
				data = strtol(argv[5], NULL, 16) & 0xff;

				status = WriteI2CData(dev_addr, reg, data);
				if (status) {
					fprintf(stderr, "%s: write failed\n", __func__);
					return status;
				}

				printf("Write reg:0x%02x, value:0x%02x\n", reg & 0xff, data & 0xff);
				return 0;
			}
		}

		usage(argc, argv);
		return -1;
	} while (0);

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;
	sFintek_sio_data sio_data;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F81512, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	nRet = eeprom_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return nRet;
}
