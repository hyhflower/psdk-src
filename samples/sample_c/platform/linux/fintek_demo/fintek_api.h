#ifndef __FINTEK_API_H__
#define __FINTEK_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define BETA_RELEASE 0

#if !defined(__arm__) && !defined(__aarch64__)
#include <sys/io.h>
#endif

#define BIT(x) (1ULL << (x))
#define SHIFT(x,y) ((x) << (y))
#define MIN(x,y) ( ((x) > (y)) ? (y) : (x) )
#define MAX(x,y) ( ((x) > (y)) ? (x) : (y) )
#define MAX_PATHNAME_SIZE 64

#define MAX_GPIO_SET 8
#define MAX_GPIO (16 * MAX_GPIO_SET)

#define CHECK_RET(x) \
	{	\
		int tmp_ret; \
		if(tmp_ret=(x)) {  \
			fprintf(stderr, #x" line: %d error: %d\n", __LINE__, tmp_ret); \
			return tmp_ret;\
		} \
	}

#define CHECK_RET_TYPE(x, y) \
	{	\
		int tmp_ret; \
		if(tmp_ret=x) {  \
			fprintf(stderr, #x" line: %d error: %d\n", __LINE__, tmp_ret); \
			return (y)(long)tmp_ret;\
		} \
	}

#define DEBUG_FUN \
	do {	\
		if (get_debug()) \
			fprintf(stderr, "%s: %d\n", __func__, __LINE__); \
	} while (0);

#define DEBUG(x) \
	do {	\
		if (get_debug()) {\
			x; \
		}\
	} while (0);

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

typedef enum {
	eGPIO_Mode_Disable,
	eGPIO_Mode_Enable,
	eGPIO_Mode_Invalid,
} eGPIO_Mode;

typedef enum {
	eGPIO_Drive_Mode_OpenDrain,
	eGPIO_Drive_Mode_Pushpull,
	eGPIO_Drive_Mode_Invalid,
} eGPIO_Drive_Mode;

typedef enum {
	eGPIO_Direction_In,
	eGPIO_Direction_Out,
	eGPIO_Direction_Invalid,
} eGPIO_Direction;

typedef enum {
	eGPIO_Pull_Low,
	eGPIO_Pull_High,
	eGPIO_Pull_Disable,
	eGPIO_Pull_Invalid,
} eGPIO_Pull_Mode;

typedef enum {
	eGPIO_Int_Trigger_Rising,
	eGPIO_Int_Trigger_Failing,
	eGPIO_Int_Trigger_Edge,
	eGPIO_Int_Trigger_Invalid,
} eGPIO_Int_Trigger;

typedef enum {
	eSIO_TYPE_SIO = 0,
	eSIO_TYPE_F71808A = eSIO_TYPE_SIO,
	eSIO_TYPE_F71869E,
	eSIO_TYPE_F71869A,
	eSIO_TYPE_F71889E,
	eSIO_TYPE_F81866,
	eSIO_TYPE_F75113,
	eSIO_TYPE_F81801,
	eSIO_TYPE_F81768,
	eSIO_TYPE_F81803 = eSIO_TYPE_F81768,
	eSIO_TYPE_F81966,
	eSIO_TYPE_F81804 = eSIO_TYPE_F81966,
	eSIO_TYPE_F81968,
	eSIO_TYPE_F81216,
	eSIO_TYPE_F81216AD,
	eSIO_TYPE_F81216H,
	eSIO_TYPE_F81214E,
	eSIO_TYPE_F81216E,
	eSIO_TYPE_F81218E,

	eSIO_TYPE_HID,
	eSIO_TYPE_F75114 = eSIO_TYPE_HID,
	eSIO_TYPE_F75115_HID,

	eSIO_TYPE_PCI,
	eSIO_TYPE_F81504 = eSIO_TYPE_PCI,
	eSIO_TYPE_F81508,
	eSIO_TYPE_F81512,
	eSIO_TYPE_F81504A,
	eSIO_TYPE_F81508A,
	eSIO_TYPE_F81512A,

	eSIO_TYPE_USB,
	eSIO_TYPE_F81532A = eSIO_TYPE_USB,
	eSIO_TYPE_F81534A,
	eSIO_TYPE_F81535,
	eSIO_TYPE_F81536,
	eSIO_TYPE_F75115,

	eSIO_TYPE_I2C,
	eSIO_TYPE_F75113_I2C = eSIO_TYPE_I2C,
	eSIO_TYPE_F75387_I2C,
	eSIO_TYPE_F75111_I2C,

	eSIO_TYPE_UNKNOWN,
	eSIO_TYPE_INVALID,
} eSIO_TYPE;

typedef enum {
	eError_NoError = 0,
} eError_Type;

typedef enum {
	eMultiModeType_GPIO,
	eMultiModeType_I2C,
	eMultiModeType_SPI,
	eMultiModeType_PWM,
	eMultiModeType_UART,
	eMultiModeType_INVALID,
} eMultiMode_Type;

typedef enum {
	ePWM_Fan_Auto_Fan_RPM_Mode,
	ePWM_Fan_Auto_Fan_Duty_Mode,
	ePWM_Fan_Manual_RPM_Mode,
	ePWM_Fan_Manual_Duty_Mode,
} ePWM_Fan_Mode;

typedef struct {
	uint32_t vid;
	uint32_t pid;
	eSIO_TYPE ic_type;

	uint8_t ic_port;
	uint8_t key;
	char path_name[MAX_PATHNAME_SIZE];
	char device_arch[MAX_PATHNAME_SIZE];

	void *next;
} sFintek_sio_data, *psFintek_sio_data;

#ifdef __cplusplus
extern "C" {
#endif

	uint8_t READ_IC(unsigned long reg);
	void WRITE_IC(unsigned long reg, uint8_t data);
	void WRITE_MASK_IC(uint8_t reg, uint8_t mask, uint8_t data);

	int init_fintek_sio(eSIO_TYPE eType, int index, psFintek_sio_data sio_data);
	int init_fintek_sio_force(eSIO_TYPE eType, psFintek_sio_data sio_data);

	void ActiveSIO(uint8_t port, uint8_t key);
	void DeactiveSIO(uint8_t port);

	int get_device_list(eSIO_TYPE eType, psFintek_sio_data * head);

	int _EnableUART(uint32_t ch);
	int _EnablePWM(uint32_t ch);
	int _EnableSPI(uint32_t ch);
	int _EnableI2C(uint32_t ch);

	int _EnableGPIO(uint32_t uIdx, eGPIO_Mode eMode);

	int _SetGpioOutputEnableIdx(uint32_t uIdx, eGPIO_Direction eMode);
	int _GetGpioOutputEnableIdx(uint32_t uIdx, eGPIO_Direction * eMode);

	int _SetGpioOutputDataIdx(uint32_t uIdx, uint32_t uValue);
	int _SetGpioGroupOutputDataIdx(uint32_t set, uint32_t uValue);
	int _GetGpioOutputDataIdx(uint32_t uIdx, uint32_t * uValue);

	int _GetGpioInputDataIdx(uint32_t uIdx, uint32_t * uValue);
	int _GetGpioGroupInputDataIdx(uint32_t set, uint32_t * uValue);

	int _SetGpioDriveEnable(uint32_t uIdx, eGPIO_Drive_Mode eMode);
	int _GetGpioDriveEnable(uint32_t uIdx, eGPIO_Drive_Mode * eMode);

	int _SetGpioPullMode(uint32_t uIdx, eGPIO_Pull_Mode eMode);
	int _GetGpioPullMode(uint32_t uIdx, eGPIO_Pull_Mode * eMode);

	int _EnableGpioInt(uint32_t set, uint32_t en_bit, uint32_t irq);
	int _DisableGpioInt(uint32_t set, uint32_t dis_bit);
	int _GetGpioIntStatus(uint32_t set, uint32_t * bit_status);
	int _ClearGpioIntStatus(uint32_t set, uint32_t clear_bit_status);

	int _SetGpioIntTriggerMode(uint32_t uIdx, eGPIO_Int_Trigger eMode);
	int _GetGpioIntTriggerMode(uint32_t uIdx, eGPIO_Int_Trigger * eMode);

	int fintek_gpio_api(int ic, int cnt, int gpio_idx, int dir, int mode, int pull_mode, int out_val, int *in_val);

	int SetWdtConfiguration(int iTimerCnt, int iClkSel, int iPulseMode, int iUnit, int iActive, int iPulseWidth);
	int SetWdtEnable();
	int SetWdtDisable();

	int GetWdtTimeoutStatus(int *Status, int *RemainTime);
	int SetWdtIdxConfiguration(int idx, int iTimerCnt, int iClkSel,
				   int iPulseMode, int iUnit, int iActive, int iPulseWidth);

	int SetWdtIdxEnable(int idx);
	int SetWdtIdxDisable(int idx);

	int GetWdtIdxTimeoutStatus(int idx, int *Status, int *RemainTime);
	int GetWdtTimeoutReadable(void);

	//int GetHWMONAddr(uint32_t *addr);

	int SelectI2CChannel(uint32_t ch);
	int WriteI2CData(uint32_t dev, uint32_t addr, uint32_t data);
	int ReadI2CData(uint32_t dev, uint32_t addr, uint32_t * data);

#if 0
	int InitEEPROM(void);
	int SetEEPROMValue(uint32_t dev, uint32_t addr, uint32_t data);
	int GetEEPROMValue(uint32_t dev, uint32_t addr, uint32_t * data);
#endif

	int I2C_GetMaxChannel();
	int I2C_Start(int idx);
	int I2C_Write_Data(int idx, uint8_t data);
	int I2C_Read_Data(int idx, uint8_t * data, int nack);
	int I2C_Stop(int idx);

	int SpiSetCs(int idx, int en);
	int SpiSetPolPha(int idx, int pol, int pha);
	int SpiReadData(int idx, uint8_t * data);
	int SpiWriteData(int idx, uint8_t data);

	void set_debug(int on);
	int get_debug(void);
	char *getFintekLibVersion(void);

	uint8_t IOReadByte(uint32_t addr);
	void IOWriteByte(uint32_t addr, uint8_t data);

	int _GPIO_Check_Index(int logicIndex);
	int _GPIO_Check_Set(int set);
	int _GPIO_Check_Int_Set(int set, uint8_t mask);

	int _WDT_count();

	int GetCaseOpenStatus(int *st);
	int ClearCaseOpenStatus();

	int GetPWMCount();
	int SetPWMSioRawConfig(int idx, uint32_t val);
	int GetPWMSioRawConfig(int idx, uint32_t * val);
	int GetPWMFreqSupport_Size(int idx);
	int GetPWMFreqSupport_List(int idx, uint32_t * data, int in_size);
	int SetPWM_Freq_Div_Percentage(int idx, int freq, int div, int percentage);
	int GetPWM_Freq_Div_Percentage(int idx, int *freq, int *div, int *percentage);
	int GetPWMMaxDivider(int idx, int *max_div);
	int IsPWMShareClock(void);

	int GetFanPWMSioRPMConfig(int idx, uint32_t * val);
	int GetFanMode(int idx, ePWM_Fan_Mode* eMode);
	int SetFanMode(int idx, ePWM_Fan_Mode eMode);

	int GetTemperatureCount();
	int GetTemperature(int idx, int *temperature);
	int GetTemperature_PECI(int *temperature);

	int UART_GetMaxChannel();
	int UART_SetBaudRate(uint32_t idx, uint32_t baudrate);
	int UART_SetDTR(uint32_t idx, uint32_t en);
	int UART_SetRTS(uint32_t idx, uint32_t en);
	int UART_GetDCD(uint32_t idx, uint32_t * status);
	int UART_GetDSR(uint32_t idx, uint32_t * status);
	int UART_GetCTS(uint32_t idx, uint32_t * status);
	int UART_GetRI(uint32_t idx, uint32_t * status);
	int UART_TX(uint32_t idx, uint8_t * data, uint8_t len);
	int UART_RX(uint32_t idx, uint8_t * data, uint8_t data_len, uint8_t * read_len, uint32_t timeout);

	int MultiFunc_Select(uint32_t idx, eMultiMode_Type mode);
	int MultiFunc_Query(uint32_t idx, eMultiMode_Type * mode);

	eSIO_TYPE get_current_ic_type(void);
#ifdef __cplusplus
}
#endif
#endif
