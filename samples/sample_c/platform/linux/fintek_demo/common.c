#include "fintek_api.h"

// wrap from different IC

int _EnableUART(uint32_t ch)
{
#if 0
	eSIO_TYPE ic_type = get_current_ic_type();
	int r = 0;

	switch (ic_type) {
	case eSIO_TYPE_F75115_HID:
		// MO/MI/CLK
		r |= MultiFunc_Select(0x15, eMultiModeType_SPI);
		r |= MultiFunc_Select(0x16, eMultiModeType_SPI);
		r |= MultiFunc_Select(0x17, eMultiModeType_SPI);

		// CSx2
		r |= MultiFunc_Select(0x14, eMultiModeType_SPI);
		r |= MultiFunc_Select(0x20, eMultiModeType_SPI);
		return r;
	}

	return -1;
#endif
	// f75115/hid always enabled UART
	return 0;
}

int _EnablePWM(uint32_t ch)
{
	eSIO_TYPE ic_type = get_current_ic_type();
	int r = 0;

	if (ch >= GetPWMCount())
		return -1;

	switch (ic_type) {
	case eSIO_TYPE_F75115:
	case eSIO_TYPE_F75115_HID:
		{
			uint32_t pwm_f75115_gpio_map[] = { 0x24, 0x25, 0x26, 0x27 };

			r = MultiFunc_Select(pwm_f75115_gpio_map[ch], eMultiModeType_PWM);
			if (r) {
				printf("MultiFunc_Select failed\n");
				return r;
			}
		}

	default:
		// no special enable control
		break;
	}

	return 0;
}

int _EnableSPI(uint32_t ch)
{
	eSIO_TYPE ic_type = get_current_ic_type();
	int r = 0;

	switch (ic_type) {
	case eSIO_TYPE_F75115_HID:
		if (ch != 0)
			return -1;
		
		// MO/MI/CLK
		r |= MultiFunc_Select(0x15, eMultiModeType_SPI);
		r |= MultiFunc_Select(0x16, eMultiModeType_SPI);
		r |= MultiFunc_Select(0x17, eMultiModeType_SPI);

		// CSx2
		r |= MultiFunc_Select(0x14, eMultiModeType_SPI);
		r |= MultiFunc_Select(0x20, eMultiModeType_SPI);
		return r;
	}

	return -1;
}

int _EnableI2C(uint32_t ch)
{
	if (ch >= I2C_GetMaxChannel()) {
		printf("idx(%d) >= I2C_GetMaxChannel()(%d)\n", ch, I2C_GetMaxChannel());
		return -1;
	}

	return 0;
}
