#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include "fintek_api.h"

//#define FINTEK_GPIO_INT_TRIGGER_PATH  "/sys/bus/platform/devices/fintek-gpio-int/gpio"
#define FINTEK_GPIO_INT_TRIGGER_PATH	"/sys/bus/platform/devices/fintek-gpio-int/irq-%d"
#define FORCE_GROUP_MODE		0

#define SECOND (1000000LL)
#define MSECOND (1000LL)

static sFintek_sio_data sio_data;
static unsigned int idx = 0;
static unsigned int data = 0;

static void time_start(struct timeval *tv)
{
	gettimeofday(tv, NULL);
}

static time_t time_delta(struct timeval *start, struct timeval *stop)
{
	time_t secs, usecs;

	secs = stop->tv_sec - start->tv_sec;
	usecs = stop->tv_usec - start->tv_usec;
	if (usecs < 0) {
		secs--;
		usecs += SECOND;
	}

	return secs * SECOND + usecs;
}

static time_t time_stop(struct timeval *begin)
{
	struct timeval end;
	gettimeofday(&end, NULL);
	return time_delta(begin, &end);
}

static void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek GPIO Demo API:\n");
	fprintf(stderr, "%s <idx> <direct> <type> <value> <pull_mode>\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "\tidx: number of GPIO, like GPIO17, idx=0x17\n");
	fprintf(stderr, "\tdir: 0: In, 1: Out\n");
	fprintf(stderr, "\ttype: 0: Open Drain, 1: Push-Pull\n");
	fprintf(stderr, "\tvalue: 0/1 when single mode, bitwise when group mode\n");
	fprintf(stderr, "\tpull_mode (option): 0: Pull_Low, 1: Pull_High, 2: Pull_Disable,\n");
}

static int input_verify(int idx)
{
	unsigned int data = 0;
	int status;
	eGPIO_Pull_Mode pull_mode;
	eGPIO_Drive_Mode drive_mode;
	eGPIO_Direction dir_mode;
	char *name;

	CHECK_RET(_SetGpioOutputEnableIdx(idx, eGPIO_Direction_In));

	status = _GetGpioOutputEnableIdx(idx, &dir_mode);
	switch (dir_mode) {
	case eGPIO_Direction_Out:
		name = "eGPIO_Direction_Out";
		break;
	case eGPIO_Direction_In:
		name = "eGPIO_Direction_In";
		break;
	case eGPIO_Direction_Invalid:
		name = "eGPIO_Direction_Invalid";
		break;
	}
	printf("%s: status: %d, dir_mode: %s\n", __func__, status, name);

	status = _GetGpioInputDataIdx(idx, &data);
	printf("%s: status: %d, input_data: %d\n", __func__, status, data);
}

static int output_verify(int idx)
{
	unsigned int data = 0;
	int status;
	eGPIO_Pull_Mode pull_mode;
	eGPIO_Drive_Mode drive_mode;
	eGPIO_Direction dir_mode;
	char *name;

	CHECK_RET(_SetGpioOutputEnableIdx(idx, eGPIO_Direction_Out));

	CHECK_RET(_SetGpioDriveEnable(idx, eGPIO_Drive_Mode_OpenDrain));
	CHECK_RET(_SetGpioPullMode(idx, eGPIO_Pull_Disable));
	CHECK_RET(_SetGpioOutputDataIdx(idx, 1));

	status = _GetGpioPullMode(idx, &pull_mode);
	switch (pull_mode) {
	case eGPIO_Pull_Low:
		name = "eGPIO_Pull_Low";
		break;
	case eGPIO_Pull_High:
		name = "eGPIO_Pull_High";
		break;
	case eGPIO_Pull_Disable:
		name = "eGPIO_Pull_Disable";
		break;
	case eGPIO_Pull_Invalid:
		name = "eGPIO_Pull_Invalid";
		break;
	}
	printf("%s: status: %d, pull_mode: %s\n", __func__, status, name);

	status = _GetGpioDriveEnable(idx, &drive_mode);
	switch (drive_mode) {
	case eGPIO_Drive_Mode_OpenDrain:
		name = "eGPIO_Drive_Mode_OpenDrain";
		break;
	case eGPIO_Drive_Mode_Pushpull:
		name = "eGPIO_Drive_Mode_Pushpull";
		break;
	case eGPIO_Drive_Mode_Invalid:
		name = "eGPIO_Drive_Mode_Invalid";
		break;
	}
	printf("%s: status: %d, drive_mode: %s\n", __func__, status, name);

	status = _GetGpioOutputEnableIdx(idx, &dir_mode);
	switch (dir_mode) {
	case eGPIO_Direction_Out:
		name = "eGPIO_Direction_Out";
		break;
	case eGPIO_Direction_In:
		name = "eGPIO_Direction_In";
		break;
	case eGPIO_Direction_Invalid:
		name = "eGPIO_Direction_Invalid";
		break;
	}
	printf("%s: status: %d, dir_mode: %s\n", __func__, status, name);

	status = _GetGpioOutputDataIdx(idx, &data);
	printf("%s: status: %d, output_data: %d\n", __func__, status, data);
}

static int set_gpio_mode(int idx, eGPIO_Direction dir, eGPIO_Drive_Mode mode, eGPIO_Pull_Mode pull)
{
	int status = 0;

	status |= _EnableGPIO(idx, eGPIO_Mode_Enable);

	if (pull != eGPIO_Pull_Invalid)
		status |= _SetGpioPullMode(idx, pull);

	status |= _SetGpioOutputEnableIdx(idx, dir);

	if (dir == eGPIO_Direction_Out)
		status |= _SetGpioDriveEnable(idx, mode);

	return status;
}

static int gpio_performance_demo(int argc, char *argv[])
{
	unsigned int idx = 0;
	unsigned int data = 0;
	unsigned int pull_mode;
	eGPIO_Direction dir_mode;
	char *desc;
	int status;
	int i;
	int test_cnt = 10000;
	struct timeval start;
	time_t elapse;

	if (argc < 5) {
		usage(argc, argv);
		exit(3);
	}

	idx = strtol(argv[1], NULL, 16);

	printf("Perform gpio speed test, please wait ... cycle: %d\n", test_cnt);

	/* single output measure */
	status = 0;
	status |= _EnableGPIO(idx, eGPIO_Mode_Enable);
	status |= _SetGpioOutputEnableIdx(idx, eGPIO_Direction_Out);
	_SetGpioPullMode(idx, eGPIO_Pull_High);

	time_start(&start);
	for (i = 0; i < test_cnt; ++i)
		status |= _SetGpioOutputDataIdx(idx, i & 1);
	elapse = time_stop(&start);

	printf("single output measure %s\n", status ? "failed" : "sucess");
	printf("Single output (low to high): %lldus\n", (long long)elapse / test_cnt);

	/* single input measure */
	status = 0;
	status |= _SetGpioOutputEnableIdx(idx, eGPIO_Direction_In);

	time_start(&start);
	for (i = 0; i < test_cnt; ++i)
		status = _GetGpioInputDataIdx(idx, &data);

	elapse = time_stop(&start);

	printf("single input measure %s\n", status ? "failed" : "sucess");
	printf("Single input: %lldus\n", (long long)elapse / test_cnt);

	/* group output measure */
	status = 0;
	data = 0;
	for (i = 0; i < 8; ++i)
		_SetGpioOutputEnableIdx((idx & 0xF0) | i, eGPIO_Direction_Out);

	time_start(&start);
	for (i = 0; i < test_cnt; ++i) {
		status |= _SetGpioGroupOutputDataIdx((idx & 0xF0) >> 4, data);

		if (data)
			data = 0;
		else
			data = 0xFF;
	}

	elapse = time_stop(&start);

	printf("Group output measure %s\n", status ? "failed" : "sucess");
	printf("Group output (low to high): %lldus\n", (long long)elapse / test_cnt);

	/* group input measure */
	status = 0;
	data = 0;
	for (i = 0; i < 8; ++i)
		_SetGpioOutputEnableIdx((idx & 0xF0) | i, eGPIO_Direction_In);

	time_start(&start);
	for (i = 0; i < test_cnt; ++i)
		status |= _GetGpioGroupInputDataIdx((idx & 0xF0) >> 4, &data);

	elapse = time_stop(&start);

	printf("Group input measure %s\n", status ? "failed" : "sucess");
	printf("Group input: %lldus\n", (long long)elapse / test_cnt);

	return 0;
}

static int gpio_demo(int argc, char *argv[])
{
	unsigned int pull_mode;
	eGPIO_Direction dir_mode;
	eGPIO_Drive_Mode drive_mode;
	eMultiMode_Type mode = eMultiModeType_INVALID;
	char *desc;
	int status;
	int i;

	if (argc < 5 && argc != 2) {
		usage(argc, argv);
		exit(3);
	}

	idx = strtol(argv[1], NULL, 16);
	CHECK_RET(_EnableGPIO(idx, eGPIO_Mode_Enable));

	/* Read current setting */
	printf("Read current setting for GPIO%02x\n", idx);

	CHECK_RET(_GetGpioOutputEnableIdx(idx, &data));
	printf("Dir: 0x%x => %s\n", data, (data == eGPIO_Direction_Out) ? "OUT" : "IN");

	status = _GetGpioPullMode(idx, &pull_mode);
	if (!status) {
		switch (pull_mode) {
		case eGPIO_Pull_Low:
			printf("Pull mode: pull low\n");
			break;
		case eGPIO_Pull_High:
			printf("Pull mode: pull high\n");
			break;
		case eGPIO_Pull_Disable:
			printf("Pull mode: pull disable\n");
			break;
		default:
		case eGPIO_Pull_Invalid:
			printf("Pull mode: invalid\n");
			break;
		}

	} else {
		printf("Not support pull mode setting\n");
	}

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

	if (argc == 2) {
		printf("view gpio current mode only\n");
		return 0;
	}

	/* Setting new setting */
	if (argc >= 6) {
		pull_mode = strtol(argv[5], NULL, 10);
		if (pull_mode >= eGPIO_Pull_Invalid) {
			printf("pull_mode >= eGPIO_Pull_Invalid failed\n");
			return 0;
		}
	} else {
		/* no pull mode when not setting */
		pull_mode = -1;
	}

	printf("\nSetting to ....\n");

	dir_mode = strtol(argv[2], NULL, 16);
	drive_mode = strtol(argv[3], NULL, 16);

#if FORCE_GROUP_MODE
	for (i = 0; i < 8; ++i) {
		set_gpio_mode((idx & 0xF0) | i, dir_mode, drive_mode,
			      (pull_mode == -1) ? eGPIO_Pull_Invalid : pull_mode);
		printf("set idx: 0x%x, dir: %s, drive_mode: %s\n",
		       (idx & 0xF0) | i, dir_mode == eGPIO_Direction_In ? "In" : "Out",
		       drive_mode == eGPIO_Drive_Mode_OpenDrain ? "OpenDrain" : "PushPull");
	}

#else
	printf("idx: 0x%x => set:%x, count:%x\n", idx, (idx >> 4) & 0xf, (idx >> 0) & 0xf);

	// out
	CHECK_RET(_SetGpioOutputEnableIdx(idx, dir_mode));
	printf("Dir: 0x%x => %s\n", dir_mode, (dir_mode == eGPIO_Direction_Out) ? "OUT" : "IN");

	if (pull_mode != -1 && (pull_mode < eGPIO_Pull_Invalid)) {
		CHECK_RET(_SetGpioPullMode(idx, pull_mode));
		switch (pull_mode) {
		case eGPIO_Pull_Low:
			desc = "eGPIO_Pull_Low";
			break;
		case eGPIO_Pull_High:
			desc = "eGPIO_Pull_High";
			break;
		case eGPIO_Pull_Disable:
			desc = "eGPIO_Pull_Disable";
			break;
		}

		printf("Pull Mode: %s\n", desc);
	}
#endif

	if (dir_mode == eGPIO_Direction_Out) {
		CHECK_RET(_SetGpioDriveEnable(idx, drive_mode));
		printf("Driving Mode: 0x%x => %s\n", drive_mode, (drive_mode == eGPIO_Drive_Mode_OpenDrain)
		       ? "OpenDrain" : "Push-Pull");

		data = strtol(argv[4], NULL, 16);

#if FORCE_GROUP_MODE
		/* Group mode */
		status = _SetGpioGroupOutputDataIdx(idx >> 4, data);
		if (!status) {
			printf("Write Group Value: 0x%x \n", data);
		} else {
			printf("Write single/group failed\n");
		}
#else
		/* Single mode */
		status = _SetGpioOutputDataIdx(idx, data);
		if (!status) {
			printf("Write Value: 0x%x \n", data);
			//syslog(LOG_NOTICE, "fintek sdk start: pin: %02X set to val: %d\n", idx, data);
		} else {
			printf("Write single failed\n");

			/* Group mode */
			status = _SetGpioGroupOutputDataIdx(idx >> 4, data);
			if (!status) {
				printf("Write Group Value: 0x%x \n", data);
			} else {
				printf("Write group failed\n");
			}
		}
#endif
	} else {
#if FORCE_GROUP_MODE
		status = _GetGpioGroupInputDataIdx(idx >> 4, &data);
		if (!status) {
			printf("Group input: GPIO%xX: %02X\n", idx >> 4, data);

			return 0;
		} else {
			printf("This IC not support Group Input mode\n");
		}
#else
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
#endif
	}

	return 0;
}

static int gpio_int_demo(int argc, char *argv[])
{
	unsigned char buf[512];
	int status;
	int fd;
	char ch;
	int cnt = 0;
	int size;
	int irq = 5;
	int i;

	if (argc < 3) {
		printf("Usage: %s <pin> <irq>\n", argv[0]);
		exit(3);
	}

	idx = strtol(argv[1], NULL, 16);
	irq = strtol(argv[2], NULL, 16);

	CHECK_RET(_EnableGPIO(idx, eGPIO_Mode_Enable));
	printf("idx: 0x%x => set:%x, count:%x, irq: %x\n", idx, (idx >> 4) & 0xf, (idx >> 0) & 0xf, irq);

	CHECK_RET(_SetGpioOutputEnableIdx(idx, eGPIO_Direction_In));
	printf("Dir: 0x%x => %s\n", data, "IN");

	// pull high if available (not error report)
	_SetGpioPullMode(idx, eGPIO_Pull_High);

	for (i = 0; i < 8; ++i)
		_DisableGpioInt(idx >> 4, BIT(i));

	CHECK_RET(_ClearGpioIntStatus(idx >> 4, BIT(idx & 0x0f)));
	CHECK_RET(_EnableGpioInt(idx >> 4, BIT(idx & 0x0f), irq));

	sprintf(buf, FINTEK_GPIO_INT_TRIGGER_PATH, irq);
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		printf("open failed\n");
		return 0;
	}

	while (1) {
		size = read(fd, &ch, 1);
		if (size < 0) {
			perror("read");
			break;
		}

		size = lseek(fd, SEEK_SET, 0);
		if (size < 0) {
			perror("lseek");
			break;
		}

		while (1) {
			CHECK_RET(_GetGpioIntStatus(idx >> 4, &data));
			if ((data & BIT(idx & 0x0f)) == 0)
				break;

			CHECK_RET(_GetGpioInputDataIdx(idx, &data));
			CHECK_RET(_ClearGpioIntStatus(idx >> 4, BIT(idx & 0x0f)));
			printf("cnt: %d, data: %d\n", cnt++, data);
		}
	}

	close(fd);

	return 0;
}

static void signal_handler(int signo)
{
	unsigned char buf[128] = { 0 };
	char *desc;
	int status;
	int i;

	if (signo != SIGHUP)
		return;

	//sprintf(buf, "touch /home/code/demo_gpio-%d; sync", signo);
	//system(buf);

	syslog(LOG_NOTICE, "fintek sdk receive shutdown: pin: %02X set to val: %d\n", idx, !data);

	DeactiveSIO(sio_data.ic_port);
	ActiveSIO(sio_data.ic_port, sio_data.key);

	_EnableGPIO(idx, eGPIO_Mode_Enable);
	//_SetGpioOutputEnableIdx(idx, eGPIO_Direction_Out);
	_SetGpioOutputDataIdx(idx, !data);

	DeactiveSIO(sio_data.ic_port);

	exit(0);
}

static int gpio_demo_daemon(int argc, char *argv[])
{
	int status;
	pid_t pid;
	int i;

	/* create new process */
	pid = fork();
	if (pid == -1)
		return -1;
	else if (pid != 0)
		exit(EXIT_SUCCESS);

	/* create new session and process group */
	if (setsid() == -1)
		return -1;

	/* set the working directory to the root directory */
	if (chdir("/") == -1)
		return -1;

	for (i = 0; i < 1024; ++i)
		close(i);

	/* redirect fd's 0,1,2 to /dev/null */
	open("/dev/null", O_RDWR);	/* stdin */
	dup(0);			/* stdout */
	dup(0);			/* stderr */

	for (i = 1; i < SIGRTMIN; ++i) {
		if (signal(i, signal_handler) == SIG_ERR)
			printf("signal: %d reg failed\n", i);
	}

	while (1)
		sleep(999);

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;

	/* Please set this to 1 enable more debug message */
	set_debug(1);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

#if 0
	// force mode, direct control device
	printf("force mode, ignore index, using /dev/hidraw3\n");

	strcpy(sio_data.path_name, "/dev/hidraw3"); // must be f75114

	if (nRet = init_fintek_sio_force(eSIO_TYPE_F75114, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

#else
	if (nRet = init_fintek_sio(eSIO_TYPE_F81966, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}
#endif

	ActiveSIO(sio_data.ic_port, sio_data.key);

	gpio_demo(argc, argv);	/* GPIO control, AP only */
	//gpio_int_demo(argc, argv);    /* GPIO interrupt test, need SIO driver */
	//gpio_performance_demo(argc, argv);    /* GPIO performance test */

	DeactiveSIO(sio_data.ic_port);

	//gpio_demo_daemon(argc, argv);

	return 0;
}
