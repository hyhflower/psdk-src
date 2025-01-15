#include "fintek_api.h"

void usage(char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek I2C Protocol Demo API:\n");
	fprintf(stderr, "\t%s <channel>\n\n", argv[0]);
}

int i2c_demo(int argc, char *argv[])
{
	unsigned char data;
	int i = 0, j, status, err_cnt = 0, stop_cnt, err_flag, r = 0;
	int set;

	if (argc != 2) {
		usage(argv);
		return 1;
	}

	set = strtol(argv[1], NULL, 0x16);
	CHECK_RET(_EnableI2C(set));

	// Write EEPROM with 0~FF data
	for (i = 0; i < 16; ++i) {
		r |= I2C_Start(set);
		r |= I2C_Write_Data(set, 0xa0);
		r |= I2C_Write_Data(set, i * 16);

		for (j = 0; j < 16; ++j) {
			r |= I2C_Write_Data(set, i * 16 + j);
		}

		r |= I2C_Stop(set);

		if (r) {
			printf("%s: write error: %d\n", __func__, r);
			return r;
		}

		usleep(1500);	// eeprom spec, delay 1~1.5ms per write transaction
	}

	// read EEPROM all data
	for (i = 0; i < 16; ++i) {
		for (j = 0; j < 16; ++j) {
			r |= I2C_Start(set);
			r |= I2C_Write_Data(set, 0xa0);
			r |= I2C_Write_Data(set, i * 16 + j);
			r |= I2C_Start(set);
			r |= I2C_Write_Data(set, 0xa1);
			r |= I2C_Read_Data(set, &data, 1);
			r |= I2C_Stop(set);

			if (r) {
				printf("%s: read error: %d\n", __func__, r);
				return r;
			}

			printf("%02x ", data);
		}
		printf("\n");
	}

	printf("\n");

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;
	sFintek_sio_data sio_data;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F75115_HID, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	i2c_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
