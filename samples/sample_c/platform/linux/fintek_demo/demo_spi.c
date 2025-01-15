#include "fintek_api.h"

static sFintek_sio_data sio_data;

void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek SPI Flash Demo API:\n");
	fprintf(stderr, "%s <op> <ch> <addr> <data>\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "\top: r read / w write\n");
	fprintf(stderr, "\taddr: \n");
	fprintf(stderr, "\tdata: String\n");
}

int spi_flash_read_status(int set, unsigned char *flash_status)
{
	int i, status;

	status = SpiSetCs(set, 1);
	if (status)
		return status;

	status = SpiSetCs(set, 0);
	if (status)
		goto err;

	status = SpiWriteData(set, 0x05);
	if (status)
		goto err;

	status = SpiReadData(set, flash_status);
	if (status)
		goto err;

	//printf("flash_status: %x\n", flash_status);

	status = SpiSetCs(set, 1);
	if (status)
		goto err;

	return 0;

 err:
	SpiSetCs(set, 1);
	return -1;
}

int spi_flash_set_write_enable(int set, unsigned int en)
{
	int status;

	status = SpiSetCs(set, 1);
	if (status)
		return status;

	status = SpiSetCs(set, 0);
	if (status)
		goto err;

	status = SpiWriteData(set, en ? 0x06 : 0x04);
	if (status)
		goto err;

	status = SpiSetCs(set, 1);
	if (status)
		goto err;

	return 0;

 err:
	SpiSetCs(0, 1);
	return -1;
}

int spi_flash_sector_erase(int set, unsigned int addr)
{
	int status;

	status = SpiSetCs(set, 1);
	if (status)
		return status;

	/* sector erase */

	status = SpiSetCs(set, 0);
	if (status)
		goto err;

	status = SpiWriteData(set, 0x20);	// erase
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 16) & 0xff);	// addr1
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 8) & 0xff);	// addr2
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 0) & 0xff);	// addr3
	if (status)
		goto err;

	status = SpiSetCs(set, 1);
	if (status)
		goto err;

	return 0;

 err:
	SpiSetCs(set, 1);
	return -1;
}

int spi_flash_read_data(int set, unsigned int addr, unsigned char *data, int size)
{
	int i, status;

	if (size > 256)
		return 0;

	status = SpiSetCs(set, 1);
	if (status)
		return status;

	status = SpiSetCs(set, 0);
	if (status)
		goto err;

	status = SpiWriteData(set, 0x03);	// read
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 16) & 0xff);	// addr1
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 8) & 0xff);	// addr2
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 0) & 0xff);	// addr3
	if (status)
		goto err;

	for (i = 0; i < size; ++i) {
		status = SpiReadData(set, &data[i]);
		if (status)
			goto err;
	}

	status = SpiSetCs(set, 1);
	if (status)
		goto err;

	return 0;

 err:
	SpiSetCs(set, 1);
	return -1;
}

int spi_flash_write_data(int set, unsigned int addr, unsigned char *data, int size)
{
	int i, status;

	if (size > 256)
		return 0;

	status = SpiSetCs(set, 1);
	if (status)
		return status;

	status = SpiSetCs(set, 0);
	if (status)
		goto err;

	status = SpiWriteData(set, 0x02);	// write
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 16) & 0xff);	// addr1
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 8) & 0xff);	// addr2
	if (status)
		goto err;

	status = SpiWriteData(set, (addr >> 0) & 0xff);	// addr3
	if (status)
		goto err;

	for (i = 0; i < size; ++i) {
		status = SpiWriteData(set, data[i]);
		if (status)
			goto err;
	}

	status = SpiSetCs(set, 1);
	if (status)
		goto err;

	return 0;

 err:
	SpiSetCs(set, 1);
	return -1;
}

int spi_flash_is_idle(int set, int retry)
{
	int status;
	unsigned char flash_status;

	while (retry--) {
		status = spi_flash_read_status(set, &flash_status);
		if (status)
			return status;

		if (!(flash_status & 0x01))
			return 0;
	}

	return -1;
}

int spi_flash_demo(int argc, char *argv[])
{
	unsigned int i, j, is_read, addr;
	int status;
	unsigned char buf[256];
	int row = 16, channel;
	int idle_cnt = 300;

	if (argc < 4) {
		usage(argc, argv);
		return 1;
	}

	if (argv[1][0] == 'r')
		is_read = 1;
	else if (argv[1][0] == 'w')
		is_read = 0;
	else {
		usage(argc, argv);
		return 1;
	}

	channel = strtol(argv[2], NULL, 16);
	addr = strtol(argv[3], NULL, 16);

	printf("channel: %x, addr: %x\n", channel, addr);

	_EnableSPI(channel);

	status = SpiSetPolPha(channel, 0, 0);
	if (status) {
		printf("spi_flash_read_data failed\n");
		return status;
	}

	if (is_read) {
		if (argc < 4) {
			usage(argc, argv);
			exit(3);
		}

		status = spi_flash_read_data(channel, addr, buf, sizeof(buf));
		if (status) {
			printf("spi_flash_read_data failed\n");
			return status;
		}

		for (i = 0; i < sizeof(buf) / row; ++i) {
			for (j = 0; j < row; ++j)
				printf("%02X ", buf[i * row + j]);

			printf("\n");
		}

	} else {
		if (argc < 5) {
			usage(argc, argv);
			exit(3);
		}

		/* Write */
		status = spi_flash_set_write_enable(channel, 1);
		if (status) {
			printf("spi_flash_set_write_enable failed\n");
			return status;
		}

		status = spi_flash_sector_erase(channel, addr);
		if (status) {
			printf("spi_flash_sector_erase failed\n");
			return status;
		}

		status = spi_flash_is_idle(channel, idle_cnt);
		if (status) {
			printf("spi_flash_is_idle failed\n");
			return status;
		}

		status = spi_flash_set_write_enable(channel, 1);
		if (status) {
			printf("spi_flash_set_write_enable failed\n");
			return status;
		}

		printf("Write %s, len: %zu\n", argv[4], strlen(argv[4]));
		status = spi_flash_write_data(channel, addr, argv[4], strlen(argv[4]));
		if (status) {
			printf("spi_flash_read_data failed\n");
			return status;
		}

		status = spi_flash_is_idle(channel, idle_cnt);
		if (status) {
			printf("spi_flash_is_idle failed\n");
			return status;
		}

		printf("Write complete\n");
	}

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
	spi_flash_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
