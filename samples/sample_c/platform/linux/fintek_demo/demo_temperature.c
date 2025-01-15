#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include "fintek_api.h"

sFintek_sio_data sio_data;

void usage(char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek Temperature Demo API:\n");

	fprintf(stderr, "Get Temperature:\n");
	fprintf(stderr, "\t%s <idx/spec>\n\n", argv[0]);
	fprintf(stderr, "\t   idx: index of temperature (0~max)\n");
	fprintf(stderr, "\t   spec: special target, e.g. PECI\n");
}

int temperature_demo(int argc, char *argv[])
{
	int temperature;
	int idx;
	int status = 0;

	if (argc <= 1) {
		usage(argv);
		return -1;
	}

	if (strncmp(argv[1], "PECI", sizeof("PECI")) == 0 ||
	    strncmp(argv[1], "peci", sizeof("peci")) == 0) {
		status = GetTemperature_PECI(&temperature);
		if (status < 0) {
			printf("GetTemperature_PECI Error\n");
			return -1;
		}

		printf("\ttemperature(PECI) : %d\n", temperature);
	} else {
		idx = strtol(argv[1], NULL, 10);
		if (idx == 0) {
			if (strlen(argv[1]) != 1 || argv[1][0] != '0') {
				printf("index not number: %s\n", argv[1]);

				return -1;
			}
		}

		if (idx >= GetTemperatureCount()) {
			printf("idx(%d) >= Max Temperature index(%d) error\n",
			       idx, GetTemperatureCount());
			return -1;
		}

		status = GetTemperature(idx, &temperature);
		if (status < 0)
			printf("temperature index Error\n");

		printf("\ttemperature(%d) : %d\n", idx, temperature);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F81966, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	temperature_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
