#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include "fintek_api.h"

sFintek_sio_data sio_data;

void usage(char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek Fan Demo API:\n");

	fprintf(stderr, "Set Fan:\n");
	fprintf(stderr, "\t%s <mode> <Read_Max_Fan_Count> <idx> <set value/mode> \n\n", argv[0]);
	fprintf(stderr, "\tMode select:\n");
	fprintf(stderr, "\t   0:get RPM value, 1:get Raw value, 2:set Raw value, 3:get mode, 4:set mode\n");
	fprintf(stderr, "\tRead_Max_Fan_Count: 0: No, 1: Yes\n");
	fprintf(stderr, "\tSet Mode:\n");
	fprintf(stderr, "\t     0 : Auto RPM Mode\n");
	fprintf(stderr, "\t     1 : Auto Duty Mode\n");
	fprintf(stderr, "\t     2 : Manual RPM Mode\n");
	fprintf(stderr, "\t     3 : Manual Duty Mode\n");
}

int fan_demo(int argc, char *argv[]){
	int pwm_count = -1;
	unsigned int val, fan_mode;
	int idx = -1;
	int q = 0, status = 0;

	if (argc <= 1) {
		usage(argv);
		return -1;
	}
	idx = strtol(argv[q+3], NULL, 10);
	if (idx >= GetPWMCount()) {
		printf("idx(%d) >= GetPWMCount()(%d)\n", idx, GetPWMCount());
		return -1;
	}

	if(0 != strtol(argv[q+2], NULL, 10)){
		pwm_count = GetPWMCount();
		printf("\tmax fan count: %d\n", pwm_count);
	}

	switch (strtol(argv[q+1], NULL, 10))
	{
	case 0:
		GetFanPWMSioRPMConfig(idx, &val);
		printf("\tfan%d RPM value: %d\n", idx, val);
		break;
	case 1:
		GetPWMSioRawConfig(idx, &val);
		printf("\tfan%d Raw value: %d\n", idx, val);
		break;
	case 2:
		val = strtol(argv[q+4], NULL, 10);
		status = SetPWMSioRawConfig(idx, val);
		if(status)
			printf("\tSet fan%d Raw Config Error!\n",idx);
		else
			printf("\tSet fan%d Raw %u\n", idx, val);
		break;
	case 3:
		status = GetFanMode(idx, &fan_mode);
		if (!status) {
			switch (fan_mode) {
				case ePWM_Fan_Auto_Fan_RPM_Mode:
					printf("\tFan mode: Auto Fan RPM Mode\n");
					break;
				case ePWM_Fan_Auto_Fan_Duty_Mode:
					printf("\tFan mode: Auto Fan Duty Mode\n");
					break;
				case ePWM_Fan_Manual_RPM_Mode:
					printf("\tFan mode: Manual RPM Mode\n");
					break;
				case ePWM_Fan_Manual_Duty_Mode:
					printf("\tFan mode: Manual Duty Mode\n");
					break;
				default:
					printf("\tGet Fan mode Error!");
					return -1;
			}
		} else {
			printf("\tget fan mode error\n");
		}

		break;
	case 4:
		fan_mode = strtol(argv[q+4], NULL, 10);
		switch (fan_mode) {
			case 0:
				status = SetFanMode(idx, ePWM_Fan_Auto_Fan_RPM_Mode);
				printf("\tSet Fan mode : Auto Fan RPM Mode\n");
				break;
			case 1:
				status = SetFanMode(idx, ePWM_Fan_Auto_Fan_Duty_Mode);
				printf("\tSet Fan mode : Auto Fan Duty Mode\n");
				break;
			case 2:
				status = SetFanMode(idx, ePWM_Fan_Manual_RPM_Mode);
				printf("\tSet Fan mode : Manual RPM Mode\n");
				break;
			case 3:
				status = SetFanMode(idx, ePWM_Fan_Manual_Duty_Mode);
				printf("\tSet Fan mode : Manual Duty Mode\n");
				break;
			default:
				printf("\tSet Fan mode Error!");
				return -1;
		}
		
		break;
	default:
		break;
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
	fan_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
