#include "fintek_api.h"

void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n");
	fprintf(stderr, "WDT Demo: Please reference function set_wdt_enable/get_wdt_enable/is_wdt_timeout\n\n");
}

int watchdog_demo(int argc, char *argv[])
{
	int status = 0, timer = 0;

	printf("Disable WDT & waiting for 5s\n");
	SetWdtDisable();
	sleep(5);

	printf("Start WDT with 20s \n");
	// countdown 10s, other parameter using original value
	SetWdtConfiguration(20, -1, -1, 0, -1, -1);
	SetWdtEnable();

	if (!GetWdtTimeoutReadable())
		printf("This IC couldnt support read WDT count function, Please wait ...\n");

	while (!GetWdtTimeoutStatus(&status, &timer)) {
		static int old_timer = 0;

		if (old_timer != timer) {
			fprintf(stderr, "status:%d, timer:%d\n", status, timer);
			old_timer = timer;
		}

		if (status) {
			fprintf(stderr, "status:%d, timer:%d\n", status, timer);
			break;
		}

		usleep(10000);
	}

	return 0;
}

int watchdog2_demo(int argc, char *argv[])
{
	int status = 0, timer = 0;
	int idx = 1;

	printf("Disable WDT & waiting for 5s\n");
	SetWdtIdxDisable(idx);
	sleep(5);

	printf("Start WDT with 20s \n");
	// countdown 10s, other parameter using original value
	SetWdtIdxConfiguration(idx, 20, -1, -1, 0, -1, -1);
	SetWdtIdxEnable(idx);

	if (!GetWdtTimeoutReadable())
		printf("This IC couldnt support read WDT count function, Please wait ...\n");

	while (!GetWdtIdxTimeoutStatus(idx, &status, &timer)) {
		static int old_timer = 0;

		if (old_timer != timer) {
			fprintf(stderr, "status:%d, timer:%d\n", status, timer);
			old_timer = timer;
		}

		if (status) {
			fprintf(stderr, "status:%d, timer:%d\n", status, timer);
			break;
		}

		usleep(10000);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;
	sFintek_sio_data sio_data;
	set_debug(1);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F81512, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);

	/*
	   watchdog_demo is designed to demo 1st wdt. If we need to use other set,
	   Please using watchdog2_demo() and set the target wdt index
	   (1st idx = 0)
	 */

	watchdog_demo(argc, argv);

	DeactiveSIO(sio_data.ic_port);

	return 0;
}
