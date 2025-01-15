
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEMO_GPIO_INT_H
#define DEMO_GPIO_INT_H

/* Includes ------------------------------------------------------------------*/
#include "dji_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
T_DjiReturnCode hhit_GetNewestPpsTriggerLocalTimeUs(uint64_t *localTimeUs);

T_DjiReturnCode hhit_gpio_Init();

T_DjiReturnCode hhit_gpio_DeInit();

#ifdef __cplusplus
}
#endif

#endif 
