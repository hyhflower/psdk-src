#include "fintek_api.h"

void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek Caseopen Demo API:\n");
	fprintf(stderr, "%s <clear>\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "<clear>: option value, any value will clear caseopen status\n");
}

void demo_caseopen(int argc, char *argv[])
{
	int st, status = 0;

	status = GetCaseOpenStatus(&st);
	if (status) {
		printf("get GetCaseOpenStatus failed\n");
		return;
	}

	printf("Case open state: %d\n", st);

	if (argc == 1)
		return;

	printf("Clear open state\n");
	status = ClearCaseOpenStatus();
	if (status) {
		printf("get ClearCaseOpenStatus failed\n");
		return;
	}
}

int main(int argc, char *argv[])
{
	int nRet = 0;
	sFintek_sio_data sio_data;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F81866, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	demo_caseopen(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
