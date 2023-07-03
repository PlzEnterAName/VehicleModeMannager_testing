 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_OTA.h 
  * @author  Application Team
  * @version V1.0.0
  * @date    1-6-2023
  * @brief   OTA manager
  *******************************************************************************/

#ifndef __M7_0_OTA_H_
#define __M7_0_OTA_H_

#include "M7_0_VMMPublic.h"

typedef enum
{
    MODE_NORMAL = 0,
    MODE_OTA
}M7_0_OTA_MODE;

typedef enum
{
    NORMAL_STATUS = 0,
    OTA_STATUS
}M7_0_OTA_STATUS;

typedef enum
{
    OTA_STS_INACTIVE = 0x00,
    OTA_STS_ACTIVE = 0x01
}M7_0_OTA_STS_FEEDBACK;

typedef enum
{
    NO_REQUEST = 0,
    POWER_OFF,
    POWER_ON,
    CONTINUOUS_ON
}M7_0_OTA_POWER_STATUS;

#define TIME_LMT_20MIN  (20 * 60 * 1000)
#define TIME_LMT_2HOUR  (2 * 60 * 60 * 1000)

void M7_0_OTA_Mode_Init(void);
void M7_0_OTAModeManagerRunnable(void);

#endif
