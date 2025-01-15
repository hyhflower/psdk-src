#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include "fintek_api.h"

sFintek_sio_data sio_data;

void usage(char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek PWM Demo API:\n");
	fprintf(stderr, "write pwm:\n");
	fprintf(stderr, "\t%s <idx> <en> <freq> <div> <duty>\n\n", argv[0]);
	fprintf(stderr, "read pwm:\n");
	fprintf(stderr, "\t%s <idx>\n\n", argv[0]);
}

int multi_func_query(int channel, eMultiMode_Type * mode)
{
	int r;

	switch (sio_data.ic_type) {
	case eSIO_TYPE_F75115_HID:
	case eSIO_TYPE_F75115:
		{
			static uint32_t pwm_gpio_map[] = {
				0x24, 0x25, 0x26, 0x27
			};

			r = MultiFunc_Query(pwm_gpio_map[channel], mode);
			if (r) {
				printf("MultiFunc_Query failed\n");
				return -1;
			}
		}

		break;

	default:
		printf("%s: may not support multi-query()\n", __func__);
	}

	return 0;
}

int multi_func_set(int channel, eMultiMode_Type mode)
{
	int r;

	switch (sio_data.ic_type) {
	case eSIO_TYPE_F75115_HID:
	case eSIO_TYPE_F75115:
		{
			uint32_t pwm_gpio_map[] = { 0x24, 0x25, 0x26, 0x27 };

			r = MultiFunc_Select(pwm_gpio_map[channel], mode);
			if (r) {
				printf("MultiFunc_Select failed\n");
				return -1;
			}

		}

		break;

	default:
		printf("%s: may not support multi-set()\n", __func__);
	}

	return 0;
}

int pwm_demo(int argc, char *argv[])
{
	eMultiMode_Type mode = eMultiModeType_INVALID;
	uint32_t freq_list[16];
	int freq, div, percentage, en;
	unsigned int val;
	int idx;
	int r, i, max_div;

	if (argc != 6 && argc != 2) {
		usage(argv);
		return -1;
	}

	idx = strtol(argv[1], NULL, 0x16);
	if (idx >= GetPWMCount()) {
		printf("idx(%d) >= GetPWMCount()(%d)\n", idx, GetPWMCount());
		return -1;
	}

	/* Multi-Function */
	r = multi_func_query(idx, &mode);
	if (r) {
		printf("Cant read current type, maybe MultiFunc_Query() not implements\n");
	} else {
		switch (mode) {
		case eMultiModeType_PWM:
			printf("idx: %x mode : PWM\n", idx);
			break;
		case eMultiModeType_GPIO:
			printf("idx: %x mode : GPIO\n", idx);
			break;
		default:
			printf("Cant get idx: %x mode\n", idx);
			return 1;
		}
	}

	if (IsPWMShareClock())
		printf("The PWMs share same clock, becareful clock change\n");

	r = GetPWMFreqSupport_Size(idx);
	if (r < 0) {
		printf("GetPWMFreqSupport_Size failed: %d\n", r);
		return 1;
	}

	if (r) {
		r = GetPWMFreqSupport_List(idx, freq_list, ARRAY_SIZE(freq_list));
		if (r < 0) {
			printf("GetPWMFreqSupport_List failed: %d\n", r);
			return 1;
		}

		printf("Support Customize Frequency:\n");
		for (i = 0; i < GetPWMFreqSupport_Size(idx); ++i)
			printf("    freq%d: %dHz\n", i, freq_list[i]);
	}

	r = GetPWMMaxDivider(idx, &max_div);
	if (r) {
		printf("GetPWMMaxDivider failed: %d\n", r);
		return 1;
	}

	printf("Max Divider: %d\n\n", max_div);

	r = GetPWM_Freq_Div_Percentage(idx, &freq, &div, &percentage);
	if (r) {
		printf("GetPWM_Freq_Div_Percentage failed: %d\n", r);
		return 1;
	}

	printf("Read Current Config:\n");
	printf("    freq: %d\n", freq);
	printf("    div: %d\n", div);
	printf("    percentage: %d%%\n", percentage);

	if (argc == 2)
		return 0;

	en = strtol(argv[2], NULL, 0);

	/* Multi-Function */
	r = multi_func_set(idx, en ? eMultiModeType_PWM : eMultiModeType_GPIO);
	if (r)
		printf("Cant set current type, maybe MultiFunc_Select() not implements\n");

	/* Setting PWM mode */
	en = strtol(argv[2], NULL, 0);
	freq = strtol(argv[3], NULL, 0);
	div = strtol(argv[4], NULL, 0);
	percentage = strtol(argv[5], NULL, 0);

	r = SetPWM_Freq_Div_Percentage(idx, freq, div, percentage);
	if (r) {
		printf("SetPWM_Freq_Div_Percentage failed: %d\n", r);
		return 1;
	}

	printf("Setting to ...\n");
	printf("    en: %d\n", en);
	printf("    freq: %d\n", freq);
	printf("    div: %d\n", div);
	printf("    percentage: %d%%\n", percentage);

	return 0;
}

int main(int argc, char *argv[])
{
	int nRet = 0;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F75115, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		exit(3);
	}

	ActiveSIO(sio_data.ic_port, sio_data.key);
	pwm_demo(argc, argv);
	DeactiveSIO(sio_data.ic_port);

	return 0;
}
